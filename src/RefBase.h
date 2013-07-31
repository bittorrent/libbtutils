#ifndef _UTORRENT_REFBASE_H_
#define _UTORRENT_REFBASE_H_

#ifdef WIN32
#include <windows.h> // for STDMETHODCALLTYPE
#else
#define STDMETHODCALLTYPE
#endif

#include "utypes.h" // for LONG

///
/// An intrusive base class for reference counting, along with a macro, REFBASE, which
/// provides COM style AddRef and Release methods in an implementation class.  We do
/// this so that the ultimate child class can implement AddRef and Release from all
/// bases that require them.
///
/// Use it like this:
///
/// \code
/// class SomeClass : public ISomeInterface, public IOtherInterface, public RefBase
/// {
/// public:
///     REFBASE;
///
///     ...
/// };
/// \endcode
/// This will make SomeClass implement AddRef and Release inherited from every interface.
///
class RefBase
{
public:
	RefBase() : refcount(0) { }
	virtual ~RefBase() { }
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

private:
	LONG refcount;
};

#define REFBASE ULONG STDMETHODCALLTYPE AddRef() { return RefBase::AddRef(); } ULONG STDMETHODCALLTYPE Release() { return RefBase::Release(); }

#endif//_UTORRENT_REFBASE_H_
