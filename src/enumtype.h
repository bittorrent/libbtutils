#ifndef ENUMTYPE_H
#define ENUMTYPE_H

#ifdef __GNUC__
//#define ENUM_TYPE(NAME, TYPE) typedef TYPE NAME; enum
// GCC sucks. use -fshort-enums
#define ENUM_TYPE(NAME, TYPE) enum NAME
#define ENUM_TYPE_BITFIELD(NAME, TYPE) typedef NAME NAME##_BITFIELD;
#elif _MSC_VER >= VS_2005
	#define ENUM_TYPE(NAME, TYPE) enum NAME : TYPE
	#define ENUM_TYPE_BITFIELD(NAME, TYPE) typedef NAME NAME##_BITFIELD;
#else
	// fun! sized type that enforces enum type assignment
	#define ENUM_TYPE(NAME, TYPE)							\
		enum NAME##_ENUM;									\
															\
		struct NAME {										\
			NAME() {}										\
			explicit NAME (TYPE n) { v = (NAME##_ENUM)n; }	\
			NAME(NAME##_ENUM n) { v = n; }					\
			void operator=(NAME##_ENUM n) { v = n; }		\
			operator TYPE() const { return v; }				\
		private:											\
			TYPE v;											\
		};													\
															\
		enum NAME##_ENUM
	// can't do struct bitfields
	#define ENUM_TYPE_BITFIELD(NAME, TYPE) typedef TYPE NAME##_BITFIELD;
#endif

#endif // ENUMTYPE_H

