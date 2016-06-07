/*
Copyright 2016 BitTorrent Inc

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef __BENCPARSER_H__
#define __BENCPARSER_H__

#include <cstring>
#include <vector>

class IBencParser {
public:
	virtual ~IBencParser(){};		// GCC
	enum PARSE_T {
		BERROR = 0,
		INT = 1,
		STRING = 2,
		LIST = 3,
		DICT = 4,
		END_E = 5,
		DONE = 6,
		BPARSE_ABSOLUTE_MAX = 0xff
	};
	virtual PARSE_T ParseNext( const unsigned char **ppStart, size_t *pSize, bool isKey = true ) = 0;
};

class BencParser : public IBencParser{
public:
	BencParser(unsigned char *p, const unsigned char *pEnd) :  _p( p ), _pEnd( pEnd ){}
	virtual ~BencParser(){};		// GCC
	PARSE_T ParseNext( const unsigned char **ppStart, size_t *pSize, bool isKey );
	const unsigned char *GetPos() const { return _p; }

protected:
	unsigned char *_p;
	const unsigned char *_pEnd;
	const unsigned char *ParseNum(size_t *pSize);
	const unsigned char *ParseString(size_t *pSize);
};

class BencParserElement : public BencParser{
public:
	// key is a 'key path' like "foo\0bar\0" for extract mode.
	// note that this causes it to be double-terminated.
	// Each part of this compound string is used as a key in a dictionary to
	// find.
	// Example:
	// key = "foo\0bar\0"
	// d3:aaad...3:bar14:does not match...e3:food...3:bar10:does match...e...e
	// Yields "10:does match" in _elementStart ... _elementEnd.
	BencParserElement(unsigned char *p, const char *key, const unsigned char *pEnd) :
		BencParser( p , pEnd ), _level(0), _elementLevel(0), _elementStart(NULL),
		_elementEnd(NULL), _lastString(NULL), _origKey(NULL),
		_keyMatch(0), // Because we'll match the key in a dictionary
		_keyLevel(0), _keyLen(0), _key(NULL)
		{
			_keys.push_back(key);
		}
	// `keys` will lazily match the first key it can
	// guaranteed to be at least as lazy as the developer who wrote it, so use with
	// caution
	BencParserElement(unsigned char *p, std::vector<const char*> const &keys, const unsigned char *pEnd) :
		BencParser( p , pEnd ), _level(0), _elementLevel(0), _elementStart(NULL),
		_elementEnd(NULL), _lastString(NULL), _keys(keys), _origKey(NULL),
		_keyMatch(0), // Because we'll match the key in a dictionary
		_keyLevel(0), _keyLen(0), _key(NULL)
		{}
	virtual ~BencParserElement(){};		// GCC
	PARSE_T ParseNext( const unsigned char **ppStart, size_t *pSize, bool isKey = true );
	void GetElement( unsigned char **ppElementStart, unsigned char **ppElementEnd ) const;
protected:
	int _level;		// Incremented when a collection starts, and vice versa
	int _elementLevel;	// Level on which we first encountered "Info" string
	unsigned char *_elementStart, *_elementEnd;
	const unsigned char *_lastString;
	std::vector<const char*> _keys; // all keys we could match
	const char *_origKey;
	size_t _keyMatch; // Last matched key
	size_t _keyLevel; // Last time we had a key
	size_t _keyLen;
	const char *_key; // pointer to the key we're currently expecting to match
};

#endif	// __BENCPARSER_H__
