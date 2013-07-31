#include "interlock.h"
#include "RefBase.h"

ULONG STDMETHODCALLTYPE RefBase::AddRef()
{
	return InterlockedIncrement(&refcount);
}

ULONG STDMETHODCALLTYPE RefBase::Release()
{
	ULONG Result = InterlockedDecrement(&refcount);
	if (!Result)
		delete this;
	return Result;
}
