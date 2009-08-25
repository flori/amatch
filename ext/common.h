#ifndef __COMMON_H__
#  define __COMMON_H__

#ifndef RSTRING_PTR
#define RSTRING_PTR(str) (RSTRING(str)->ptr)
#endif

#ifndef RSTRING_LEN
#define RSTRING_LEN(str) (RSTRING(str)->len)
#endif

#ifndef RARRAY_PTR
#define RARRAY_PTR(ary) (RARRAY(ary)->ptr)
#endif

#ifndef RARRAY_LEN
#define RARRAY_LEN(ary) (RARRAY(ary)->len)
#endif

#ifndef RFLOAT_VALUE
#define RFLOAT_VALUE(val) (RFLOAT(val)->value)
#endif


#endif
