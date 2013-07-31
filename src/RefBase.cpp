#include "StdAfxCore.h"
#include "interlock.h"
#include "RefBase.h"

STDMETHODIMP_(ULONG) RefBase::AddRef()
{
	return InterlockedIncrement(&refcount);
}

STDMETHODIMP_(ULONG) RefBase::Release()
{
	ULONG Result = InterlockedDecrement(&refcount);
	if (!Result)
		delete this;
	return Result;
}
