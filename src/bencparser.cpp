#include "bencparser.h"
#include <cassert>
#include <cstdio>
//#include "btstring.h"

#define READBYTE2(START, END, CURRENT) assert(START <= END); if (START == END) return NULL; CURRENT = *((START)++);
#ifdef _DEBUG
#define DEBUG_ASSERT(x) do {assert(x);} while (false)
#else
#define DEBUG_ASSERT(x) do {} while (false)
#endif //__DEBUG

const unsigned char *BencParser::ParseString(size_t *pSize)
{
	unsigned char m;
	READBYTE2(_p, _pEnd, m);

	size_t val = 0;

	for(;;) {
		val = val * 10 + (m - '0');
		READBYTE2(_p, _pEnd, m);
		if (m == ':') break;
		if (!(m >= '0' && m <= '9'))
			return NULL;
	}
	unsigned char *pReturn = _p;
	_p += val;
	*pSize = val;

	// See if this string length goes off the end of the buffer or file
	if(_p > _pEnd) {
		/*DbgLogf("Can't parse string with length longer than remaining buffer:  %Lu %s",
			(uint64_t) val, (const unsigned char*) pReturn);*/
		DEBUG_ASSERT(0);
		return NULL;	// Error will propagate out of the parser
	}

	return pReturn;
}

const unsigned char *BencParser::ParseNum(size_t *pSize)
{
	// this is a more forgiving interpretation of the integer.
	// sometimes torrents are broken and encode integers incorrectly
	// this will avoid failing to parse those

	unsigned char *e = _p;
	// skip any remaining garbage
	while (_p < _pEnd && *_p != (unsigned char) 'e') ++_p;
	*pSize = _p - e;
	if (_p != _pEnd) ++_p; // skip the 'e'
	return (const unsigned char*)e;
}

IBencParser::PARSE_T  BencParser::ParseNext(const unsigned char **ppElement, size_t *pSize, bool isKey )
{
	PARSE_T tReturn = BERROR;
	*pSize = 0;
	if( _p == _pEnd ) return DONE;
	if(_p > _pEnd) {
		DEBUG_ASSERT(0);
		return BERROR;
	}
	unsigned char m = *((_p)++);

	if (m >= '0' && m <= '9') {
		_p--;
		*ppElement = ParseString(pSize);
		tReturn = STRING;
	} else if (m == 'i') {
		*ppElement = ParseNum(pSize);
		tReturn = INT;
	} else if (m == 'l') {
		*ppElement = _p;
		tReturn = LIST;
	} else if (m == 'd') {
		*ppElement = _p;
		tReturn = DICT;
	} else if (m == 'e') {
		*ppElement = _p;
		tReturn = END_E;
	} else {
		*ppElement = NULL;
	}

	if( *ppElement == NULL )
		return BERROR;
	return tReturn;
}

// Look for a specified dictionary element
// Mark its start and end.
// isKey means match a key and update match information, otherwise capture a value if we matched the key last time
// _key is a 'key path' like "foo\0bar\0\0"
// This means, match [benc] in "d ... 3:food ... 3:bar[ benc ] ... e ... e"
// Note that the double eol is required.
IBencParser::PARSE_T  BencParserElement::ParseNext( const unsigned char **ppStart, size_t *pSize, bool isKey )
{
	const unsigned char* pElementStart = _p; // store the pointer to the start of the element before ParseNext moves it away
	PARSE_T parseReturn = BencParser::ParseNext( ppStart, pSize, isKey );

	// Skip this business if we think we have found the element
	if ( _elementStart && _elementEnd ) {
		return parseReturn;
	}

	// _lastString is expected to be the key we are interested in.
	switch ( parseReturn )
	{
		case INT:
			// This condition is repeated verbatim for each case below:
			// It means that we capture the result if we have matched every key in the key path so far, and there are no more
			// keys to match in depth.
			if( _lastString && _elementStart == NULL && _level && static_cast<size_t>((1 << _level) - 1) == _keyMatch && !*(_key + _keyLen + 1) ) {
				// +1 and -1 are to include the i-prefix and e-suffix
				_elementStart = const_cast<unsigned char*>((*ppStart)-1);
				_elementEnd = const_cast<unsigned char*>((*ppStart) + *pSize + 1);
			}
			break;
		case STRING :
			if (isKey && _level > 0) 
			{
				_lastString = *ppStart;
				_lastSize = *pSize;
				// We mark a key match if we were parsing a key position, and the string matches in length and content
				if (_keyMatch == 0) {
					for (std::vector<const char*>::const_iterator k = _keys.begin(); k != _keys.end(); k++) {
						if (memcmp( (unsigned char *) _lastString, *k, strlen(*k)) == 0 && strlen(*k) == *pSize) {

							_origKey = _key = *k;
							_keyLen = strlen(*k);
							_keyMatch |= 1 << (_level - 1);
							break;
						}
					}
				}
				else if (memcmp( (unsigned char *) _lastString, _key, _keyLen) == 0 && _keyLen == *pSize) {
					// _keyMatch is a bitstring whose i'th bit remembers whether the key leading here at level i matched
					// the desired key in the key path in _key.
					_keyMatch |= 1 << (_level - 1);
				}
			} else if( _lastString && _elementStart == NULL && _level && static_cast<size_t>((1 << _level) - 1) == _keyMatch && !*(_key + _keyLen + 1) ) {
				_elementStart = const_cast<unsigned char*>(pElementStart);
				_elementEnd = const_cast<unsigned char*>(*ppStart + *pSize);
			}
			break;
		case DICT :
		case LIST :
			// check for "element", record start
			if( _lastString && _elementStart == NULL && _level && static_cast<size_t>((1 << _level) - 1) == _keyMatch && !*(_key + _keyLen + 1) ) {
				_elementStart = const_cast<unsigned char*>(*ppStart - 1);
				_elementLevel = _level;
			}
			_lastString = 0;
			_lastSize = 0;
			// _keyLen == 0 means that we've exceeded the length of our key path, but we're in a deeper structure
			// in bencoding.  We need to remember how far until we walk back into the key.
			if (_keyLen == 0) {
				_keyLevel++;
			} else if( _level != 0 ) {
				// Go past the \0 end of this key to the next key in the key path.
				_key += _keyLen + 1;
				_keyLen = strlen(_key);
			}
			_level++;
			break;
		case END_E :
			// Unset match for this level, because we're exiting the dictionary.  Note that we might have matches
			// and nonmatches mixed.  We only consider that we have the right element if the value is "all-1s"
			// and we're at the last key.
			_keyMatch &= ~(1 << (_level - 1));
			_level--;
			// _keyLevel remembers how many levels deep we are beyond the key path we were given
			if (_keyLevel > 0)
				_keyLevel--;
			else if ( _level != 0 ) {
				// scan in reverse to the beginning of the previous key in the key path.  Make sure not to
				// check a byte before the whole key path.
				do {
					_key--;
				} while (_key-1 >= _origKey && _key[-1]);
				// We could compute this while scanning but i'm lazy
				_keyLen = strlen(_key);
			}
			if( _elementLevel && _elementStart && _elementLevel == _level && _elementEnd == NULL ) {
				_elementEnd = const_cast<unsigned char*>(*ppStart);	// there, we're done
			}
			break;
		case BERROR:
		case DONE:
		case BPARSE_ABSOLUTE_MAX:
		default:
			// just to quiet some warnings about the switch not addressing these enum items
			break;
	}
	return parseReturn;
}

void BencParserElement::GetElement( unsigned char **ppElementStart, unsigned char **ppElementEnd ) const
{
	*ppElementStart = _elementStart;
	*ppElementEnd = _elementEnd;
}
