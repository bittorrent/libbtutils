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
#include <utility>	// std::pair
#include "bencparser.h"
//#include "string_type.h" // namespace aux_
//#include "util.h"
#include "enumtype.h"
//#include "templates.h"
#include "utypes.h" // for tstr, ctstr etc.

#ifdef _UNICODE
typedef std::wstring t_string;
#else
typedef std::string t_string;
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
template< typename T > class BencArray
{
protected:
	std::vector<T> _arr;
public:
	// These are necessary for Posix/GCC (wtf?)
	/*
	using LList<T>::Init;
	using LList<T>::Guarantee;
	using LList<T>::Append;
	using LList<T>::mem;
	using LList<T>::SetArray;
	using LList<T>::StealArray;
	using LList<T>::GetCount;
	using LList<T>::Free;
	*/

	BencArray() {}
	BencArray( const BencArray<T> &b ) {
		this->_arr = b._arr;
	}
	BencArray( T *p, int count ) {
		this->_arr.assign(p, p + count * sizeof(T));
	}
	bool operator<(const BencArray<T>& arg) const {
		return (this->_arr<arg._arr)?true:false;
		/*std::pair<T*, T*> match;
		match = std::mismatch(this->_arr.begin(), this->_arr.end(), arg._arr.begin());
		if (match.first == match.second)
			return (this->_arr.size() < arg._arr.size())?true:false;
		return (match.first < match.second)?true:false;*/
	}
	BencArray<T>& operator=(const BencArray<T>& arg) {
		this->_arr = arg._arr;
		return *this;
	}
	T& operator[] (size_t index) {
		return this->_arr[index];
	}
	const T& operator[] (size_t index) const {
		return this->_arr[index];
	}
	size_t GetCount() const {return this->_arr.size();}
	void Clear() {this->_arr.clear();}
	void Append(const T *p, int count) { this->_arr.insert(this->_arr.end(), p, p + count*sizeof(p)); }
	void AppendTerminated(const T *p, int count) { this->Append(p, count-1); this->Append(0, 1); }
	void Resize(size_t size) { this->_arr.resize(size); }
	// whyyyy is len in bytes?
	void SetArray(T *p, size_t len) { assert(len%sizeof(p)==0); this->_arr.assign(p, p + len); }
	const char* GetRaw() const { return (const char*)&this->_arr[0];}
};

typedef BencArray<unsigned char> BencKey;
typedef BencArray<unsigned char> BencodedMem;
typedef std::map<BencKey, BencEntity> BencodedEntityMap;
typedef std::vector<BencEntity> BencodedEntityList;

typedef void (*BencVListCallback)(void *user, size_t i, BencEntity *result);
struct VListData;

class BencodedEmitterBase {
	std::vector<char> _emit_buf;
public:
	BencodedEmitterBase() { _emit_buf.reserve(4096); };
	void EmitChar(char);
	void Emit(const void *a, size_t len);
	unsigned char* GetResult(size_t* len);
	virtual void EmitEntity(const BencEntity *e) = 0;
};

class BencodedEmitter : public BencodedEmitterBase {
public:
	virtual void EmitEntity(const BencEntity *e);
};

class BencEntity {
public:
	union {
		int64_t num;
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
		virtual bool LazyInt(void){ return false; }
	};

	class AllocateMemRegime : virtual public AllocRegime {
	public:
		virtual ~AllocateMemRegime(){};	// required for GCC
		virtual BencKey* NewKey( unsigned char *p, int count ) {
			return new BencKey( p, count ); 	// allocate
		}
		virtual BencodedMem* NewMem( unsigned char *p, int count ) {
			return new BencodedMem( p, count ); 	// allocate
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
	void MoveFrom(BencEntity&);
//	BencEntity &ListGet(size_t i) const;
//
	void Print(int indent = 0);
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

	void SetInt64(int64_t i);
	void SetInt(int i);
	int GetInt(int def = 0) const;
	int64_t GetInt64(int64_t def = 0) const;

//	BencodedList *SetList();
	BencodedList *SetVList(BencVListCallback callback, size_t count, void *user);
//	BencodedDict *SetDict();

	static BencodedList *AsList(BencEntity *e) { return e && e->bencType == BENC_LIST ? (BencodedList*)e : NULL; }
	static const BencodedList *AsList(const BencEntity *e) { return e && (e->bencType == BENC_LIST || e->bencType == BENC_VLIST) ? (BencodedList*)e : NULL; }
	static BencodedDict *AsDict(BencEntity *e) { return e && e->bencType == BENC_DICT ? (BencodedDict*)e : NULL; }
	static const BencodedDict *AsDict(const BencEntity *e) { return e && e->bencType == BENC_DICT ? (BencodedDict*)e : NULL; }
	static BencEntityMem *AsBencString(BencEntity *e) { return e && e->bencType == BENC_STR ? (BencEntityMem*)e : NULL; }
	static const BencEntityMem *AsBencString(const BencEntity *e) { return e && e->bencType == BENC_STR ? (BencEntityMem*)e : NULL; }

/*	unsigned char *SerializeAsText(size_t *len = NULL) const;
	char * SerializeAsXML(const char * tag, size_t* len = NULL) const;
	char * SerializeAsJson(size_t* len = NULL) const;
	char * SerializeAsAscii(size_t* len = NULL) const;
	char * SerializeByMimeType(const char * mime_type, const char * tag, const char *& encoding, const char * json_callback = NULL);*/

	//int LoadFromFile_Safe(ctstr filename);

	// Parse a list of web RPC params of the form:
	//  action?param1=val1?param2=val2 ...
	// into a dict:
	//  dict {
	//	  "action" = dict {
	//	  "param1" = string "val1"
	//	  "param2" = string "val2"
	//	  ...
	//	  }
	//  }
// Hmm, does a BencEntity need to know about RPC format?
//	BencodedDict* ParseRpcParams(char * paramlist, bool allowmultiple = false);

	static const unsigned char *ParseInPlace(unsigned char *p, BencEntity &ent, const unsigned char *pend);
	static const unsigned char *ParseInPlace(const unsigned char *p, BencEntity &ent, const unsigned char *pend, const char *key, std::pair<unsigned char*, unsigned char*> *rgn);
	static const unsigned char *Parse(const unsigned char *p, BencEntity &ent, const unsigned char *pend);
	static const unsigned char *Parse(const unsigned char *p, BencEntity &ent, const unsigned char *pend, const char *key, std::pair<unsigned char*, unsigned char*> *rgn);
	bool ParseFlat(BencEntity &ent, IBencParser *pParser, AllocRegime *regime);
	bool SetParsed( IBencParser::PARSE_T parseResult, const unsigned char *pElement, size_t size, AllocRegime *regime );

	void FreeMembers();
protected:
	void ParseNum(const unsigned char *p);
private:
	static bool DoParse(BencEntity &ent, IBencParser *pParser, AllocRegime *regime);
};

unsigned char* SerializeBencEntity(const BencEntity* entity, size_t* len);

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
	char const *GetRaw(void) const { return (char const*)&(*mem)[0]; }
	size_t GetSize(void) const { return mem->GetCount(); }
	void CopyFrom(const BencEntity& b);

	const char * GetString(size_t *count) const {
		if (!(bencType == BENC_STR)) return NULL;
		if (count) *count = GetSize();
		//if (mem->IsInplace() && GetSize()) GetRaw()[GetSize()] = 0;
		return GetSize() ? GetRaw() : "";
	}

	t_string GetStringT(int encoding, size_t *count) const;

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
	int64_t GetInt64(int64_t def = 0);
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
	BencodedDict *GetDict(size_t i);
	BencodedList *GetList(size_t i);
	const char * GetString(size_t i, size_t *length = NULL) const;
	// See comment at GetStringT()
	t_string GetStringT(size_t i, int encoding = 0, size_t *length = NULL) const;

	int GetInt(size_t i, int def = 0) const;
	int64_t GetInt64(size_t i, int64_t def = 0) const;
	BencEntity *Append(BencEntity &e);
	BencEntityMem *AppendString(const char * str, size_t length = -1);
	BencEntityMem *AppendStringT(ctstr str, size_t length = -1);
	BencEntity *AppendInt(int arg);
	BencEntity *AppendInt64(int64_t arg);
	BencodedDict *AppendDict();
	BencodedList *AppendList();
	void Delete(size_t i);
	void FreeMembers();
	bool ResumeList(IBencParser *pParser, BencEntity **ent, AllocRegime *regime);
	void CopyFrom(const BencEntity& b);

protected:
	void grow(unsigned int num);
private:
	BencodedList(const BencodedList&);
};

class BencodedDict : public BencEntity {
public:
	BencodedDict(): BencEntity( BENC_DICT ){ dict = new BencodedEntityMap(); }

	size_t GetCount() const { return dict->size(); }
	const BencEntity *Get(const char * key) const;

	// Same as above, except with de-const wrappage
	BencEntity* Get(const char * key) {
		const BencodedDict* me = static_cast<const BencodedDict*>(this);
		return const_cast<BencEntity*>(me->Get(key));
	}

	BencodedDict *GetDict(const char * key);
	BencodedList *GetList(const char * key);
	const char * GetString(const char * key, size_t *length = NULL) const;

	// Like GetString, but returns a malloc'ed copy
	char * GetStringCopy(const char * key) const;

	// See comment at GetStringT()
	t_string GetStringT(const char * key, int encoding = 0, size_t *length = NULL) const;
	char * GetString(const char * key, size_t length) const;
	int GetInt(const char * key, int def = 0) const;
	BencEntityMem *InsertString(const char * key, const char * str, int length=-1);
	BencEntityMem *InsertStringT(const char * key, ctstr tstr);
	BencEntityMem *InsertString(const std::string& key, const std::string& str, int length =-1);
	BencEntity *InsertInt(const char * key, int arg);
	BencEntity *InsertInt64(const char * key, int64_t arg);
	BencodedDict *InsertDict(const char * key);
	BencodedList *InsertList(const char * key);

	int64_t GetInt64(const char * key, int64_t def = 0) const;
	bool HasKey(const char * key) const { return Get(key) != NULL; }

	BencEntity *Insert(const char * key, BencEntity &);
	BencEntityMem *AppendMultiple(char * key, bool allowmultiple = true);
	void Delete(const char * key);
	void CopyFrom(const BencEntity& b);

	unsigned char *Serialize(size_t *len);

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
