#include "bitfield.h"

reference BitField::operator [](size_t i)
{
	return reference(*this, i);
}
