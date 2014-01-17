
#include "interlock.h"

#ifdef __APPLE__

#include <libkern/OSAtomic.h>
OSSpinLock g_interlocked_spin;

#elif !USE_GLIBCXX_ATOMIC_BUILTINS && !defined _WIN32

#include <pthread.h>
pthread_mutex_t g_interlocked_mutex = PTHREAD_MUTEX_INITIALIZER;

#endif // !USE_GLIBCXX_ATOMIC_BUILTINS

