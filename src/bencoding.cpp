#include "bencoding.h"
#include "invariant_check.hpp"
#include "DecodeEncodedString.h"
#include <climits> //for INT_MAX
#include <stdlib.h> // for _atoi64 or strtoll
#include <vector>
#include "snprintf.h"

char* wstr_to_utf8(const wchar_t * input, size_t *out_len)
{
	size_t size = 127;
	size_t i = 0;
	// +4 to make sure we don't over-write the buffer while decoding
	char* output = (char*)malloc(sizeof(char) * (127 + 4));

	for(;;) {
		unsigned int c = *input++;
		if (c >= 0x80) {
			if (c >= 0x800) {
				// 3-byte code
				output[i++] = 0xE0 | (c >> 12);
				output[i++] = ((c >> 6) & 0x3F) | 0x80;
			} else {
				// 2-byte code
				output[i++] = 0xC0 | (c >> 6);
			}
			c = (c & 0x3F) | 0x80;
		}

		output[i++] = c;

		if (!c)
			break;

		// Need to make more room for next round?
		if (i >= size)
			output = (char*)realloc(output, (size *= 2) + 4);
	}

	if (out_len) *out_len = i - 1;

	return output;
}

/* TODO: this is not really valid. Not knowing what is the encoding of
   <input>, we can't convert it to utf8, so this really works only if
   <input> is already valid utf8 */
char* str_to_utf8(const char * input, size_t *out_len)
{
	if (out_len) *out_len = strlen(input);
	return strdup(input);
}

#ifdef _UNICODE
char* EncodeUtf8(ctstr input, size_t *out_len = NULL) { return wstr_to_utf8(input, out_len); }
#else
char* EncodeUtf8(ctstr input, size_t *out_len = NULL) { return str_to_utf8(input, out_len); }
#endif

BencEntityMem::BencEntityMem(ctstr memArg, size_t len): BencEntity( BENC_STR ) {
	assert(memArg);
	std::string utf8(EncodeUtf8(memArg));
	if(len == static_cast<size_t>(~0))
		len = utf8.size();
	mem = new BencodedMem( (unsigned char *) utf8.c_str(), static_cast<int>(len) );
}

// Move memory and values from the argument, then
// zeroes out argument memory.
// I guess we need unique_ptr
void BencEntity::MoveFrom(BencEntity& be)
{
	assert(bencType == be.bencType || bencType == BENC_VOID || bencType == BENC_NULL);
	assert(this != &be);
	FreeMembers();
	switch (be.bencType) {
		case BENC_STR:
		case BENC_STR_INPLACE:
		case BENC_INT_LAZY:
			mem = be.mem;
			break;
		case BENC_VLIST:
			vlist = be.vlist;
			break;
		case BENC_LIST:
			list = be.list;
			break;
		case BENC_DICT:
			dict = be.dict;
			break;
		case BENC_INT:
		case BENC_BIGINT:
		case BENC_VOID:
		default:
			num = be.num;
			break;
	}
	bencType = be.bencType;
	be.ZeroOut();
}

// Shallow copy for Map operations.
// Arg constness is violated so we can call MoveFrom.
BencEntity::BencEntity(const BencEntity& be)
{
	ZeroOut();
	MoveFrom( const_cast<BencEntity&>( be ) );
}

void BencEntity::CopyFrom(const BencEntity& b)
{
	switch (b.bencType) {
		case BENC_BOOL:
			bencType = b.bencType;
			num = b.num;
			break;
		case BENC_NULL:
			bencType = b.bencType;
			break;
		case BENC_VOID:
			break;
		case BENC_INT:
			SetInt(b.num);
			break;
		case BENC_BIGINT:
			SetInt64(b.num);
			break;
		case BENC_STR: // What about TSTR?
		case BENC_INT_LAZY:
			((BencEntityMem *) this)->CopyFrom( b );
			break;
		case BENC_LIST:
		case BENC_VLIST:
		{
			((BencodedList *)this)->CopyFrom( b );
		}
			break;
		case BENC_DICT:
		{
			((BencodedDict *)this)->CopyFrom( b );
		}
			break;
		default:
			assert(0);
			break;
	}
}

void BencEntity::FreeMembers()
{
	switch(bencType) {
	case BENC_VOID:
	case BENC_BIGINT:
	case BENC_INT: break;
	case BENC_STR:
	case BENC_INT_LAZY:
		((BencEntityMem *) this)->FreeMembers();
		break;
	case BENC_LIST:
		((BencodedList *) this)->FreeMembers();
		break;
	case BENC_VLIST:
		delete vlist;
		vlist = NULL;
		break;
	case BENC_DICT:
		((BencodedDict *) this)->FreeMembers();
		break;
	default:
		// Types used by JsonParser
			assert(bencType == (BENC_T)0xfd || bencType == (BENC_T)0xff || bencType == (BENC_T)0xfe);
		break;
	}

#ifdef _DEBUG
	switch(bencType) {
		case BENC_INT:
		case BENC_BIGINT:
			num = 0;
			break;
		case BENC_DICT:
			dict = 0;
			break;
		case BENC_STR:
		case BENC_INT_LAZY:
			mem = 0;
			break;
		case BENC_VLIST:
			vlist = 0;
			break;
		case BENC_LIST:
			list = 0;
			break;
		default:
			break;
	}
#endif

	bencType = BENC_VOID;
}

void BencEntity::ZeroOut()
{
	mem = NULL;
	list = NULL;
	dict = NULL;
	num = 0;
	bencType = BENC_VOID;
}

size_t BencodedList::GetCount() const
{
	assert (bencType == BENC_LIST || bencType == BENC_VLIST);
	if (bencType == BENC_LIST) {
		return list->size();
	}
	else {
		return vlist->count;
	}
}


BencEntity *BencodedList::Get(size_t i) const
{
	assert (bencType == BENC_LIST || bencType == BENC_VLIST);
	if (bencType == BENC_LIST) {
		return &(*list)[i];
	}
	else {
		if (vlist->focus != i) {
			vlist->focus = i;
			vlist->ent.FreeMembers();
			vlist->callback(vlist->user, i, &vlist->ent);
		}
		return &(vlist->ent);
	}
}

#ifdef _DEBUG
static char* NeedPrintAsHex(unsigned char *mem, int len)
{
	for(int i=0; i!=len; i++)
		if (mem[i] < 32) {
			char* hex = new char[len * 2 + 1];
			if (!hex) return NULL;
			hex[len*2] = 0;
			for(i=0; i!=len; i++) {
				unsigned char b = mem[i];
				hex[i*2] = "0123456789ABCDEF"[b >> 4];
				hex[i*2+1] = "0123456789ABCDEF"[b & 0xF];
			}
			return hex;
		}
	return NULL;
}

void BencEntity::Print(int indent)
{

	unsigned int i;
	int j;

	switch(bencType) {
	case BENC_INT:
		printf("%Ld", num);
		break;

	case BENC_BIGINT:
		printf("%Ld", num);
		break;

	case BENC_STR: {
		char* hex = NeedPrintAsHex((unsigned char *) mem->GetRaw(), mem->GetCount());
		if (!hex) {
			//btprintf("\"%.*s\"", mem->GetCount(), mem->GetRaw());
			printf("\"%.*s\"", static_cast<int>(mem->GetCount()), mem->GetRaw());
		} else {
			if (strlen(hex) > 200)
				printf("%.200s...", hex);
			else
				printf("%s", hex);
			free(hex);
		}
		break;
	}

	case BENC_LIST:
	case BENC_VLIST:
		printf("[");
		for(i=0; i!=((BencodedList*)this)->GetCount(); i++) {
			if (i != 0) printf(", ");
			((BencodedList *) this )->Get(i)->Print(indent);
		}
		printf("]");
		break;

	case BENC_DICT:
		printf("{");
		i = 0;
		for(BencodedEntityMap::iterator it = dict->begin();
			it != dict->end(); it++, i++ ) {
			if (i != 0) printf(", ");
			printf("\n");
			for(j=0; j!=indent; j++) printf("	");
			unsigned int len  = it->first.GetCount();
			char* s = (char*)malloc(sizeof(char)*(len+1));
			memcpy(s, it->first.GetRaw(), len);
			s[len] = 0;
			printf("%s = ", s);
			free(s);
			it->second.Print(indent+1);
		}
		printf("}");
		break;
	default:
		assert(0);
	}

}
#endif

void BencodedEmitterBase::EmitChar(char a) {
	_emit_buf.push_back(a);
}

void BencodedEmitterBase::Emit(const void *a, size_t len) {
	if (len > 0) {
		assert(a);
		_emit_buf.insert(_emit_buf.end(), (char*)a, (char*)((char*)a + len*sizeof(char)));
	}
}

unsigned char* BencodedEmitterBase::GetResult(size_t* len) {
	unsigned char* result = static_cast<unsigned char*>(malloc(_emit_buf.size()*sizeof(unsigned char)));
	memcpy(result, &_emit_buf[0], _emit_buf.size()*sizeof(unsigned char));
	if (len) *len = _emit_buf.size();
	_emit_buf.clear();
	return result;
}

void BencodedEmitter::EmitEntity(const BencEntity *e) {
	char buf[64];

	switch(e->bencType) {
	case BENC_INT:
		//Emit(buf, btsnprintf(buf, lenof(buf), "i%Lde", e->num));
		Emit(buf, snprintf(buf, sizeof(buf), "i%llde", e->num));
		break;

	case BENC_BIGINT:
		//Emit(buf, btsnprintf(buf, lenof(buf), "i%Lde", e->num));
		Emit(buf, snprintf(buf, sizeof(buf), "i%llde", e->num));
		break;

	case BENC_STR: {
		const BencEntityMem *me = BencEntity::AsBencString( e );
		//Emit(buf, btsnprintf(buf, lenof(buf), "%d:", me->GetSize()));
		Emit(buf, snprintf(buf, sizeof(buf), "%lu:", me->GetSize()));
		Emit(me->GetRaw(), me->GetSize());
		break;
	}
	case BENC_LIST:
	case BENC_VLIST:
	{
		const BencodedList *el = BencEntity::AsList( e );
		EmitChar('l');
		for(size_t i=0; i!=el->GetCount(); i++)
			EmitEntity(el->Get(i));
		EmitChar('e');
		break;
	}
	case BENC_DICT: {
		const BencodedDict *ed = BencEntity::AsDict( e );
		EmitChar('d');
		for(BencodedEntityMap::const_iterator it = ed->dict->begin(); it != ed->dict->end(); it++ ) {
			size_t j = strnlen((char*)(&(it->first[0])),
					it->first.GetCount());
			//Emit(buf, btsnprintf(buf, lenof(buf), "%u:", j));
			Emit(buf, snprintf(buf, sizeof(buf), "%lu:", j));
			Emit(&(it->first[0]), j);
			EmitEntity(&it->second);
		}
		EmitChar('e');
		break;
	}
	default:
		assert(0);

	}
}

#if 0	//ascii
	void EmitQuotedAscii(const void* a, int len)
	{
		for (int i = 0; i < len; ++i) {
			int c = ((const char*)a)[i];
			switch (c) {
			case 10:
			case 13:
			case '\\':
				_emit_buf.push_back("\\");
				// fall through
			default:
				EmitChar(c);
			}
		}
	}


	void EmitAsAscii(const BencEntity *e, const char* prefix = "")
	{
		char buf[64];

		switch(e->bencType) {
		case BENC_INT:
			Emit(buf, btsnprintf(buf, lenof(buf), "%d", e->num));
			break;

		case BENC_BIGINT:
			Emit(buf, btsnprintf(buf, lenof(buf), "%Ld", e->num));
			break;

		case BENC_STR:
			EmitQuotedAscii(e->mem->GetRaw(), (int)e->mem->GetCount());
			break;

		case BENC_LIST:
		case BENC_VLIST: {
			const BencodedList *el = BencEntity::AsList( e );

			for (size_t i=0; i!=el->list->GetCount(); i++) {
				const BencEntity *ent = el->Get(i);

				switch(ent->bencType) {
				case BENC_LIST:
				case BENC_VLIST:
				case BENC_DICT:
					btsnprintf(buf, lenof(buf), "%s%u.", prefix, i);
					EmitAsAscii(ent, buf);
					break;
				default: {
					int n = btsnprintf(buf, lenof(buf), "%s%u=", prefix, i);
					Emit(buf, n);
					EmitAsAscii(ent, buf);
					Emit("\r\n", 2);
					break;
				}
				}
			}
			break;
		}

		case BENC_DICT: {
			const BencodedDict *ed = BencEntity::AsDict( e );
			for(BencodedEntityMap::const_iterator it = ed->dict->begin(); it != ed->dict->end(); it++ ) {

				switch(it->second.bencType) {
				case BENC_DICT:
				case BENC_LIST:
				case BENC_VLIST:
					btsnprintf(buf, lenof(buf), "%s%s.", prefix, it->first.GetRaw());
					EmitAsAscii(&it->second, buf);
					break;
				default: {
					int n = btsnprintf(buf, lenof(buf), "%s%s=", prefix, it->first.GetRaw());
					Emit(buf, n);
					EmitAsAscii(&it->second, buf);
					Emit("\r\n", 2);
					break;
				}
				}
			}
			break;
		}
		default:
			assert(0);

		}
	}
#endif

unsigned char* SerializeBencEntity(const BencEntity* entity, size_t* len) {
	BencodedEmitter emit;
	emit.EmitEntity(entity);
	return emit.GetResult(len);
}

#if 0
char* BencEntity::SerializeAsAscii(size_t* len) const
{
	BencodedEmitter emit;
	emit.EmitAsAscii(this);
	emit.Emit("", 1);	// 0 terminate
	return (char*)emit.GetResult(len); // len includes 0 terminator
}


char* BencEntity::SerializeAsXML(const char* tag, size_t* len) const
{
	BencodedEmitter emit;
	emit.EmitAsXML(tag, this);
	emit.Emit("", 1);	// 0 terminate
	return (char*)emit.GetResult(len); // len includes 0 terminator
}


char* BencEntity::SerializeAsJson(size_t* len) const
{
	BencodedEmitter emit;
	emit.EmitAsJson(this);
	emit.Emit("", 1);	// 0 terminate
	return (char*)emit.GetResult(len); // len includes 0 terminator
}


char* BencEntity::SerializeByMimeType(const char* mime_type, const char* tag, const char*& encoding, const char* json_callback)
{
	// This is very simple.  Pass in "text/xml" (params ignored) if you want XML.
	// Otherwise you get text/json.
	if (mime_type && strbegins(mime_type, "text/xml")) {
		encoding = "text/xml; charset=UTF-8";
		return SerializeAsXML(tag);
	}

	if (mime_type && strbegins(mime_type, "text/ascii")) {
		encoding = "text/ascii; charset=UTF-8";
		return SerializeAsAscii();
	}

	if (mime_type && strbegins(mime_type, "application/jsonrequest"))
		encoding = "application/jsonrequest"; // echo for completeness

	if (json_callback) {
		string serialized((char*)SerializeAsJson(), adopt_string);
		char* result = str_fmt("%s(%s);", json_callback, serialized.c_str());
		return result;
	} else {
		return SerializeAsJson();
	}
}
#endif


bool BencEntityIsValid(unsigned char *b, size_t len, void *userdata)
{
	BencEntity *self = (BencEntity*)userdata;
	unsigned char *bend = b + len;
	return (BencEntity::Parse(b, *self, bend) == bend);
}

/*int BencEntity::LoadFromFile_Safe(ctstr filename)
{
	return LoadFile_Safe(filename, BencEntityIsValid, this);
}
*/

#define READBYTE() assert(p <= pend); if (p == pend) return NULL; m = *p++;
#define READBYTE2(START, END, CURRENT) assert(START <= END); if (START == END) return NULL; CURRENT = *((START)++);

#define START_SIZE 4

void BencEntity::ParseNum(const unsigned char *p)
{
#ifdef WIN32
	// _strtoi64 does not exist in the old msvcrt.dll,
	// so we can't use it here
	SetInt64(_atoi64((const char*)p));
#else
	char* e;
	SetInt64(strtoll((const char*)p, &e, 10));
#endif
}

// This is where the wacky homerolled polymorphism happens:
// we go from a generic entity to the parsed type with
// placement syntax
bool BencEntity::SetParsed( IBencParser::PARSE_T parseResult, const unsigned char *pElement, size_t size, AllocRegime *regime )
{
	switch( parseResult ) {
		case IBencParser::INT : {
			if(! regime->LazyInt() ){
				BencEntity be(BENC_INT);
				MoveFrom(be);
				ParseNum(pElement);
			} else {
				BencEntityLazyInt beLazy(regime->NewMem((unsigned char *) pElement, (int)size));
				MoveFrom( (BencEntity &) beLazy );
			}
			break;
		}
		case IBencParser::STRING : {
			BencEntityMem beM(regime->NewMem((unsigned char *) pElement, (int)size));
			MoveFrom(beM);
			break;
		}
		case IBencParser::LIST : {
			BencodedList beL;
			MoveFrom(beL);
			break;
		}
		case IBencParser::DICT : {
			BencodedDict beD;
			MoveFrom(beD);
			break;
		}
		case IBencParser::BERROR :
		case IBencParser::END_E :
		case IBencParser::DONE :
		default:
			return false;
	}
	return true;
}


void BencodedList::grow(unsigned int num)
{
	size_t i = START_SIZE;
	do{
		if (num < static_cast<unsigned int>(1<<i)) {
			list->reserve(1<<i);
			break;
		}
		i++;
	} while (i < 32 && num >= static_cast<unsigned int>(1<<i));
}

bool BencodedList::ResumeList(IBencParser *pParser, BencEntity **ent, AllocRegime *regime)
{
	const unsigned char *pElement;
	size_t elementSize;
	IBencParser::PARSE_T parseResult;
	while ((parseResult = pParser->ParseNext(&pElement, &elementSize)) != IBencParser::BERROR) {

		if (parseResult == IBencParser::DONE ||
			parseResult == IBencParser::BERROR) {
			return false;
		}

		// Instead of another item, we have reached the end of this collection
		if (parseResult == IBencParser::END_E) {
//			if (list->size()) {
//				list->shrink_to_fit();
//			}

			*ent = NULL;
			break;
		}

		grow((unsigned int)(list->size()));
		*ent = new BencEntity();
		if((*ent)->SetParsed( parseResult, pElement, (int)elementSize, regime )){
			int bencType = (*ent)->bencType;
			// Don't handle vlist since this is for parsing
			list->push_back(**ent);
			delete *ent;
			*ent = &(list->back());
			if (bencType == BENC_LIST || bencType == BENC_DICT) {
				break;
			}
		}
	}

	return true;
}

 bool BencodedDict::ResumeDict(IBencParser *pParser, BencEntity **ent, AllocRegime *regime )
{
	const unsigned char *pKey;
	size_t keySize;
	IBencParser::PARSE_T parseResult;
	BencodedEntityMap::iterator lastIt = dict->begin();
	// Each dict entry should start with a label/key
	while ((parseResult = pParser->ParseNext(&pKey, &keySize, true)) != IBencParser::BERROR) {

		// Instead of another key, we have reached the end of this collection
		if (parseResult == IBencParser::END_E) {
			*ent = NULL;
			break;
		}

		if (parseResult == IBencParser::DONE) {
			return false;
		}

		// Make sure we have a valid label
		if (parseResult != IBencParser::STRING) {
			return false;
		}

		const unsigned char *pElement;
		size_t elementSize;
		parseResult = pParser->ParseNext(&pElement, &elementSize, false);

		// At this point we have parsed the label and really need an element
		if (parseResult == IBencParser::END_E ||
			parseResult == IBencParser::DONE ||
			parseResult == IBencParser::BERROR) {
			return false;
		}

		BencKey *bencKey = regime->NewKey((unsigned char *)pKey, (int)keySize);
		//std::pair<BencodedEntityMap::iterator, bool> inserted =
		BencodedEntityMap::value_type value (*bencKey, BencEntity(BENC_VOID));
		BencodedEntityMap::iterator inserted = dict->insert(lastIt, value);
		delete bencKey;

		// Check whether the dict has
		// duplicate keys, either from a previous instance,
		// or because the serialized dict being read has dupes.
		// We need to choose the right behaviour for dicts that
		// break spec in each of several cases
		// (e.g. DHT queries, settings file updates, etc.
		// this actually happens in the wild when broken DHT nodes
		// sends invalid dht messages
		if(inserted == lastIt) {
			return false;
		}

		// Point to the one in our dict
		*ent = const_cast<BencEntity *>( &(inserted->second) );
		//Assign it the parsed value/type
		(*ent)->SetParsed( parseResult, pElement, elementSize, regime );
		// Don't handle vlist since this is for parsing
		if ((*ent)->bencType == BENC_LIST || (*ent)->bencType == BENC_DICT) {
			return true;
		}

		lastIt = inserted;
	}
	return(parseResult != IBencParser::BERROR);
}

const unsigned char *BencEntity::ParseInPlace(unsigned char *p, BencEntity &ent, const unsigned char *pend)
{
	InplaceMemRegime regime;
	BencParser parser( const_cast<unsigned char *>(p), pend );
	if(! BencEntity::DoParse(ent, &parser, &regime))
		return NULL;
	return parser.GetPos();
}

/**
NOTE:  if there are multiple instances of a key in the benc string, the first
instance is returned in the region regardless of the key's level in the string.
*/
const unsigned char* BencEntity::ParseInPlace(const unsigned char *p, BencEntity &ent, const unsigned char *pend, const char* key, std::pair<unsigned char*, unsigned char*> *rgn)
{
	assert( rgn );
	InplaceMemRegime regime;
	BencParserElement parser(const_cast<unsigned char *>(p), key, pend);
	if( !BencEntity::DoParse(ent, &parser, &regime) )
		return NULL;
	parser.GetElement( &(rgn->first), &( rgn->second ) );
	return parser.GetPos();
}

/**
NOTE:  if there are multiple instances of a key in the benc string, the first
instance is returned in the region regardless of the key's level in the string.
*/
const unsigned char *BencEntity::Parse(const unsigned char *p, BencEntity &ent, const unsigned char *pend, const char* key, std::pair<unsigned char*, unsigned char*> *rgn)
{
	assert( rgn );
	AllocateMemRegime regime;
	BencParserElement parser( const_cast<unsigned char *>(p), key, pend);
	if( !BencEntity::DoParse(ent, &parser, &regime) )
		return NULL;
	parser.GetElement( &(rgn->first), &( rgn->second ) );
	return parser.GetPos();
}

const unsigned char *BencEntity::Parse(const unsigned char *p, BencEntity &ent, const unsigned char *pend )
{
	AllocateMemRegime regime;
	BencParser parser( const_cast<unsigned char *>(p), pend );
	if( !BencEntity::DoParse(ent, &parser, &regime) )
		return NULL;
	return parser.GetPos();
}

bool BencEntity::DoParse(BencEntity &ent, IBencParser *pParser, AllocRegime *pRegime)
{
	bool bReturn = false;
	ent.FreeMembers();
	const unsigned char *pEnt;
	size_t entSize;
	IBencParser::PARSE_T parseResult = pParser->ParseNext(&pEnt, &entSize);
	// Don't handle vlist since this is for parsing
	if (parseResult == IBencParser::BERROR ||
		(parseResult != IBencParser::LIST && parseResult != IBencParser::DICT)) {
		return false;
	}
	ent.SetParsed( parseResult, pEnt, entSize, pRegime );
	std::vector<BencEntity*> stack;
	stack.push_back(&ent);

	// depth-first tree creation
	while (stack.size()) {
		BencEntity *nent = NULL;
		BencEntity *tent = stack.back();

		// No vlist
		if (tent->bencType == BENC_LIST) {
			bReturn = ((BencodedList *) tent)->ResumeList(pParser, &nent, pRegime);
		} else if (tent->bencType == BENC_DICT) {
			bReturn = ((BencodedDict *) tent)->ResumeDict(pParser, &nent, pRegime);
		}
		if (!bReturn) {
			// error! delete everything
			ent.FreeMembers();
			break;
		}
		if (nent) {
			stack.push_back(nent);
		} else {
			stack.pop_back();
		}
	}

	return bReturn;
}

void BencEntity::SetInt64(int64 i)
{
	FreeMembers();
	bencType = BENC_BIGINT;
	num = i;
}

void BencEntity::SetInt(int i)
{
	FreeMembers();
	bencType = BENC_INT;
	num = i;
}

int BencEntity::GetInt(int def /*= 0*/) const {
	if(bencType != BENC_INT && bencType != BENC_BIGINT) return def;
	return num;
}

int64 BencEntity::GetInt64(int64 def /*= 0*/) const {
	if(bencType != BENC_INT && bencType != BENC_BIGINT) return def;
	return num;
}

//todo: -1 case apparently not covered in unit tests
void BencEntityMem::SetStr(const char* ss, int len /*= -1*/)
{
	assert(bencType == BENC_STR);
	assert(mem);
	if(len == -1 && ss){
		len = (int)strlen(ss);
		assert(len >= 0);
	}
	mem->Clear();
	if( len && ss ){
		mem->AppendTerminated( (const unsigned char *) ss, len );
	}
}

void BencEntityMem::SetStrT(ctstr ss)
{
	assert(bencType == BENC_STR);
	assert(mem);
	mem->Resize(0);
	if (ss != NULL) {
		size_t len = 0;
		char* pEncoded = EncodeUtf8(ss, &len);	//	on error, len set to -1.
		assert(pEncoded);
		assert((int)len >= 0);
		mem->SetArray((unsigned char *) pEncoded, len);
	}
}

void BencEntityMem::SetMem(const void *arg, size_t len)
{
	assert(bencType == BENC_STR);
	assert(arg);
	mem->Clear();
	mem->Append((const unsigned char *) arg, len);
}

void BencEntityMem::SetMemOwn(void *arg, size_t len)
{
	assert(bencType == BENC_STR);
	assert(len == 0 || arg != NULL);
	mem->SetArray((unsigned char *) arg, len);
}

void BencEntityMem::FreeMembers()
{
	if(mem) {
		delete mem;
		mem = NULL;
	}
}

void BencEntityMem::CopyFrom(const BencEntity& b)
{
	assert(bencType == BENC_STR || bencType == BENC_INT_LAZY || bencType == BENC_VOID);
	FreeMembers();
	bencType = b.bencType;
	mem = new BencodedMem( *(b.mem) );
}


int BencEntityLazyInt::GetInt(int def /*= 0*/) {
	assert(bencType == BENC_INT_LAZY);
	return BencEntityLazyInt::GetInt64( def );
}

// Here we do a transformation: once the lazy int gets read/loaded,
// we are transformed to a regular int
int64 BencEntityLazyInt::GetInt64(int64 def /*= 0*/) {
	assert(bencType == BENC_INT_LAZY);
	if(bencType != BENC_INT_LAZY) return def;
	BencodedMem *pMem = mem;
	mem = 0;
	ParseNum( reinterpret_cast<const unsigned char*>(pMem->GetRaw()));
	delete pMem;
	bencType = BENC_INT;
	return BencEntity::GetInt64( def );
}


BencodedList *BencEntity::SetVList(BencVListCallback callback, size_t count, void *user)
{
	FreeMembers();
	bencType = BENC_VLIST;
	vlist = new VListData(callback, user);
	vlist->count = count;
	return (BencodedList*)this;
}


t_string BencEntityMem::GetStringT(int encoding, size_t *count) const {
	if (!(bencType == BENC_STR)) return NULL;
	size_t tmp = 0;
#ifdef _UNICODE
	tchar* str = DecodeEncodedString(encoding, (char*) GetRaw(), GetSize(), &tmp);
	t_string tmps(str);
	free(str);
	assert(tmp <= INT_MAX);
	if (count) *count = tmp;
	return tmps;
#else
	if (count) *count = tmp;
	const char* str = GetString(&tmp);
	return t_string(str, tmp);
#endif
}

// C++ interface functions
BencodedDict *BencodedList::GetDict(size_t i)
{
	return AsDict(Get(i));
}

BencodedList *BencodedList::GetList(size_t i)
{
	return AsList(Get(i));
}
const char* BencodedList::GetString(size_t i, size_t *length) const
{
	const BencEntityMem *pMem = AsBencString(Get(i));
	return (pMem?pMem->GetString(length):NULL);
}

t_string BencodedList::GetStringT(size_t i, int encoding, size_t *length) const
{
	const BencEntityMem *pMem = AsBencString(Get(i));
	return (pMem?pMem->GetStringT(encoding, length):NULL);
}

// This is one of the weird places.  The members of e
// are taken over by the new entry, which is returned.
BencEntity *BencodedList::Append(BencEntity &e)
{
	assert(bencType == BENC_LIST);
	assert(list);
	list->push_back(e);
	return &((*list)[(list->size())-1]);
}

void BencodedList::Delete(size_t i)
{
	assert(bencType == BENC_LIST);
	assert(list);
	BencEntity *d = Get(i);
	d->~BencEntity();
	list->erase(list->begin() + i);
}

int BencodedList::GetInt(size_t item, int def) const
{
	assert(bencType == BENC_LIST);
	assert(list);
	BencEntity const* e = Get(item);
	if (!e) return def;
	return e->GetInt(def);
}

int64 BencodedList::GetInt64(size_t item, int64 def) const
{
	assert(bencType == BENC_LIST);
	assert(list);
	BencEntity const* e = Get(item);
	if (!e) return def;
	return e->GetInt64(def);
}

void BencodedList::FreeMembers()
{
	assert(bencType == BENC_LIST);
	if(list) {
		for(unsigned int i = 0; i < list->size(); i++ )
			(*list)[i].FreeMembers();
		list->clear();
		delete list;
		list = NULL;
	}
}

void BencodedList::CopyFrom(const BencEntity &b)
{
	assert(bencType == BENC_LIST);
	FreeMembers();
	bencType = b.bencType;
	//list = new BencodedEntityList( b.list->size() );
	list = new BencodedEntityList(b.list->begin(), b.list->end());
	/*
	for( int i = 0; i < b.list->size(); i++ ) {
		(*list)[i].ZeroOut(); //ideally need placement new
		(*list)[i].CopyFrom((*b.list)[i]);
}
	list->SetCount(b.list->size());
	*/
}

BencEntityMem *BencodedList::AppendString(const char* str, size_t length /*=-1*/)
{
	assert(bencType == BENC_LIST);
	BencEntityMem beM;
	beM.SetStr( str, (int)length );
	return (BencEntityMem *) Append( beM );
}

BencEntityMem *BencodedList::AppendStringT(ctstr str, size_t length /*=-1*/)
{
	assert(bencType == BENC_LIST);
	BencEntityMem beM;
	beM.SetStrT( str );
	return (BencEntityMem *) Append( beM );
}
BencEntity *BencodedList::AppendInt( int val )
{
	assert(bencType == BENC_LIST);
	BencEntity beInt;
	beInt.SetInt( val );
	return Append( beInt );
}
BencEntity *BencodedList::AppendInt64( int64 val )
{
	assert(bencType == BENC_LIST);
	BencEntity beInt;
	beInt.SetInt64( val );
	return Append( beInt );
}
BencodedDict *BencodedList::AppendDict()
{
	assert(bencType == BENC_LIST);
	BencodedDict bDict;
	return (BencodedDict *) Append(bDict);
}
BencodedList *BencodedList::AppendList()
{
	assert(bencType == BENC_LIST);
	BencodedList beL;
	return (BencodedList *) Append(beL);
}

// lookup function.  Returns NULL if the key is not found.
// The returned pointer is not a copy.  Do not deallocate it.
const BencEntity *BencodedDict::Get(const char* key) const
{
	assert(bencType == BENC_DICT);
	assert(dict);
	BencKey Key( (unsigned char *) key, (int)strlen(key));
	BencodedEntityMap::const_iterator it = dict->find( Key );
	if( it == dict->end() )
		return NULL;
	else
		return &(it->second);
}

// Returns dict matching the given key or returns NULL if the
// key does not exist or the associated value is not a dict.
// The returned pointer is not a copy.  Do not deallocate it.
BencodedDict *BencodedDict::GetDict(const char* key) {
	return AsDict(Get(key));
}

// Returns list matching the given key or returns NULL if the
// key does not exist or the associated value is not a list.
// The returned pointer is not a copy.  Do not deallocate it.
BencodedList *BencodedDict::GetList(const char* key) {
	return AsList(Get(key));
}

// Returns string matching the given key or returns NULL if the
// key does not exist or the associated value is not a string.
// The returned pointer is not a copy.  Do not deallocate it.
// If length is not NULL then upon return it contains the length of
// the returned string.
const char* BencodedDict::GetString(const char* key, size_t *length) const
{
	const BencEntityMem *pMem = AsBencString(Get(key));
	return (pMem?pMem->GetString(length):NULL);
}


char* BencodedDict::GetStringCopy(const char* key) const
{
	size_t len;
	const BencEntityMem *pMem = AsBencString(Get(key));
	const char* val = pMem?pMem->GetString(&len):NULL;
	assert(!val || val[len] == 0);
	return strdup(val);
}

t_string BencodedDict::GetStringT(const char* key, int encoding, size_t *length) const
{
	const BencEntityMem *pMem = AsBencString(Get(key));
	return (pMem?pMem->GetStringT(encoding, length):_T(""));
}

char* BencodedDict::GetString(const char* key, size_t length) const
{
	const BencEntityMem *pMem = AsBencString(Get(key));
	if (!pMem || pMem->GetSize() != length) return NULL;
	return (char*)pMem->GetRaw();
}

int BencodedDict::GetInt(const char* key, int def) const
{
	assert(bencType == BENC_DICT);
	assert(list);
	BencEntity const* e = Get(key);
	if (!e) return def;
	return e->GetInt(def);
}

int64 BencodedDict::GetInt64(const char* key, int64 def) const
{
	assert(bencType == BENC_DICT);
	assert(list);
	BencEntity const* e = Get(key);
	if (!e) return def;
	return e->GetInt64(def);
}

void BencodedDict::CopyFrom(const BencEntity& b)
{
	FreeMembers();
	bencType = BENC_DICT;
	dict = new BencodedEntityMap();
	BencodedEntityMap::iterator lastIt = dict->begin();
	for( BencodedEntityMap::iterator it = b.dict->begin(); it != b.dict->end(); it++) {
		BencKey bencKey(it->first);
		BencodedEntityMap::iterator inserted =
			dict->insert( lastIt, BencodedEntityMap::value_type(bencKey, BencEntity(it->second.bencType)) );
		//bencKey.StealArray();	// leave the new array in the dict
		assert(inserted != lastIt);	// should always be new
		// Point to the one in our dict
		BencEntity *ent = const_cast<BencEntity *>( &(inserted->second) );
		//Assign it the parsed value/type
		ent->CopyFrom( it->second );
		lastIt = inserted;
	}
}

void BencodedDict::FreeMembers()
{
	if(dict) {
		for( BencodedEntityMap::iterator it = dict->begin(); it != dict->end(); it++) {
			// Shouldn't be necessary.
//			const_cast<BencKey *>(&(it->first))->Free();
			it->second.FreeMembers();
		}
		delete dict;
		dict = NULL;
	}
}


#ifdef _DEBUG
bool StrHasElement(std::vector<char*>& l, const char* s, size_t len)
{
	for (size_t i = 0; i < l.size(); i++) {
		if (memcmp(l[i], s, len) == 0)
			return true;
	}
	return false;
}

void BencodedDict::check_invariant() const
{
// There is no such thing as duplicate keys any more in our map.
#if NOT_DONE_YET
	size_t i;
	BencDict *d;
	LList<char*> keys;
	keys.Init();

	for(i=0,d=dict.list; i<dict.num; i++,d++) {
		// If you hit this, uTorrent has screwed up and placed two of the
		// same dictionary keys in one dictionary! If you think you have an old,
		// broken settings.dat, run "fix_duplicate_settings.py settings.dat"
		// Otherwise, find the bug!
		assert(!StrHasElement(keys, d->key, d->keylen));
		char* x = (char*)malloc(char * (d->keylen+1));
		copy(x, d->key, d->keylen);
		x[d->keylen] = 0;
		keys.Append(x);
	}

	keys.FreeAll();
#endif	// NOT_DONE_YET
}

#endif

BencEntity *BencodedDict::Insert(const char* key, BencEntity &val)
{
	INVARIANT_CHECK;

	assert(key);
	assert(bencType == BENC_DICT);
	assert(dict);
	BencKey Key;
	// This way, Key copies as not inplace (deep copy)
	Key.SetArray( (unsigned char *) key, strlen( key ) );
	// insert will make a deep copy of bKey, but a shallow copy of val,
	// which we zero out before returning.
	std::pair<BencodedEntityMap::iterator, bool> result = dict->insert(std::pair<BencKey, BencEntity>(Key, val));
	val.ZeroOut();
	//Key.StealArray();
	return &(result.first->second);
}

// An extension of Append() with the option to do the "push" operation used when
// we are parsing RPCs. This is not relevant to the standard bencoding format,
// and is here because sometimes internally to uTorrent we overload the
// BencodedDict structure to use it like a general dictionary class.
BencEntityMem* BencodedDict::AppendMultiple(char* key, bool allowmultiple) {
	BencEntityMem *slot = NULL;


	if (allowmultiple) { // Possibly populate slot using allowmultiple rules
		slot = (BencEntityMem *) this->Get(key);

		if (slot) { // If there is already a value under that key:
			BencodedList *list = NULL;

			if (slot->bencType == BENC_LIST)
				list = BencEntity::AsList(slot);

			if (!list) { // Entity is not a multiple list yet, so split it out into one
				BencEntity backup;
				backup.MoveFrom( *slot );
				// Null out heap memory pointers so leak asserts don't trigger;
				// a copy of slot was made so should not free resources.
				BencodedList beL;
				slot->MoveFrom(beL);
				list = BencEntity::AsList(slot);	// Transform entity into list

				// Item 0 of the new list is the entity there before
				list->Append(backup);
			}
			BencEntityMem beM;
			slot = (BencEntityMem *) list->Append(beM);
		}
	}

	if (!slot) {// allowmultiple is off, or allowmultiple found nothing present
		BencEntityMem beM;
		slot = (BencEntityMem *) this->Insert(key, beM);
	}

	return slot;
}

void BencodedDict::Delete(const char* key)
{
	INVARIANT_CHECK;
	assert(bencType == BENC_DICT);
	assert(key);
	assert(dict);
	BencKey bKey;
	// Assume we omit the null terminator
	bKey.SetArray( (unsigned char *) key, strlen(key) );
	dict->erase( bKey );
	//bKey.StealArray();
}

unsigned char *BencodedDict::Serialize(size_t *len)
{
	assert(bencType == BENC_DICT);
	INVARIANT_CHECK;

	return SerializeBencEntity(this, len);
}

BencEntityMem *BencodedDict::InsertString(const std::string& key, const std::string& str, int length /*=-1*/)
{
	// contiguous if string length > 0
	assert(!key.empty() && !str.empty());
	return InsertString(&(key[0]), &(str[0]), length);
}

BencEntityMem *BencodedDict::InsertString(const char* key, const char* str, int length /*=-1*/)
{
	assert(bencType == BENC_DICT);
	BencEntityMem beM;
	beM.SetStr( str, length );
	return (BencEntityMem *) Insert( key, beM );
}

BencEntityMem *BencodedDict::InsertStringT(const char* key, ctstr tstr)
{
	assert(bencType == BENC_DICT);
	BencEntityMem beM;
	beM.SetStrT( tstr );
	return (BencEntityMem *) Insert( key, beM );
}

BencEntity *BencodedDict::InsertInt(const char* key, int val)
{
	assert(bencType == BENC_DICT);
	BencEntity beInt;
	beInt.SetInt( val );
	return Insert( key, beInt );
}

BencEntity *BencodedDict::InsertInt64(const char* key, int64 val)
{
	assert(bencType == BENC_DICT);
	BencEntity beInt;
	beInt.SetInt64( val );
	return Insert( key, beInt );
}

BencodedDict *BencodedDict::InsertDict(const char* key)
{
	assert(bencType == BENC_DICT);
	BencodedDict beD;
	return (BencodedDict *) Insert( key, beD);
}
BencodedList *BencodedDict::InsertList(const char* key)
{
	assert(bencType == BENC_DICT);
	BencodedList beL;
	return (BencodedList *) Insert(key, beL);
}
