#ifndef _TARGET_H__
#define _TARGET_H__

// Define BT_LITTLE_ENDIAN to be 1 for a LE target and 0 for BE.
// Use #if BT_LITTLE_ENDIAN and !BT_LITTLE_ENDIAN to check for endianness.
#if defined(BYTE_ORDER)
#  if BYTE_ORDER==LITTLE_ENDIAN
#    define BT_LITTLE_ENDIAN 1
#  elif BYTE_ORDER==BIG_ENDIAN
#    define BT_LITTLE_ENDIAN 0
#  else
#    error "Unable to determine byte order based on value of BYTE_ORDER"
#  endif
#elif defined(WIN32)
#  define BT_LITTLE_ENDIAN 1
#else
#  error "Unable to determine byte order"
#endif

#if defined(__ppc__) || defined(__ppc64__)
#if BT_LITTLE_ENDIAN
#error "I think ppc should be big endian"
#endif
#endif

// Define STRICT_ALIGN for processors with hard alignment requirements.
// Some of these can run properly without this, but with degraded performance.
// (Degraded as in trapping into the kernel to perform an unaligned memory
// access.)
// PACKED is intended for "convenience" packing.  Places where
// performance is slightly lower but with improved memory usage.  They're
// no-ops where packing breaks things.
// Some places still require packing, such as to parse the 6-byte compact
// responses from trackers.  STRICT_ALIGN tells us whether we can walk
// this as a plain array or need to copy out bytes into an aligned temporary
// because the array won't match the data layout.

#if defined(__sparc__) || defined(__arm__) || defined(__mips__) || defined(__ppc__) || defined(__ppc64__) || defined(__sh__)
#  define STRICT_ALIGN 1
#  define PACKED
#else
#  ifdef _MSC_VER
#    define PACKED __declspec(align(1))
#  else
#    define PACKED  __attribute__((packed))
#  endif
#endif

#endif
