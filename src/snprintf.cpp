#if defined WIN32

#include <stdarg.h>

int snprintf(char* buf, int len, char const* fmt, ...)
{
   va_list lp;
   va_start(lp, fmt);
   int ret = _vsnprintf(buf, len, fmt, lp);
   va_end(lp);
   if (ret < 0) { buf[len-1] = 0; ret = len-1; }
   return ret;
}
#endif
