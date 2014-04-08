#ifndef TAILQUEUE_H
#define TAILQUEUE_H

#include <stddef.h> // for offsetof()

template<typename T, size_t offs>  class TailQueueX {
public:
	T *_first, **_last;
	static T **next(T *a) { return (T**)((byte*)a + offs); }

public:
	void unlinknext(T **a) { if (!(*a = (*a)->next)) _last = a;	  }
	void unlinkhead() { unlinknext(&_first); }

	void init() { _first = NULL; _last = &_first; }
	void enqueue(T *a) { *_last = a; _last = next(a); *_last = NULL; }
	T *dequeue() { assert(_first); T *f = _first; unlinknext(&_first); return f; }
	bool empty() { return _first == NULL; }
	T *last() { assert(_first); return (T*)((byte*)_last - offs); }
	T *&first() { return _first; }
	const T *last() const { assert(_first); return (T*)((byte*)_last - offs); }
	const T *first() const { return _first; }
	void enqueue_head(T *a) { if (!(a->next = _first)) _last = &a->next; _first = a; }
	void swap(TailQueueX& rhs)
	{
		// this is non-trivial since _last is potentially
		// self-referencial
		T** tmp2 = _last;
		if (rhs._last != &rhs._first) _last = rhs._last; else _last = &_first;
		if (tmp2 != &_first) rhs._last = tmp2; else rhs._last = &rhs._first;

		T* tmp = rhs._first;
		rhs._first = _first;
		_first = tmp;
	}
	// prepend the specified list to this list
	void enqueue_head(TailQueueX& a)
	{
		if (a.empty()) return;
		if (_last == &_first) _last = a._last;
		*a._last = _first;
		_first = a._first;
		a.init();
	}

	// append the specified list to this list
	void enqueue(TailQueueX& a)
	{
		if (a.empty()) return;
		*_last = a._first;
		_last = a._last;
		a.init();
	}

#ifdef _DEBUG_MEM_LEAK
	void clean_up(){
		BtScopedLock network_lock;
		while(!empty()) {delete(dequeue());}
		init();
	}
#endif
	void copy_from(TailQueueX<T,offs> *a) {
		*this = *a;
		if (a->_last == &a->_first)
			_last = &_first;
	}

};
#define TailQueue(T,next) TailQueueX<T, offsetof(T, next)>

#endif // TAILQUEUE_H

