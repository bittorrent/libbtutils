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
#ifndef _UTORRENT_REFBASE_H_
#define _UTORRENT_REFBASE_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // for STDMETHODCALLTYPE
#undef WIN32_LEAN_AND_MEAN
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
