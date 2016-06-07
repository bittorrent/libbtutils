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
#ifndef __SMART_PTR_H__
#define __SMART_PTR_H__

template <typename Interface>
class smart_ptr
{
	typedef void (smart_ptr::*bool_type)() const;
	void this_type_does_not_support_comparisons() const {}
public:
	smart_ptr() {
		ptr = NULL;
	}
	smart_ptr(const smart_ptr& com) {
		ptr = com.get();
		if (ptr) ptr->AddRef();
	}
	explicit smart_ptr(Interface* p) {
		ptr = p;
		if (ptr) {
			ptr->AddRef();
		}
	}
	~smart_ptr() {
		if (ptr) {
			ptr->Release();
		}
	}

	smart_ptr & operator=(smart_ptr const & r) {
		if (ptr != r.ptr) {
			if (r.ptr) r.ptr->AddRef();
			if (ptr) ptr->Release();
			ptr = r.ptr;
		}
		return *this;
	}

	smart_ptr & operator=(Interface* p) {
		if(ptr != p) {
			if (p) p->AddRef();
			if (ptr) ptr->Release();
			ptr = p;
		}
		return *this;
	}

	template<class Y> smart_ptr & operator=(smart_ptr<Y> const & r) {
		if(ptr != r.ptr) {
			if (r.ptr) r.ptr->AddRef();
			if (ptr) ptr->Release();
			ptr = r.ptr;
		}
		return *this;
	}

	template<class Y> smart_ptr(smart_ptr<Y> const & r) {
		ptr = r.get();
		if (ptr) ptr->AddRef();
	}

	Interface* operator->() const { return ptr; }
	bool operator==(smart_ptr<Interface> const& c) const { return c.ptr == ptr; }
	bool operator==(Interface* p) const { return p == ptr; }
	bool operator!=(smart_ptr<Interface> const& c) const { return c.ptr != ptr; }
	bool operator!=(Interface* p) const { return p != ptr; }
	bool operator <(smart_ptr<Interface> const &c) const { return ptr < c.ptr; }
	//purely here for convention's sake
	Interface* get() const { return ptr; }

	Interface** operator&() const
	{ return (Interface**)&ptr; }

	void reset(Interface* p = NULL)
	{
		// Exempt p == ptr since ptr would be released (and maybe deleted) before it is addref'd back
		if (p == ptr) return;
		if (ptr) ptr->Release();
		ptr = p;
		if (ptr) ptr->AddRef();
	}

	void swap(smart_ptr& p) {
		Interface* t = p.ptr;
		p.ptr = ptr;
		ptr = t;
	}
	operator bool_type() const {
		return ptr ? &smart_ptr::this_type_does_not_support_comparisons : 0;
	}

	Interface& operator*() { return *ptr; }
	Interface const& operator*() const { return *ptr; }

protected:
	Interface* ptr;
};


#endif

