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

