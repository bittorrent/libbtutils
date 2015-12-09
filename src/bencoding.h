#ifndef __BENCODING2_H__
#define __BENCODING2_H__

//#include "bt_string.h" // for strncmp_exact
//#include "avltree.h"	// Map
#include <algorithm> // std::mismatch
#include <cassert>
#include <cwchar>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <utility>	// std::pair
#include "bencparser.h"
//#include "string_type.h" // namespace aux_
//#include "util.h"
#include "enumtype.h"
//#include "templates.h"
#include "utypes.h" // for tstr, ctstr etc.

#include "RefBase.h" // for BencEntity RefBase

#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif // _UNICODE

ENUM_TYPE(BENC_T, uint8) {
	BENC_VOID 	      = 0,
	BENC_INT 	      = 1,
	BENC_BIGINT       = 2,
	BENC_STR 	      = 3,
	BENC_LIST 	      = 4,
	BENC_DICT 	      = 5,
	BENC_VLIST 	      = 6,
	BENC_STR_INPLACE  = 7,
	BENC_INT_LAZY	  = 8,
	BENC_NULL = 0xfd,
	BENC_BOOL = 0xfe,
	BENC_DOUBLE = 0xff,
	BENC_ABSOLUTE_MAX = 0xff
};

class BencEntity;
class BencEntityMem;
class BencodedList;
class BencodedDict;

// This is just because the key is char and the data is byte
// TODO: This should _really_ just be a std::string
template< typename T > class BencArray
{
protected:
	std::vector<T> _arr;
public:
	BencArray() { Clear(); }
	BencArray( const BencArray<T> &b ) {
		// BencArrays must always be null terminated
		assert(b._arr.size() >= 1);
		_arr = b._arr;
	}
	BencArray( T *p, int count ) {
		_arr.reserve(count + 1);
		_arr.assign(p, p + count);
		_arr.push_back(0);
	}
	bool operator<(const BencArray<T>& arg) const {
		return _arr < arg._arr;
	}
	BencArray<T>& operator=(const BencArray<T>& arg) {
		assert(arg._arr.size() >= 1);
		_arr = arg._arr;
		return *this;
	}
	T& operator[] (size_t index) {
		return _arr[index];
	}
	const T& operator[] (size_t index) const {
		return _arr[index];
	}
	// -1 is to disregard the null terminator
	size_t GetCount() const {return _arr.size()-1;}
	void Clear() { _arr.clear(); _arr.push_back(0); }
	void Append(const T *p, int count)
	{
		assert(_arr.size() >= 1);
		// remove null terminator
		_arr.reserve(_arr.size() + count);
		_arr.erase(_arr.end()-1);
		_arr.insert(_arr.end(), p, p + count);
		// null terminate
		_arr.push_back(0);
	}
	void AppendTerminated(const T *p, int count) {
		this->Append(p, count);
	}
	void Resize(size_t size) { _arr.resize(size+1); _arr[size] = 0; }
	void SetArray(T *p, size_t len)
	{
		assert(len % sizeof(T) == 0);
		_arr.reserve(len / sizeof(T) + 1);
		_arr.assign(p, p + len / sizeof(T));
		_arr.push_back(0);
	}
	const char* GetRaw() const { return (const char*)&this->_arr[0];}
};

typedef BencArray<unsigned char> BencKey;
typedef BencArray<unsigned char> BencodedMem;
typedef std::map<BencKey, BencEntity> BencodedEntityMap;
typedef std::vector<BencEntity> BencodedEntityList;

typedef void (*BencVListCallback)(void *user, size_t i, BencEntity *result);
struct VListData;

class BencodedEmitterBase {
protected:
	std::ostringstream _emit_buf;
public:
	BencodedEmitterBase() { };
	void EmitChar(char);
	void Emit(const void *a, size_t len);
	std::string GetResult();
};

class BencodedEmitter : public BencodedEmitterBase {
public:
	virtual ~BencodedEmitter() {}
	virtual void EmitEntity(const BencEntity *e);
};

class BencEntity : public RefBase {
public:
	REFBASE
	union {
		int64 num;
		BencodedMem *mem;
		// Base member maintained by subclass BencodedList
		BencodedEntityList *list;
		VListData *vlist;
		// Base member maintained by subclass BencodedDict
		BencodedEntityMap *dict;
	};
	BENC_T bencType;

	class AllocRegime {
	public:
		virtual ~AllocRegime(){};	// required for GCC
		virtual BencKey* NewKey( unsigned char *p, int count ) = 0;
		virtual BencodedMem* NewMem( unsigned char *p, int count ) = 0;
		virtual BencodedMem* NewStr( unsigned char *p, int count ) = 0;
		virtual bool LazyInt(void){ return false; }
	};

	class AllocateMemRegime : virtual public AllocRegime {
	public:
		virtual ~AllocateMemRegime(){};	// required for GCC
		virtual BencKey* NewKey( unsigned char *p, int count ) {
			return new BencKey(p, count); // allocate
		}
		virtual BencodedMem* NewMem( unsigned char *p, int count ) {
			return new BencodedMem( p, count ); 	// allocate
		}
		virtual BencodedMem* NewStr( unsigned char *p, int count ) {
			// null terminate the string
			BencodedMem* ret = new BencodedMem( p, count + 1); // allocate
			(*ret)[ret->GetCount()-1] = 0;
			ret->Resize(count);
			return ret;
		}
	};

	class InplaceMemRegime : virtual public AllocRegime {
	public:
		virtual ~InplaceMemRegime(){};	// required for GCC
		virtual BencKey* NewKey( unsigned char *p, int count ) {
			return new BencKey(p, count);
		}
		virtual BencodedMem* NewMem( unsigned char *p, int count ) {
			return new BencodedMem(p, count); 	// allocate
		}
		virtual BencodedMem* NewStr( unsigned char *p, int count ) {
			return new BencodedMem(p, count); 	// allocate
		}
		virtual bool LazyInt(void){ return AllocRegime::LazyInt(); }
	};

	class InplaceMemLazyIntRegime : public InplaceMemRegime {
	public:
		virtual ~InplaceMemLazyIntRegime(){};	// required for GCC
		virtual bool LazyInt(void){ return true; }
	};

	BencEntity( BENC_T argType ) : bencType(argType) {
		mem = NULL;
		list = NULL;
		dict = NULL;
		num = 0;
	}

	BencEntity() {
		ZeroOut();
	}
	BencEntity(const BencEntity&);
	BencEntity& operator=(const BencEntity&);
	void MoveFrom(BencEntity&);

	void Print(bool oneline = false, int indent = 0);
	void CopyFrom(const BencEntity& b);
	void ZeroOut();

	~BencEntity() {
#ifndef BT_STD_MAP
		FreeMembers();
#endif
//#ifdef _DEBUG
//		assert( inplace ||
//				((type==BENC_STR && !mem) || (type==BENC_LIST && !list) || (type==BENC_DICT && !dict.list)) ||
//				(type==BENC_VOID || type==BENC_INT) || (type==BENC_VLIST && !vlist));
//#endif
	}

	BENC_T GetType() const { return bencType; }

	void SetInt64(int64 i);
	void SetInt(int i);
	int GetInt(int def = 0) const;
	int64 GetInt64(int64 def = 0) const;

//	BencodedList *SetList();
	BencodedList *SetVList(BencVListCallback callback, size_t count, void *user);
//	BencodedDict *SetDict();

	static BencodedList *AsList(BencEntity *e) { return e && e->bencType == BENC_LIST ? (BencodedList*)e : NULL; }
	static const BencodedList *AsList(const BencEntity *e) { return e && (e->bencType == BENC_LIST || e->bencType == BENC_VLIST) ? (BencodedList*)e : NULL; }
	static BencodedDict *AsDict(BencEntity *e) { return e && e->bencType == BENC_DICT ? (BencodedDict*)e : NULL; }
	static const BencodedDict *AsDict(const BencEntity *e) { return e && e->bencType == BENC_DICT ? (BencodedDict*)e : NULL; }
	static BencEntityMem *AsBencString(BencEntity *e) { return e && e->bencType == BENC_STR ? (BencEntityMem*)e : NULL; }
	static const BencEntityMem *AsBencString(const BencEntity *e) { return e && e->bencType == BENC_STR ? (BencEntityMem*)e : NULL; }

	static const unsigned char *ParseInPlace(unsigned char *p, BencEntity &ent, const unsigned char *pend);
	static const unsigned char *ParseInPlace(const unsigned char *p, BencEntity &ent, const unsigned char *pend, const char *key, std::pair<unsigned char*, unsigned char*> *rgn);
	static const unsigned char *ParseInPlace(const unsigned char *p, BencEntity &ent, const unsigned char *pend, std::vector<const char *> const &keys, std::pair<unsigned char*, unsigned char*> *rgn);
	static const unsigned char *Parse(const unsigned char *p, BencEntity &ent, const unsigned char *pend);
	static const unsigned char *Parse(const unsigned char *p, BencEntity &ent, const unsigned char *pend, const char *key, std::pair<unsigned char*, unsigned char*> *rgn);
	static const unsigned char *Parse(const unsigned char *p, BencEntity &ent, const unsigned char *pend, std::vector<const char*> const &keys, std::pair<unsigned char*, unsigned char*> *rgn);
	bool ParseFlat(BencEntity &ent, IBencParser *pParser, AllocRegime *regime);
	bool SetParsed( IBencParser::PARSE_T parseResult, const unsigned char *pElement, size_t size, AllocRegime *regime );

	void FreeMembers();
protected:
	void ParseNum(const unsigned char *p);
private:
	static bool DoParse(BencEntity &ent, IBencParser *pParser, AllocRegime *regime);
};

std::string SerializeBencEntity(const BencEntity* entity);

class BencEntityMem : public BencEntity {
public:
	BencEntityMem(): BencEntity( BENC_STR ){
		mem = new BencodedMem();
	}
	BencEntityMem( BencodedMem *pMem ): BencEntity( BENC_STR ){
		mem = pMem;
	}
	BencEntityMem(ctstr memArg, size_t len = ~0);
	BencEntityMem(const void *memArg, size_t len = ~0): BencEntity( BENC_STR ){
		assert(memArg);

		if( len == static_cast<size_t>(~0) )
			len = strlen( (const char *) memArg );
		mem = new BencodedMem( (unsigned char *) memArg, static_cast<int>(len));
	}

	void SetStr(const char * s, int len = -1);

	// make sure we don't access element 0 in an empty array
	// because that's not allowed (and triggers assertions when
	// debug iterators are enabled).
	char const *GetRaw(void) const { return mem->GetCount() == 0 ? NULL : (char const*)&(*mem)[0]; }

	size_t GetSize(void) const { return mem->GetCount(); }
	void CopyFrom(const BencEntity& b);

	const char * GetString(size_t *count) const {
		if (!(bencType == BENC_STR)) return NULL;
		if (count) *count = GetSize();
		//if (mem->IsInplace() && GetSize()) GetRaw()[GetSize()] = 0;
		return GetSize() ? GetRaw() : "";
	}

	tstring GetStringT(int encoding, size_t *count) const;

	// Sets a unicode string, internally converts to utf-8
	void SetStrT(ctstr s);

	void SetMem(const void *mem, size_t len);
	void SetMem( BencodedMem *pMem )
	{
		assert(bencType == BENC_INT_LAZY || bencType == BENC_STR);
		delete mem;
		mem = pMem;
	}
	void SetMemOwn(void *mem, size_t len);
	void FreeMembers();
};

class BencEntityLazyInt :  public BencEntityMem {
public:
	BencEntityLazyInt(){
		bencType = BENC_INT_LAZY;
		mem = new BencodedMem();
		//mem->Init();
	}
	BencEntityLazyInt( BencodedMem *pMem ): BencEntityMem( pMem ){
		bencType = BENC_INT_LAZY;
		mem = pMem;
	}
	int GetInt(int def = 0);
	int64 GetInt64(int64 def = 0);
};

struct VListData {
	BencVListCallback callback;
	void *user;
	size_t focus;
	BencEntity ent;
	VListData(BencVListCallback callback, void *user) : callback(callback), user(user), focus(~0), count(0) { }
	size_t count;
};

class BencodedList : public BencEntity {
public:
	BencodedList(): BencEntity( BENC_LIST ){
		list = new BencodedEntityList();
		//list->Init();
	}

	BencEntity *Get(size_t i) const;
	size_t GetCount() const;
	BencodedDict* GetDict(size_t i);
	BencodedDict const *GetDict(size_t i) const;
	BencodedList* GetList(size_t i);
	BencodedList const *GetList(size_t i) const;

	const char * GetString(size_t i, size_t *length = NULL) const;
	// See comment at GetStringT()
	tstring GetStringT(size_t i, int encoding = 0, size_t *length = NULL) const;

	int GetInt(size_t i, int def = 0) const;
	int64 GetInt64(size_t i, int64 def = 0) const;
	BencEntity *Append(BencEntity &e);
	BencEntityMem *AppendString(const char * str, size_t length = -1);
	BencEntityMem *AppendStringT(ctstr str, size_t length = -1);
	BencEntity *AppendInt(int arg);
	BencEntity *AppendInt64(int64 arg);
	BencodedDict *AppendDict();
	BencodedList *AppendList();
	void Delete(size_t i);
	void FreeMembers();
	bool ResumeList(IBencParser *pParser, BencEntity **ent, AllocRegime *regime);
	void CopyFrom(const BencEntity& b);

	std::string Serialize();

protected:
	void grow(unsigned int num);
private:
	BencodedList(const BencodedList&);
};

// TODO: make all these classes consistent! If it's not supposed to be
// copyable, make it non-copy-assignable. Make them all movable!
class BencodedDict : public BencEntity {
public:
	BencodedDict(): BencEntity( BENC_DICT ){ dict = new BencodedEntityMap(); }

	size_t GetCount() const { return dict->size(); }
	const BencEntity *Get(const char * key, int len = -1) const;

	// Same as above, except with de-const wrappage
	BencEntity* Get(const char * key, int len = -1) {
		const BencodedDict* me = static_cast<const BencodedDict*>(this);
		return const_cast<BencEntity*>(me->Get(key, len));
	}

	BencodedDict* GetDict(const char* key, int len = -1);
	BencodedDict const* GetDict(const char* key, int len = -1) const;
	BencodedList* GetList(const char* key, int len = -1);
	BencodedList const* GetList(const char* key, int len = -1) const;
	const char* GetString(const char* key, size_t *length = NULL) const;

	// Like GetString, but returns a malloc'ed copy
	char * GetStringCopy(const char * key) const;

	// See comment at GetStringT()
	tstring GetStringT(const char * key, int encoding = 0, size_t *length = NULL) const;
	char * GetString(const char * key, size_t length) const;
	int GetInt(const char * key, int def = 0) const;
	BencEntityMem *InsertString(const char * key, const char * str, int length=-1);
	BencEntityMem *InsertStringT(const char * key, ctstr tstr);
	BencEntityMem *InsertString(const std::string& key, const std::string& str, int length =-1);
	BencEntity *InsertInt(const char * key, int arg);
	BencEntity *InsertInt64(const char * key, int64 arg);
	BencodedDict *InsertDict(const char * key, int len = -1);
	BencodedList *InsertList(const char * key, int len = -1);

	int64 GetInt64(const char * key, int64 def = 0) const;
	bool HasKey(const char * key, int len = -1) const { return Get(key, len) != NULL; }

	BencEntity *Insert(const char* key, int len, BencEntity& e);
	BencEntity *Insert(const char* key, BencEntity& e)
	{ return Insert(key, -1, e); }
	BencEntityMem *AppendMultiple(char * key, bool allowmultiple = true);
	void Delete(const char * key);
	void CopyFrom(const BencEntity& b);

	std::string Serialize();

#ifdef _DEBUG
	void check_invariant() const;
#endif


	void FreeMembers();
	bool ResumeDict(IBencParser *pParser, BencEntity **ent, AllocRegime *regime);
private:
	BencodedDict(const BencodedDict&);

};

bool BencEntityIsValid(unsigned char *b, size_t len, void *userdata);

#endif //__BENCODING2_H__
