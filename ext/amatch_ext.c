#include "ruby.h"
#include "pair.h"
#include <ctype.h>
#include "common.h"

static VALUE rb_mAmatch, rb_mAmatchStringMethods, rb_cLevenshtein,
             rb_cDamerauLevenshtein, rb_cSellers, rb_cHamming,
             rb_cPairDistance, rb_cLongestSubsequence, rb_cLongestSubstring,
             rb_cJaro, rb_cJaroWinkler;

static ID id_split, id_to_f;

#define GET_STRUCT(klass)                 \
    klass *amatch;                        \
    Data_Get_Struct(self, klass, amatch);

#define DEF_ALLOCATOR(type)                                             \
static type *type##_allocate()                                          \
{                                                                       \
    type *obj = ALLOC(type);                                            \
    MEMZERO(obj, type, 1);                                              \
    return obj;                                                         \
}

#define DEF_CONSTRUCTOR(klass, type)                                    \
static VALUE rb_##klass##_s_allocate(VALUE klass2)                      \
{                                                                       \
    type *amatch = type##_allocate();                                   \
    return Data_Wrap_Struct(klass2, NULL, rb_##klass##_free, amatch);   \
}                                                                       \
VALUE rb_##klass##_new(VALUE klass2, VALUE pattern)                     \
{                                                                       \
    VALUE obj = rb_##klass##_s_allocate(klass2);                        \
    rb_##klass##_initialize(obj, pattern);                              \
    return obj;                                                         \
}

#define DEF_RB_FREE(klass, type)                            \
static void rb_##klass##_free(type *amatch)                 \
{                                                           \
    MEMZERO(amatch->pattern, char, amatch->pattern_len);    \
    xfree(amatch->pattern);                                 \
    MEMZERO(amatch, type, 1);                               \
    xfree(amatch);                                          \
}

#define DEF_PATTERN_ACCESSOR(type)                              \
static void type##_pattern_set(type *amatch, VALUE pattern)     \
{                                                               \
    Check_Type(pattern, T_STRING);                              \
    xfree(amatch->pattern);                                     \
    amatch->pattern_len = (int) RSTRING_LEN(pattern);           \
    amatch->pattern = ALLOC_N(char, amatch->pattern_len);       \
    MEMCPY(amatch->pattern, RSTRING_PTR(pattern), char,         \
        RSTRING_LEN(pattern));                                  \
}                                                               \
static VALUE rb_##type##_pattern(VALUE self)                    \
{                                                               \
    GET_STRUCT(type)                                            \
    return rb_str_new(amatch->pattern, amatch->pattern_len);    \
}                                                               \
static VALUE rb_##type##_pattern_set(VALUE self, VALUE pattern) \
{                                                               \
    GET_STRUCT(type)                                            \
    type##_pattern_set(amatch, pattern);                        \
    return Qnil;                                                \
}

#define DEF_ITERATE_STRINGS(type)                                   \
static VALUE type##_iterate_strings(type *amatch, VALUE strings,    \
    VALUE (*match_function) (type *amatch, VALUE strings))          \
{                                                                   \
    if (TYPE(strings) == T_STRING) {                                \
        return match_function(amatch, strings);                     \
    } else {                                                        \
        int i;                                                      \
        VALUE result;                                               \
        Check_Type(strings, T_ARRAY);                               \
        result = rb_ary_new2(RARRAY_LEN(strings));                  \
        for (i = 0; i < RARRAY_LEN(strings); i++) {                 \
            VALUE string = rb_ary_entry(strings, i);                \
            if (TYPE(string) != T_STRING) {                         \
                rb_raise(rb_eTypeError,                             \
                    "array has to contain only strings (%s given)", \
                    NIL_P(string) ?                                 \
                        "NilClass" :                                \
                        rb_class2name(CLASS_OF(string)));           \
            }                                                       \
            rb_ary_push(result, match_function(amatch, string));    \
        }                                                           \
        return result;                                              \
    }                                                               \
}

#define DEF_RB_READER(type, function, name, converter)              \
VALUE function(VALUE self)                                          \
{                                                                   \
    GET_STRUCT(type)                                                \
    return converter(amatch->name);                                 \
}

#define DEF_RB_WRITER(type, function, name, vtype, caster, converter, check)\
VALUE function(VALUE self, VALUE value)                                 \
{                                                                       \
    vtype value_ ## vtype;                                              \
    GET_STRUCT(type)                                                    \
    caster(value);                                                      \
    value_ ## vtype = converter(value);                                 \
    if (!(value_ ## vtype check))                                       \
        rb_raise(rb_eTypeError, "check of value " #check " failed");    \
    amatch->name = value_ ## vtype;                                     \
    return Qnil;                                                        \
}


#define CAST2FLOAT(obj) \
    if (TYPE(obj) != T_FLOAT && rb_respond_to(obj, id_to_f))    \
            obj = rb_funcall(obj, id_to_f, 0, 0);               \
        else                                                    \
            Check_Type(obj, T_FLOAT)
#define FLOAT2C(obj) (RFLOAT_VALUE(obj))

#define CAST2BOOL(obj)                  \
    if (obj == Qfalse || obj == Qnil)   \
        obj = Qfalse;                   \
    else                                \
        obj = Qtrue;
#define BOOL2C(obj) (obj == Qtrue)
#define C2BOOL(obj) (obj ? Qtrue : Qfalse)

#define OPTIMIZE_TIME                                   \
    if (amatch->pattern_len < RSTRING_LEN(string)) {    \
        a_ptr = amatch->pattern;                        \
        a_len = (int) amatch->pattern_len;              \
        b_ptr = RSTRING_PTR(string);                    \
        b_len = (int) RSTRING_LEN(string);              \
    } else {                                            \
        a_ptr = RSTRING_PTR(string);                    \
        a_len = (int) RSTRING_LEN(string);              \
        b_ptr = amatch->pattern;                        \
        b_len = (int) amatch->pattern_len;              \
    }

#define DONT_OPTIMIZE                                   \
        a_ptr = amatch->pattern;                        \
        a_len = (int) amatch->pattern_len;              \
        b_ptr = RSTRING_PTR(string);                    \
        b_len = (int) RSTRING_LEN(string);              \

/*
 * C structures of the Amatch classes
 */

typedef struct GeneralStruct {
    char        *pattern;
    int         pattern_len;
} General;

DEF_ALLOCATOR(General)
DEF_PATTERN_ACCESSOR(General)
DEF_ITERATE_STRINGS(General)

typedef struct SellersStruct {
    char        *pattern;
    int         pattern_len;
    double      substitution;
    double      deletion;
    double      insertion;
} Sellers;

DEF_ALLOCATOR(Sellers)
DEF_PATTERN_ACCESSOR(Sellers)
DEF_ITERATE_STRINGS(Sellers)

static void Sellers_reset_weights(Sellers *self)
{
    self->substitution = 1.0;
    self->deletion     = 1.0;
    self->insertion    = 1.0;
}

typedef struct PairDistanceStruct {
    char        *pattern;
    int         pattern_len;
    PairArray   *pattern_pair_array;
} PairDistance;

DEF_ALLOCATOR(PairDistance)
DEF_PATTERN_ACCESSOR(PairDistance)

typedef struct JaroStruct {
    char *pattern;
    int   pattern_len;
    int   ignore_case;
} Jaro;

DEF_ALLOCATOR(Jaro)
DEF_PATTERN_ACCESSOR(Jaro)
DEF_ITERATE_STRINGS(Jaro)

typedef struct JaroWinklerStruct {
    char   *pattern;
    int    pattern_len;
    int    ignore_case;
    double scaling_factor;
} JaroWinkler;

DEF_ALLOCATOR(JaroWinkler)
DEF_PATTERN_ACCESSOR(JaroWinkler)
DEF_ITERATE_STRINGS(JaroWinkler)

/*
 * Levenshtein edit distances are computed here:
 */

#define COMPUTE_LEVENSHTEIN_DISTANCE                                        \
    c = 0;                                                                  \
    p = 0;                                                                  \
    for (i = 1; i <= a_len; i++) {                                          \
        c = i % 2;                      /* current row */                   \
        p = (i - 1) % 2;                /* previous row */                  \
        v[c][0] = i;                    /* first column */                  \
        for (j = 1; j <= b_len; j++) {                                      \
            /* Bellman's principle of optimality: */                        \
            weight = v[p][j - 1] + (a_ptr[i - 1] == b_ptr[j - 1] ? 0 : 1);  \
            if (weight > v[p][j] + 1) {                                     \
                 weight = v[p][j] + 1;                                      \
            }                                                               \
            if (weight > v[c][j - 1] + 1) {                                 \
                weight = v[c][j - 1] + 1;                                   \
            }                                                               \
            v[c][j] = weight;                                               \
        }                                                                   \
    }

static VALUE Levenshtein_match(General *amatch, VALUE string)
{
    VALUE result;
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int *v[2], weight;
    int  i, j, c, p;

    Check_Type(string, T_STRING);
    DONT_OPTIMIZE

    v[0] = ALLOC_N(int, b_len + 1);
    v[1] = ALLOC_N(int, b_len + 1);
    for (i = 0; i <= b_len; i++) {
        v[0][i] = i;
        v[1][i] = i;
    }

    COMPUTE_LEVENSHTEIN_DISTANCE

    result = INT2FIX(v[c][b_len]);

    xfree(v[0]);
    xfree(v[1]);

    return result;
}

static VALUE Levenshtein_similar(General *amatch, VALUE string)
{
    VALUE result;
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int *v[2], weight;
    int  i, j, c, p;

    Check_Type(string, T_STRING);
    DONT_OPTIMIZE

    if (a_len == 0 && b_len == 0) return rb_float_new(1.0);
    if (a_len == 0 || b_len == 0) return rb_float_new(0.0);
    v[0] = ALLOC_N(int, b_len + 1);
    v[1] = ALLOC_N(int, b_len + 1);
    for (i = 0; i <= b_len; i++) {
        v[0][i] = i;
        v[1][i] = i;
    }

    COMPUTE_LEVENSHTEIN_DISTANCE

    if (b_len > a_len) {
        result = rb_float_new(1.0 - ((double) v[c][b_len]) / b_len);
    } else {
        result = rb_float_new(1.0 - ((double) v[c][b_len]) / a_len);
    }

    xfree(v[0]);
    xfree(v[1]);

    return result;
}

static VALUE Levenshtein_search(General *amatch, VALUE string)
{
    VALUE result;
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int *v[2], weight, min;
    int  i, j, c, p;

    Check_Type(string, T_STRING);
    DONT_OPTIMIZE

    v[0] = ALLOC_N(int, b_len + 1);
    v[1] = ALLOC_N(int, b_len + 1);
    MEMZERO(v[0], int, b_len + 1);
    MEMZERO(v[1], int, b_len + 1);

    COMPUTE_LEVENSHTEIN_DISTANCE

    for (i = 0, min = a_len; i <= b_len; i++) {
        if (v[c][i] < min) min = v[c][i];
    }

    result = INT2FIX(min);

    xfree(v[0]);
    xfree(v[1]);

    return result;
}

/*
 * DamerauLevenshtein edit distances are computed here:
 */

#define COMPUTE_DAMERAU_LEVENSHTEIN_DISTANCE                                \
    c = 0;                                                                  \
    p = 0;                                                                  \
    pp = 0;                                                                 \
    for (i = 1; i <= a_len; i++) {                                          \
        c = i % 3;                      /* current row */                   \
        p = (i - 1) % 3;                /* previous row */                  \
        pp = (i - 2) % 3;               /* previous previous row */         \
        v[c][0] = i;                    /* first column */                  \
        for (j = 1; j <= b_len; j++) {                                      \
            /* Bellman's principle of optimality: */                        \
            weight = v[p][j - 1] + (a_ptr[i - 1] == b_ptr[j - 1] ? 0 : 1);  \
            if (weight > v[p][j] + 1) {                                     \
                 weight = v[p][j] + 1;                                      \
            }                                                               \
            if (weight > v[c][j - 1] + 1) {                                 \
                weight = v[c][j - 1] + 1;                                   \
            }                                                               \
            if (i > 2 && j > 2 && a_ptr[i - 1] == b_ptr[j - 2] && a_ptr[i - 2] == b_ptr[j - 1]) {\
                if (weight > v[pp][j - 2]) {                                \
                    weight = v[pp][j - 2] + (a_ptr[i - 1] == b_ptr[j - 1] ? 0 : 1);       \
                }                                                           \
            }                                                               \
            v[c][j] = weight;                                               \
        }                                                                   \
    }

static VALUE DamerauLevenshtein_match(General *amatch, VALUE string)
{
    VALUE result;
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int *v[3], weight;
    int  i, j, c, p, pp;

    Check_Type(string, T_STRING);
    DONT_OPTIMIZE

    v[0] = ALLOC_N(int, b_len + 1);
    v[1] = ALLOC_N(int, b_len + 1);
    v[2] = ALLOC_N(int, b_len + 1);
    for (i = 0; i <= b_len; i++) {
        v[0][i] = i;
        v[1][i] = i;
        v[2][i] = i;
    }

    COMPUTE_DAMERAU_LEVENSHTEIN_DISTANCE

    result = INT2FIX(v[c][b_len]);

    xfree(v[0]);
    xfree(v[1]);
    xfree(v[2]);

    return result;
}

static VALUE DamerauLevenshtein_similar(General *amatch, VALUE string)
{
    VALUE result;
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int *v[3], weight;
    int  i, j, c, p, pp;

    Check_Type(string, T_STRING);
    DONT_OPTIMIZE

    if (a_len == 0 && b_len == 0) return rb_float_new(1.0);
    if (a_len == 0 || b_len == 0) return rb_float_new(0.0);
    v[0] = ALLOC_N(int, b_len + 1);
    v[1] = ALLOC_N(int, b_len + 1);
    v[2] = ALLOC_N(int, b_len + 1);
    for (i = 0; i <= b_len; i++) {
        v[0][i] = i;
        v[1][i] = i;
        v[2][i] = i;
    }

    COMPUTE_DAMERAU_LEVENSHTEIN_DISTANCE

    if (b_len > a_len) {
        result = rb_float_new(1.0 - ((double) v[c][b_len]) / b_len);
    } else {
        result = rb_float_new(1.0 - ((double) v[c][b_len]) / a_len);
    }

    xfree(v[0]);
    xfree(v[1]);
    xfree(v[2]);

    return result;
}

static VALUE DamerauLevenshtein_search(General *amatch, VALUE string)
{
    VALUE result;
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int *v[3], weight, min;
    int  i, j, c, p, pp;

    Check_Type(string, T_STRING);
    DONT_OPTIMIZE

    v[0] = ALLOC_N(int, b_len + 1);
    v[1] = ALLOC_N(int, b_len + 1);
    v[2] = ALLOC_N(int, b_len + 1);
    MEMZERO(v[0], int, b_len + 1);
    MEMZERO(v[1], int, b_len + 1);
    MEMZERO(v[2], int, b_len + 1);

    COMPUTE_DAMERAU_LEVENSHTEIN_DISTANCE

    for (i = 0, min = a_len; i <= b_len; i++) {
        if (v[c][i] < min) min = v[c][i];
    }

    result = INT2FIX(min);

    xfree(v[0]);
    xfree(v[1]);
    xfree(v[2]);

    return result;
}

/*
 * Sellers edit distances are computed here:
 */

#define COMPUTE_SELLERS_DISTANCE                                            \
    c = 0;                                                                  \
    p = 0;                                                                  \
    for (i = 1; i <= a_len; i++) {                                          \
        c = i % 2;                      /* current row */                   \
        p = (i - 1) % 2;                /* previous row */                  \
        v[c][0] = i * amatch->deletion; /* first column */                  \
        for (j = 1; j <= b_len; j++) {                                      \
            /* Bellman's principle of optimality: */                        \
            weight = v[p][j - 1] +                                          \
                (a_ptr[i - 1] == b_ptr[j - 1] ? 0 : amatch->substitution);  \
            if (weight > v[p][j] + amatch->insertion) {                     \
                 weight = v[p][j] + amatch->insertion;                      \
            }                                                               \
            if (weight > v[c][j - 1] + amatch->deletion) {                  \
                weight = v[c][j - 1] + amatch->deletion;                    \
            }                                                               \
            v[c][j] = weight;                                               \
        }                                                                   \
        p = c;                                                              \
    }

static VALUE Sellers_match(Sellers *amatch, VALUE string)
{
    VALUE result;
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    double *v[2], weight;
    int  i, j, c, p;

    Check_Type(string, T_STRING);
    DONT_OPTIMIZE

    v[0] = ALLOC_N(double, b_len + 1);
    v[1] = ALLOC_N(double, b_len + 1);
    for (i = 0; i <= b_len; i++) {
        v[0][i] = i * amatch->deletion;
        v[1][i] = i * amatch->deletion;
    }

    COMPUTE_SELLERS_DISTANCE

    result = rb_float_new(v[p][b_len]);
    xfree(v[0]);
    xfree(v[1]);
    return result;
}

static VALUE Sellers_similar(Sellers *amatch, VALUE string)
{
    VALUE result;
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    double *v[2], weight, max_weight;
    int  i, j, c, p;

    if (amatch->insertion >= amatch->deletion) {
        if (amatch->substitution >= amatch->insertion) {
            max_weight = amatch->substitution;
        } else {
            max_weight = amatch->insertion;
        }
    } else {
        if (amatch->substitution >= amatch->deletion) {
            max_weight = amatch->substitution;
        } else {
            max_weight = amatch->deletion;
        }
    }

    Check_Type(string, T_STRING);
    DONT_OPTIMIZE

    if (a_len == 0 && b_len == 0) return rb_float_new(1.0);
    if (a_len == 0 || b_len == 0) return rb_float_new(0.0);
    v[0] = ALLOC_N(double, b_len + 1);
    v[1] = ALLOC_N(double, b_len + 1);
    for (i = 0; i <= b_len; i++) {
        v[0][i] = i * amatch->deletion;
        v[1][i] = i * amatch->deletion;
    }

    COMPUTE_SELLERS_DISTANCE

    if (b_len > a_len) {
        result = rb_float_new(1.0 - v[p][b_len] / (b_len * max_weight));
    } else {
        result = rb_float_new(1.0 - v[p][b_len] / (a_len * max_weight));
    }
    xfree(v[0]);
    xfree(v[1]);
    return result;
}

static VALUE Sellers_search(Sellers *amatch, VALUE string)
{
    VALUE result;
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    double *v[2], weight, min;
    int  i, j, c, p;

    Check_Type(string, T_STRING);
    DONT_OPTIMIZE

    v[0] = ALLOC_N(double, b_len + 1);
    v[1] = ALLOC_N(double, b_len + 1);
    MEMZERO(v[0], double, b_len + 1);
    MEMZERO(v[1], double, b_len + 1);

    COMPUTE_SELLERS_DISTANCE

    for (i = 0, min = a_len; i <= b_len; i++) {
        if (v[p][i] < min) min = v[p][i];
    }
    result = rb_float_new(min);
    xfree(v[0]);
    xfree(v[1]);

    return result;
}

/*
 * Pair distances are computed here:
 */

static VALUE PairDistance_match(PairDistance *amatch, VALUE string, VALUE regexp, int use_regexp)
{
    double result;
    VALUE string_tokens, tokens;
    PairArray *pattern_pair_array, *pair_array;

    Check_Type(string, T_STRING);
    if (!NIL_P(regexp) || use_regexp) {
        tokens = rb_funcall(
            rb_str_new(amatch->pattern, amatch->pattern_len),
            id_split, 1, regexp
        );
        string_tokens = rb_funcall(string, id_split, 1, regexp);
    } else {
        VALUE tmp = rb_str_new(amatch->pattern, amatch->pattern_len);
        tokens = rb_ary_new4(1, &tmp);
        string_tokens = rb_ary_new4(1, &string);
    }

    if (!amatch->pattern_pair_array) {
        pattern_pair_array = PairArray_new(tokens);
        amatch->pattern_pair_array = pattern_pair_array;
    } else {
        pattern_pair_array = amatch->pattern_pair_array;
        pair_array_reactivate(amatch->pattern_pair_array);
    }
    pair_array = PairArray_new(string_tokens);

    result = pair_array_match(pattern_pair_array, pair_array);
    pair_array_destroy(pair_array);
    return rb_float_new(result);
}

/*
 * Hamming distances are computed here:
 */

#define COMPUTE_HAMMING_DISTANCE                            \
    for (i = 0, result = b_len - a_len; i < a_len; i++) {   \
        if (i >= b_len) {                                   \
            result +=  a_len - b_len;                       \
            break;                                          \
        }                                                   \
        if (b_ptr[i] != a_ptr[i]) result++;                 \
    }

static VALUE Hamming_match(General *amatch, VALUE string)
{
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int i, result;

    Check_Type(string, T_STRING);
    OPTIMIZE_TIME
    COMPUTE_HAMMING_DISTANCE
    return INT2FIX(result);
}

static VALUE Hamming_similar(General *amatch, VALUE string)
{
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int i, result;

    Check_Type(string, T_STRING);
    OPTIMIZE_TIME
    if (a_len == 0 && b_len == 0) return rb_float_new(1.0);
    if (a_len == 0 || b_len == 0) return rb_float_new(0.0);
    COMPUTE_HAMMING_DISTANCE
    return rb_float_new(1.0 - ((double) result) / b_len);
}

/*
 * Longest Common Subsequence computation
 */

#define COMPUTE_LONGEST_SUBSEQUENCE                         \
    l[0] = ALLOC_N(int, b_len + 1);                         \
    l[1] = ALLOC_N(int, b_len + 1);                         \
    for (i = a_len, c = 0, p = 1; i >= 0; i--) {            \
        for (j = b_len; j >= 0; j--) {                      \
            if (i == a_len || j == b_len) {                 \
                l[c][j] = 0;                                \
            } else if (a_ptr[i] == b_ptr[j]) {              \
                l[c][j] = 1 + l[p][j + 1];                  \
            } else {                                        \
                int x = l[p][j], y = l[c][j + 1];           \
                if (x > y) l[c][j] = x; else l[c][j] = y;   \
            }                                               \
        }                                                   \
        p = c;                                              \
        c = (c + 1) % 2;                                    \
    }                                                       \
    result = l[p][0];                                       \
    xfree(l[0]);                                             \
    xfree(l[1]);


static VALUE LongestSubsequence_match(General *amatch, VALUE string)
{
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int result, c, p, i, j, *l[2];

    Check_Type(string, T_STRING);
    OPTIMIZE_TIME

    if (a_len == 0 || b_len == 0) return INT2FIX(0);
    COMPUTE_LONGEST_SUBSEQUENCE
    return INT2FIX(result);
}

static VALUE LongestSubsequence_similar(General *amatch, VALUE string)
{
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int result, c, p, i, j, *l[2];

    Check_Type(string, T_STRING);
    OPTIMIZE_TIME

    if (a_len == 0 && b_len == 0) return rb_float_new(1.0);
    if (a_len == 0 || b_len == 0) return rb_float_new(0.0);
    COMPUTE_LONGEST_SUBSEQUENCE
    return rb_float_new(((double) result) / b_len);
}

/*
 * Longest Common Substring computation
 */

#define COMPUTE_LONGEST_SUBSTRING                           \
    l[0] = ALLOC_N(int, b_len);                             \
    MEMZERO(l[0], int, b_len);                              \
    l[1] = ALLOC_N(int, b_len);                             \
    MEMZERO(l[1], int, b_len);                              \
    result = 0;                                             \
    for (i = 0, c = 0, p = 1; i < a_len; i++) {             \
        for (j = 0; j < b_len; j++) {                       \
            if (a_ptr[i] == b_ptr[j]) {                     \
                l[c][j] = j == 0 ? 1 : 1 + l[p][j - 1];     \
                if (l[c][j] > result) result = l[c][j];     \
            } else {                                        \
                l[c][j] = 0;                                \
            }                                               \
        }                                                   \
        p = c;                                              \
        c = (c + 1) % 2;                                    \
    }                                                       \
    xfree(l[0]);                                             \
    xfree(l[1]);

static VALUE LongestSubstring_match(General *amatch, VALUE string)
{
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int result, c, p, i, j, *l[2];

    Check_Type(string, T_STRING);
    OPTIMIZE_TIME
    if (a_len == 0 || b_len == 0) return INT2FIX(0);
    COMPUTE_LONGEST_SUBSTRING
    return INT2FIX(result);
}

static VALUE LongestSubstring_similar(General *amatch, VALUE string)
{
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int result, c, p, i, j, *l[2];

    Check_Type(string, T_STRING);
    OPTIMIZE_TIME
    if (a_len == 0 && b_len == 0) return rb_float_new(1.0);
    if (a_len == 0 || b_len == 0) return rb_float_new(0.0);
    COMPUTE_LONGEST_SUBSTRING
    return rb_float_new(((double) result) / b_len);
}

/*
 * Jaro computation
 */

#define COMPUTE_JARO                                                                \
    l[0] = ALLOC_N(int, a_len);                                                     \
    MEMZERO(l[0], int, a_len);                                                      \
    l[1] = ALLOC_N(int, b_len);                                                     \
    MEMZERO(l[1], int, b_len);                                                      \
    max_dist = ((a_len > b_len ? a_len : b_len) / 2) - 1;                           \
    m = 0;                                                                          \
    for (i = 0; i < a_len; i++) {                                                   \
        low = (i > max_dist ? i - max_dist : 0);                                     \
        high = (i + max_dist < b_len ? i + max_dist : b_len - 1);                   \
        for (j = low; j <= high; j++) {                                             \
            if (!l[1][j] && a_ptr[i] == b_ptr[j]) {                                 \
                l[0][i] = 1;                                                        \
                l[1][j] = 1;                                                        \
                m++;                                                                \
                break;                                                              \
            }                                                                       \
        }                                                                           \
    }                                                                               \
    if (m == 0) {                                                                   \
        result = 0.0;                                                               \
    } else {                                                                        \
        k = t = 0;                                                                  \
        for (i = 0; i < a_len; i++) {                                               \
            if (l[0][i]) {                                                          \
                for (j = k; j < b_len; j++) {                                       \
                    if (l[1][j]) {                                                  \
                        k = j + 1;                                                  \
                        break;                                                      \
                    }                                                               \
                }                                                                   \
                if (a_ptr[i] != b_ptr[j]) {                                         \
                    t++;                                                            \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        t = t / 2;                                                                  \
        result = (((double)m)/a_len + ((double)m)/b_len + ((double)(m-t))/m)/3.0;   \
    }                                                                               \
    xfree(l[0]);                                                                     \
    xfree(l[1]);


#define LOWERCASE_STRINGS                                       \
     char *ying, *yang;                                         \
     ying = ALLOC_N(char, a_len);                               \
     MEMCPY(ying, a_ptr, char, a_len);                          \
     a_ptr = ying;                                              \
     yang = ALLOC_N(char, b_len);                               \
     MEMCPY(yang, b_ptr, char, b_len);                          \
     b_ptr = yang;                                              \
     for (i = 0; i < a_len; i++) {                              \
         if (islower(a_ptr[i])) a_ptr[i] = toupper(a_ptr[i]);   \
     }                                                          \
     for (i = 0; i < b_len; i++) {                              \
         if (islower(b_ptr[i])) b_ptr[i] = toupper(b_ptr[i]);   \
     }

static VALUE Jaro_match(Jaro *amatch, VALUE string)
{
    char *a_ptr, *b_ptr;
    int a_len, b_len, max_dist, m, t, i, j, k, low, high;
    int *l[2];
    double result;

    Check_Type(string, T_STRING);
    OPTIMIZE_TIME
    if (a_len == 0 && b_len == 0) return rb_float_new(1.0);
    if (a_len == 0 || b_len == 0) return rb_float_new(0.0);
    if (amatch->ignore_case) {
        LOWERCASE_STRINGS
    }
    COMPUTE_JARO
    if (amatch->ignore_case) {
        xfree(a_ptr);
        xfree(b_ptr);
    }
    return rb_float_new(result);
}

/*
 * Jaro-Winkler computation
 */

static VALUE JaroWinkler_match(JaroWinkler *amatch, VALUE string)
{
    char *a_ptr, *b_ptr;
    int a_len, b_len, max_dist, m, t, i, j, k, low, high, n;
    int *l[2];
    double result;

    Check_Type(string, T_STRING);
    OPTIMIZE_TIME
    if (a_len == 0 && b_len == 0) return rb_float_new(1.0);
    if (a_len == 0 || b_len == 0) return rb_float_new(0.0);
    if (amatch->ignore_case) {
        LOWERCASE_STRINGS
    }
    COMPUTE_JARO
    n = 0;
    for (i = 0; i < (a_len >= 4 ? 4 : a_len); i++) {
        if (a_ptr[i] == b_ptr[i]) {
            n++;
        } else {
            break;
        }
    }
    result = result + n*amatch->scaling_factor*(1-result);
    if (amatch->ignore_case) {
        xfree(a_ptr);
        xfree(b_ptr);
    }
    return rb_float_new(result);
}

/*
 * Ruby API
 */

 /*
  * Document-class: Amatch::Levenshtein
  *
  * The Levenshtein edit distance is defined as the minimal costs involved to
  * transform one string into another by using three elementary operations:
  * deletion, insertion and substitution of a character. To transform "water"
  * into "wine", for instance, you have to substitute "a" -> "i": "witer", "t"
  * -> "n": "winer" and delete "r": "wine". The edit distance between "water"
  * and "wine" is 3, because you have to apply three operations. The edit
  * distance between "wine" and "wine" is 0 of course: no operation is
  * necessary for the transformation -- they're already the same string. It's
  * easy to see that more similar strings have smaller edit distances than
  * strings that differ a lot.
  */

DEF_RB_FREE(Levenshtein, General)

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::Levenshtein instance from <code>pattern</code>.
 */
static VALUE rb_Levenshtein_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(General)
    General_pattern_set(amatch, pattern);
    return self;
}

DEF_CONSTRUCTOR(Levenshtein, General)

/*
 * call-seq: match(strings) -> results
 *
 * Uses this Amatch::Levenshtein instance to match Amatch::Levenshtein#pattern
 * against <code>strings</code>. It returns the number operations, the Sellers
 * distance. <code>strings</code> has to be either a String or an Array of
 * Strings. The returned <code>results</code> is either a Float or an Array of
 * Floats respectively.
 */
static VALUE rb_Levenshtein_match(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, Levenshtein_match);
}

/*
 * call-seq: similar(strings) -> results
 *
 * Uses this Amatch::Levenshtein instance to match  Amatch::Levenshtein#pattern
 * against <code>strings</code>, and compute a Levenshtein distance metric
 * number between 0.0 for very unsimilar strings and 1.0 for an exact match.
 * <code>strings</code> has to be either a String or an Array of Strings. The
 * returned <code>results</code> is either a Fixnum or an Array of Fixnums
 * respectively.
 */
static VALUE rb_Levenshtein_similar(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, Levenshtein_similar);
}

/*
 * call-seq: levenshtein_similar(strings) -> results
 *
 * If called on a String, this string is used as a Amatch::Levenshtein#pattern
 * to match against <code>strings</code>. It returns a Levenshtein distance
 * metric number between 0.0 for very unsimilar strings and 1.0 for an exact
 * match. <code>strings</code> has to be either a String or an Array of
 * Strings. The returned <code>results</code> is either a Float or an Array of
 * Floats respectively.
 */
static VALUE rb_str_levenshtein_similar(VALUE self, VALUE strings)
{
    VALUE amatch = rb_Levenshtein_new(rb_cLevenshtein, self);
    return rb_Levenshtein_similar(amatch, strings);
}

/*
 * call-seq: search(strings) -> results
 *
 * searches Amatch::Levenshtein#pattern in <code>strings</code> and returns the
 * edit distance (the sum of character operations) as a Fixnum value, by greedy
 * trimming prefixes or postfixes of the match. <code>strings</code> has
 * to be either a String or an Array of Strings. The returned
 * <code>results</code> is either a Float or an Array of Floats respectively.
 */
static VALUE rb_Levenshtein_search(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, Levenshtein_search);
}

/*
 * Document-class: Amatch::DamerauLevenshtein
 * XXX
 * The DamerauLevenshtein edit distance is defined as the minimal costs
 * involved to transform one string into another by using three elementary
 * operations: deletion, insertion and substitution of a character. To
 * transform "water" into "wine", for instance, you have to substitute "a" ->
 * "i": "witer", "t" -> "n": "winer" and delete "r": "wine". The edit distance
 * between "water" and "wine" is 3, because you have to apply three
 * operations. The edit distance between "wine" and "wine" is 0 of course: no
 * operation is necessary for the transformation -- they're already the same
 * string. It's easy to see that more similar strings have smaller edit
 * distances than strings that differ a lot.
 */

DEF_RB_FREE(DamerauLevenshtein, General)

/*
 * call-seq: new(pattern)
  * XXX
 * Creates a new Amatch::DamerauLevenshtein instance from <code>pattern</code>.
 */
static VALUE rb_DamerauLevenshtein_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(General)
    General_pattern_set(amatch, pattern);
    return self;
}

DEF_CONSTRUCTOR(DamerauLevenshtein, General)

/*
 * call-seq: match(strings) -> results
 * XXX
 * Uses this Amatch::DamerauLevenshtein instance to match Amatch::DamerauLevenshtein#pattern
 * against <code>strings</code>. It returns the number operations, the Sellers
 * distance. <code>strings</code> has to be either a String or an Array of
 * Strings. The returned <code>results</code> is either a Float or an Array of
 * Floats respectively.
 */
static VALUE rb_DamerauLevenshtein_match(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, DamerauLevenshtein_match);
}

/*
 * call-seq: similar(strings) -> results
 * XXX
 * Uses this Amatch::DamerauLevenshtein instance to match  Amatch::DamerauLevenshtein#pattern
 * against <code>strings</code>, and compute a DamerauLevenshtein distance metric
 * number between 0.0 for very unsimilar strings and 1.0 for an exact match.
 * <code>strings</code> has to be either a String or an Array of Strings. The
 * returned <code>results</code> is either a Fixnum or an Array of Fixnums
 * respectively.
 */
static VALUE rb_DamerauLevenshtein_similar(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, DamerauLevenshtein_similar);
}

/*
 * call-seq: levenshtein_similar(strings) -> results
 * XXX
 * If called on a String, this string is used as a Amatch::DamerauLevenshtein#pattern
 * to match against <code>strings</code>. It returns a DamerauLevenshtein distance
 * metric number between 0.0 for very unsimilar strings and 1.0 for an exact
 * match. <code>strings</code> has to be either a String or an Array of
 * Strings. The returned <code>results</code> is either a Float or an Array of
 * Floats respectively.
 */
static VALUE rb_str_damerau_levenshtein_similar(VALUE self, VALUE strings)
{
    VALUE amatch = rb_DamerauLevenshtein_new(rb_cDamerauLevenshtein, self);
    return rb_DamerauLevenshtein_similar(amatch, strings);
}

/*
 * call-seq: search(strings) -> results
 * XXX
 * searches Amatch::DamerauLevenshtein#pattern in <code>strings</code> and returns the
 * edit distance (the sum of character operations) as a Fixnum value, by greedy
 * trimming prefixes or postfixes of the match. <code>strings</code> has
 * to be either a String or an Array of Strings. The returned
 * <code>results</code> is either a Float or an Array of Floats respectively.
 */
static VALUE rb_DamerauLevenshtein_search(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, DamerauLevenshtein_search);
}

/*
 * Document-class: Amatch::Sellers
 *
 * The Sellers edit distance is very similar to the Levenshtein edit distance.
 * The difference is, that you can also specify different weights for every
 * operation to prefer special operations over others. This extension of the
 * Sellers edit distance is also known under the names: Needleman-Wunsch
 * distance.
 */

DEF_RB_FREE(Sellers, Sellers)

/*
 * Document-method: substitution
 *
 * call-seq: substitution -> weight
 *
 * Returns the weight of the substitution operation, that is used to compute
 * the Sellers distance.
 */
DEF_RB_READER(Sellers, rb_Sellers_substitution, substitution,
    rb_float_new)

/*
 * Document-method: deletion
 *
 * call-seq: deletion -> weight
 *
 * Returns the weight of the deletion operation, that is used to compute
 * the Sellers distance.
 */
DEF_RB_READER(Sellers, rb_Sellers_deletion, deletion,
    rb_float_new)

/*
 * Document-method: insertion
 *
 * call-seq: insertion -> weight
 *
 * Returns the weight of the insertion operation, that is used to compute
 * the Sellers distance.
 */
DEF_RB_READER(Sellers, rb_Sellers_insertion, insertion,
    rb_float_new)

/*
 * Document-method: substitution=
 *
 * call-seq: substitution=(weight)
 *
 * Sets the weight of the substitution operation, that is used to compute
 * the Sellers distance, to <code>weight</code>. The <code>weight</code>
 * should be a Float value >= 0.0.
 */
DEF_RB_WRITER(Sellers, rb_Sellers_substitution_set, substitution,
    double, CAST2FLOAT, FLOAT2C, >= 0)

/*
 * Document-method: deletion=
 *
 * call-seq: deletion=(weight)
 *
 * Sets the weight of the deletion operation, that is used to compute
 * the Sellers distance, to <code>weight</code>. The <code>weight</code>
 * should be a Float value >= 0.0.
 */
DEF_RB_WRITER(Sellers, rb_Sellers_deletion_set, deletion,
    double, CAST2FLOAT, FLOAT2C, >= 0)

/*
 * Document-method: insertion=
 *
 * call-seq: insertion=(weight)
 *
 * Sets the weight of the insertion operation, that is used to compute
 * the Sellers distance, to <code>weight</code>. The <code>weight</code>
 * should be a Float value >= 0.0.
 */
DEF_RB_WRITER(Sellers, rb_Sellers_insertion_set, insertion,
    double, CAST2FLOAT, FLOAT2C, >= 0)

/*
 * Resets all weights (substitution, deletion, and insertion) to 1.0.
 */
static VALUE rb_Sellers_reset_weights(VALUE self)
{
    GET_STRUCT(Sellers)
    Sellers_reset_weights(amatch);
    return self;
}

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::Sellers instance from <code>pattern</code>,
 * with all weights initially set to 1.0.
 */
static VALUE rb_Sellers_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(Sellers)
    Sellers_pattern_set(amatch, pattern);
    Sellers_reset_weights(amatch);
    return self;
}

DEF_CONSTRUCTOR(Sellers, Sellers)

/*
 * Document-method: pattern
 *
 * call-seq: pattern -> pattern string
 *
 * Returns the current pattern string of this Amatch::Sellers instance.
 */

/*
 * Document-method: pattern=
 *
 * call-seq: pattern=(pattern)
 *
 * Sets the current pattern string of this Amatch::Sellers instance to
 * <code>pattern</code>.
 */

/*
 * call-seq: match(strings) -> results
 *
 * Uses this Amatch::Sellers instance to match Sellers#pattern against
 * <code>strings</code>, while taking into account the given weights. It
 * returns the number of weighted character operations, the Sellers distance.
 * <code>strings</code> has to be either a String or an Array of Strings. The
 * returned <code>results</code> is either a Float or an Array of Floats
 * respectively.
 */
static VALUE rb_Sellers_match(VALUE self, VALUE strings)
{
    GET_STRUCT(Sellers)
    return Sellers_iterate_strings(amatch, strings, Sellers_match);
}

/*
 * call-seq: similar(strings) -> results
 *
 * Uses this Amatch::Sellers instance to match Amatch::Sellers#pattern
 * against <code>strings</code> (taking into account the given weights), and
 * compute a Sellers distance metric number between 0.0 for very unsimilar
 * strings and 1.0 for an exact match. <code>strings</code> has to be either a
 * String or an Array of Strings. The returned <code>results</code> is either
 * a Fixnum or an Array of Fixnums
 * respectively.
 */
static VALUE rb_Sellers_similar(VALUE self, VALUE strings)
{
    GET_STRUCT(Sellers)
    return Sellers_iterate_strings(amatch, strings, Sellers_similar);
}

/*
 * call-seq: search(strings) -> results
 *
 * searches Sellers#pattern in <code>strings</code> and returns the edit
 * distance (the sum of weighted character operations) as a Float value, by
 * greedy trimming prefixes or postfixes of the match. <code>strings</code> has
 * to be either a String or an Array of Strings. The returned
 * <code>results</code> is either a Float or an Array of Floats respectively.
 */
static VALUE rb_Sellers_search(VALUE self, VALUE strings)
{
    GET_STRUCT(Sellers)
    return Sellers_iterate_strings(amatch, strings, Sellers_search);
}

/*
 * Document-class: Amatch::PairDistance
 *
 * The pair distance between two strings is based on the number of adjacent
 * character pairs, that are contained in both strings. The similiarity
 * metric of two strings s1 and s2 is
 *   2*|union(pairs(s1), pairs(s2))| / |pairs(s1)| + |pairs(s2)|
 * If it is 1.0 the two strings are an exact match, if less than 1.0 they
 * are more dissimilar. The advantage of considering adjacent characters, is to
 * take account not only of the characters, but also of the character ordering
 * in the original strings.
 *
 * This metric is very capable to find similarities in natural languages.
 * It is explained in more detail in Simon White's article "How to Strike a
 * Match", located at this url:
 * http://www.catalysoft.com/articles/StrikeAMatch.html
 * It is also very similar (a special case) to the method described under
 * http://citeseer.lcs.mit.edu/gravano01using.html in "Using q-grams in a DBMS
 * for Approximate String Processing."
 */
DEF_RB_FREE(PairDistance, PairDistance)

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::PairDistance instance from <code>pattern</code>.
 */
static VALUE rb_PairDistance_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(PairDistance)
    PairDistance_pattern_set(amatch, pattern);
    return self;
}

DEF_CONSTRUCTOR(PairDistance, PairDistance)

/*
 * call-seq: match(strings, regexp = /\s+/) -> results
 *
 * Uses this Amatch::PairDistance instance to match  PairDistance#pattern against
 * <code>strings</code>. It returns the pair distance measure, that is a
 * returned value of 1.0 is an exact match, partial matches are lower
 * values, while 0.0 means no match at all.
 *
 * <code>strings</code> has to be either a String or an
 * Array of Strings. The argument <code>regexp</code> is used to split the
 * pattern and strings into tokens first. It defaults to /\s+/. If the
 * splitting should be omitted, call the method with nil as <code>regexp</code>
 * explicitly.
 *
 * The returned <code>results</code> is either a Float or an
 * Array of Floats respectively.
 */
static VALUE rb_PairDistance_match(int argc, VALUE *argv, VALUE self)
{
    VALUE result, strings, regexp = Qnil;
    int use_regexp;
    GET_STRUCT(PairDistance)

    rb_scan_args(argc, argv, "11", &strings, &regexp);
    use_regexp = NIL_P(regexp) && argc != 2;
    if (TYPE(strings) == T_STRING) {
        result = PairDistance_match(amatch, strings, regexp, use_regexp);
    } else {
        int i;
        Check_Type(strings, T_ARRAY);
        result = rb_ary_new2(RARRAY_LEN(strings));
        for (i = 0; i < RARRAY_LEN(strings); i++) {
            VALUE string = rb_ary_entry(strings, i);
            if (TYPE(string) != T_STRING) {
                rb_raise(rb_eTypeError,
                    "array has to contain only strings (%s given)",
                    NIL_P(string) ?
                        "NilClass" :
                        rb_class2name(CLASS_OF(string)));
            }
            rb_ary_push(result,
                PairDistance_match(amatch, string, regexp, use_regexp));
        }
    }
    pair_array_destroy(amatch->pattern_pair_array);
    amatch->pattern_pair_array = NULL;
    return result;
}

/*
 * call-seq: pair_distance_similar(strings, regexp = nil) -> results
 *
 * If called on a String, this string is used as a Amatch::PairDistance#pattern
 * to match against <code>strings</code> using /\s+/ as the tokenizing regular
 * expression. It returns a pair distance metric number between 0.0 for very
 * unsimilar strings and 1.0 for an exact match. <code>strings</code> has to be
 * either a String or an Array of Strings.
 *
 * The returned <code>results</code> is either a Float or an Array of Floats
 * respectively.
 */
static VALUE rb_str_pair_distance_similar(int argc, VALUE *argv, VALUE self)
{
    VALUE amatch, string, regexp = Qnil;
    rb_scan_args(argc, argv, "11",  &string, &regexp);
    amatch = rb_PairDistance_new(rb_cPairDistance, self);
    if (NIL_P(regexp)) {
        return rb_PairDistance_match(1, &string, amatch);
    } else {
        VALUE *args = alloca(2);
        args[0] = string;
        args[1] = regexp;
        return rb_PairDistance_match(2, args, amatch);
    }
}

/*
 * Document-class: Amatch::Hamming
 *
 *  This class computes the Hamming distance between two strings.
 *
 *  The Hamming distance between two strings is the number of characters, that
 *  are different. Thus a hamming distance of 0 means an exact
 *  match, a hamming distance of 1 means one character is different, and so on.
 *  If one string is longer than the other string, the missing characters are
 *  counted as different characters.
 */

DEF_RB_FREE(Hamming, General)

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::Hamming instance from <code>pattern</code>.
 */
static VALUE rb_Hamming_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(General)
    General_pattern_set(amatch, pattern);
    return self;
}

DEF_CONSTRUCTOR(Hamming, General)

/*
 * call-seq: match(strings) -> results
 *
 * Uses this Amatch::Hamming instance to match Amatch::Hamming#pattern against
 * <code>strings</code>, that is compute the hamming distance between
 * <code>pattern</code> and <code>strings</code>. <code>strings</code> has to
 * be either a String or an Array of Strings. The returned <code>results</code>
 * is either a Fixnum or an Array of Fixnums respectively.
 */
static VALUE rb_Hamming_match(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, Hamming_match);
}

/*
 * call-seq: similar(strings) -> results
 *
 * Uses this Amatch::Hamming instance to match  Amatch::Hamming#pattern against
 * <code>strings</code>, and compute a Hamming distance metric number between
 * 0.0 for very unsimilar strings and 1.0 for an exact match.
 * <code>strings</code> has to be either a String or an Array of Strings. The
 * returned <code>results</code> is either a Fixnum or an Array of Fixnums
 * respectively.
 */
static VALUE rb_Hamming_similar(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, Hamming_similar);
}

/*
 * call-seq: hamming_similar(strings) -> results
 *
 * If called on a String, this string is used as a Amatch::Hamming#pattern to
 * match against <code>strings</code>. It returns a Hamming distance metric
 * number between 0.0 for very unsimilar strings and 1.0 for an exact match.
 * <code>strings</code>
 * has to be either a String or an Array of Strings. The returned
 * <code>results</code> is either a Float or an Array of Floats respectively.
 */
static VALUE rb_str_hamming_similar(VALUE self, VALUE strings)
{
    VALUE amatch = rb_Hamming_new(rb_cHamming, self);
    return rb_Hamming_similar(amatch, strings);
}


/*
 * Document-class: Amatch::LongestSubsequence
 *
 *  This class computes the length of the longest subsequence common to two
 *  strings. A subsequence doesn't have to be contiguous. The longer the common
 *  subsequence is, the more similar the two strings will be.
 *
 *  The longest common subsequence between "test" and "test" is of length 4,
 *  because "test" itself is this subsequence. The longest common subsequence
 *  between "test" and "east" is "e", "s", "t" and the length of the
 *  sequence is 3.
 */
DEF_RB_FREE(LongestSubsequence, General)

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::LongestSubsequence instance from <code>pattern</code>.
 */
static VALUE rb_LongestSubsequence_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(General)
    General_pattern_set(amatch, pattern);
    return self;
}

DEF_CONSTRUCTOR(LongestSubsequence, General)

/*
 * call-seq: match(strings) -> results
 *
 * Uses this Amatch::LongestSubsequence instance to match
 * LongestSubsequence#pattern against <code>strings</code>, that is compute the
 * length of the longest common subsequence. <code>strings</code> has to be
 * either a String or an Array of Strings. The returned <code>results</code>
 * is either a Fixnum or an Array of Fixnums respectively.
 */
static VALUE rb_LongestSubsequence_match(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, LongestSubsequence_match);
}

/*
 * call-seq: similar(strings) -> results
 *
 * Uses this Amatch::LongestSubsequence instance to match
 * Amatch::LongestSubsequence#pattern against <code>strings</code>, and compute
 * a longest substring distance metric number between 0.0 for very unsimilar
 * strings and 1.0 for an exact match. <code>strings</code> has to be either a
 * String or an Array of Strings. The returned <code>results</code> is either
 * a Fixnum or an Array of Fixnums
 */
static VALUE rb_LongestSubsequence_similar(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, LongestSubsequence_similar);
}

/*
 * call-seq: longest_subsequence_similar(strings) -> results
 *
 * If called on a String, this string is used as a
 * Amatch::LongestSubsequence#pattern to match against <code>strings</code>. It
 * returns a longest subsequence distance metric number between 0.0 for very
 * unsimilar strings and 1.0 for an exact match. <code>strings</code> has to be
 * either a String or an Array of Strings. The returned <code>results</code>
 * is either a Float or an Array of Floats respectively.
 */
static VALUE rb_str_longest_subsequence_similar(VALUE self, VALUE strings)
{
    VALUE amatch = rb_LongestSubsequence_new(rb_cLongestSubsequence, self);
    return rb_LongestSubsequence_similar(amatch, strings);
}

/*
 * Document-class: Amatch::LongestSubstring
 *
 * The longest common substring is the longest substring, that is part of
 * two strings. A substring is contiguous, while a subsequence need not to
 * be. The longer the common substring is, the more similar the two strings
 * will be.
 *
 * The longest common substring between 'string' and 'string' is 'string'
 * again, thus the longest common substring length is 6. The longest common
 * substring between 'string' and 'storing' is 'ring', thus the longest common
 * substring length is 4.
 */

DEF_RB_FREE(LongestSubstring, General)

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::LongestSubstring instance from <code>pattern</code>.
 */
static VALUE rb_LongestSubstring_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(General)
    General_pattern_set(amatch, pattern);
    return self;
}

DEF_CONSTRUCTOR(LongestSubstring, General)

/*
 * call-seq: match(strings) -> results
 *
 * Uses this Amatch::LongestSubstring instance to match
 * LongestSubstring#pattern against <code>strings</code>, that is compute the
 * length of the longest common substring. <code>strings</code> has to be
 * either a String or an Array of Strings. The returned <code>results</code>
 * is either a Fixnum or an Array of Fixnums respectively.
 */
static VALUE rb_LongestSubstring_match(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, LongestSubstring_match);
}

/*
 * call-seq: similar(strings) -> results
 *
 * Uses this Amatch::LongestSubstring instance to match
 * Amatch::LongestSubstring#pattern against <code>strings</code>, and compute a
 * longest substring distance metric number between 0.0 for very unsimilar
 * strings and 1.0 for an exact match. <code>strings</code> has to be either a
 * String or an Array of Strings. The returned <code>results</code> is either
 * a Fixnum or an Array of Fixnums
 * respectively.
 */
static VALUE rb_LongestSubstring_similar(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, LongestSubstring_similar);
}

/*
 * call-seq: longest_substring_similar(strings) -> results
 *
 * If called on a String, this string is used as a
 * Amatch::LongestSubstring#pattern to match against <code>strings</code>. It
 * returns a longest substring distance metric number between 0.0 for very
 * unsimilar strings and 1.0 for an exact match. <code>strings</code> has to be
 * either a String or an Array of Strings. The returned <code>results</code>
 * is either a Float or an Array of Floats respectively.
 */
static VALUE rb_str_longest_substring_similar(VALUE self, VALUE strings)
{
    VALUE amatch = rb_LongestSubstring_new(rb_cLongestSubstring, self);
    return rb_LongestSubstring_similar(amatch, strings);
}

/*
 * Document-class: Amatch::Jaro
 *
 * This class computes the Jaro metric for two strings.
 * The Jaro metric computes the similarity between 0 (no match)
 * and 1 (exact match) by looking for matching and transposed characters.
 */
DEF_RB_FREE(Jaro, Jaro)

/*
 * Document-method: ignore_case
 *
 * call-seq: ignore_case -> true/false
 *
 * Returns whether case is ignored when computing matching characters.
 */
DEF_RB_READER(Jaro, rb_Jaro_ignore_case, ignore_case, C2BOOL)

/*
 * Document-method: ignore_case=
 *
 * call-seq: ignore_case=(true/false)
 *
 * Sets whether case is ignored when computing matching characters.
 */
DEF_RB_WRITER(Jaro, rb_Jaro_ignore_case_set, ignore_case,
    int, CAST2BOOL, BOOL2C, != Qundef)

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::Jaro instance from <code>pattern</code>.
 */
static VALUE rb_Jaro_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(Jaro)
    Jaro_pattern_set(amatch, pattern);
    amatch->ignore_case = 1;
    return self;
}

DEF_CONSTRUCTOR(Jaro, Jaro)

/*
 * call-seq: match(strings) -> results
 *
 * Uses this Amatch::Jaro instance to match
 * Jaro#pattern against <code>strings</code>, that is compute the
 * jaro metric with the strings. <code>strings</code> has to be
 * either a String or an Array of Strings. The returned <code>results</code>
 * is either a Float or an Array of Floats respectively.
 */
static VALUE rb_Jaro_match(VALUE self, VALUE strings)
{
    GET_STRUCT(Jaro)
    return Jaro_iterate_strings(amatch, strings, Jaro_match);
}

/*
 * call-seq: jaro_similar(strings) -> results
 *
 * If called on a String, this string is used as a
 * Amatch::Jaro#pattern to match against <code>strings</code>. It
 * returns a Jaro metric number between 0.0 for very
 * unsimilar strings and 1.0 for an exact match. <code>strings</code> has to be
 * either a String or an Array of Strings. The returned <code>results</code>
 * is either a Float or an Array of Floats respectively.
 */
static VALUE rb_str_jaro_similar(VALUE self, VALUE strings)
{
    VALUE amatch = rb_Jaro_new(rb_cJaro, self);
    return rb_Jaro_match(amatch, strings);
}

/*
 * Document-class: Amatch::JaroWinkler
 *
 * This class computes the Jaro-Winkler metric for two strings.
 * The Jaro-Winkler metric computes the similarity between 0 (no match)
 * and 1 (exact match) by looking for matching and transposed characters.
 *
 * It is a variant of the Jaro metric, with additional weighting towards
 * common prefixes.
 */
DEF_RB_FREE(JaroWinkler, JaroWinkler)

/*
 * Document-method: ignore_case
 *
 * call-seq: ignore_case -> true/false
 *
 * Returns whether case is ignored when computing matching characters.
 * Default is true.
 */
DEF_RB_READER(JaroWinkler, rb_JaroWinkler_ignore_case, ignore_case, C2BOOL)

/*
 * Document-method: scaling_factor
 *
 * call-seq: scaling_factor -> weight
 *
 * The scaling factor is how much weight to give common prefixes.
 * Default is 0.1.
 */
DEF_RB_READER(JaroWinkler, rb_JaroWinkler_scaling_factor, scaling_factor, rb_float_new)

/*
 * Document-method: ignore_case=
 *
 * call-seq: ignore_case=(true/false)
 *
 * Sets whether case is ignored when computing matching characters.
 */
DEF_RB_WRITER(JaroWinkler, rb_JaroWinkler_ignore_case_set, ignore_case,
    int, CAST2BOOL, BOOL2C, != Qundef)

/*
 * Document-method: scaling_factor=
 *
 * call-seq: scaling_factor=(weight)
 *
 * Sets the weight to give common prefixes.
 */
DEF_RB_WRITER(JaroWinkler, rb_JaroWinkler_scaling_factor_set, scaling_factor,
    double, CAST2FLOAT, FLOAT2C, >= 0)

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::JaroWinkler instance from <code>pattern</code>.
 */
static VALUE rb_JaroWinkler_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(JaroWinkler)
    JaroWinkler_pattern_set(amatch, pattern);
    amatch->ignore_case = 1;
    amatch->scaling_factor = 0.1;
    return self;
}

DEF_CONSTRUCTOR(JaroWinkler, JaroWinkler)

/*
 * call-seq: match(strings) -> results
 *
 * Uses this Amatch::Jaro instance to match
 * Jaro#pattern against <code>strings</code>, that is compute the
 * jaro metric with the strings. <code>strings</code> has to be
 * either a String or an Array of Strings. The returned <code>results</code>
 * is either a Float or an Array of Floats respectively.
 */
static VALUE rb_JaroWinkler_match(VALUE self, VALUE strings)
{
    GET_STRUCT(JaroWinkler)
    return JaroWinkler_iterate_strings(amatch, strings, JaroWinkler_match);
}

/*
 * call-seq: jarowinkler_similar(strings) -> results
 *
 * If called on a String, this string is used as a
 * Amatch::JaroWinkler#pattern to match against <code>strings</code>. It
 * returns a Jaro-Winkler metric number between 0.0 for very
 * unsimilar strings and 1.0 for an exact match. <code>strings</code> has to be
 * either a String or an Array of Strings. The returned <code>results</code>
 * are either a Float or an Array of Floats respectively.
 */
static VALUE rb_str_jarowinkler_similar(VALUE self, VALUE strings)
{
    VALUE amatch = rb_JaroWinkler_new(rb_cJaro, self);
    return rb_JaroWinkler_match(amatch, strings);
}

/*
 * This is the namespace module that includes all other classes, modules, and
 * constants.
 */

void Init_amatch_ext()
{
    rb_require("amatch/version");
    rb_mAmatch = rb_define_module("Amatch");
    /* This module can be mixed into ::String or its subclasses to mixin the similary methods directly. */
    rb_mAmatchStringMethods = rb_define_module_under(rb_mAmatch, "StringMethods");

    /* Levenshtein */
    rb_cLevenshtein = rb_define_class_under(rb_mAmatch, "Levenshtein", rb_cObject);
    rb_define_alloc_func(rb_cLevenshtein, rb_Levenshtein_s_allocate);
    rb_define_method(rb_cLevenshtein, "initialize", rb_Levenshtein_initialize, 1);
    rb_define_method(rb_cLevenshtein, "pattern", rb_General_pattern, 0);
    rb_define_method(rb_cLevenshtein, "pattern=", rb_General_pattern_set, 1);
    rb_define_method(rb_cLevenshtein, "match", rb_Levenshtein_match, 1);
    rb_define_method(rb_cLevenshtein, "search", rb_Levenshtein_search, 1);
    rb_define_method(rb_cLevenshtein, "similar", rb_Levenshtein_similar, 1);
    rb_define_method(rb_mAmatchStringMethods, "levenshtein_similar", rb_str_levenshtein_similar, 1);

    /* DamerauLevenshtein */
    rb_cDamerauLevenshtein = rb_define_class_under(rb_mAmatch, "DamerauLevenshtein", rb_cObject);
    rb_define_alloc_func(rb_cDamerauLevenshtein, rb_DamerauLevenshtein_s_allocate);
    rb_define_method(rb_cDamerauLevenshtein, "initialize", rb_DamerauLevenshtein_initialize, 1);
    rb_define_method(rb_cDamerauLevenshtein, "pattern", rb_General_pattern, 0);
    rb_define_method(rb_cDamerauLevenshtein, "pattern=", rb_General_pattern_set, 1);
    rb_define_method(rb_cDamerauLevenshtein, "match", rb_DamerauLevenshtein_match, 1);
    rb_define_method(rb_cDamerauLevenshtein, "search", rb_DamerauLevenshtein_search, 1);
    rb_define_method(rb_cDamerauLevenshtein, "similar", rb_DamerauLevenshtein_similar, 1);
    rb_define_method(rb_mAmatchStringMethods, "damerau_levenshtein_similar", rb_str_damerau_levenshtein_similar, 1);

    /* Sellers */
    rb_cSellers = rb_define_class_under(rb_mAmatch, "Sellers", rb_cObject);
    rb_define_alloc_func(rb_cSellers, rb_Sellers_s_allocate);
    rb_define_method(rb_cSellers, "initialize", rb_Sellers_initialize, 1);
    rb_define_method(rb_cSellers, "pattern", rb_Sellers_pattern, 0);
    rb_define_method(rb_cSellers, "pattern=", rb_Sellers_pattern_set, 1);
    rb_define_method(rb_cSellers, "substitution", rb_Sellers_substitution, 0);
    rb_define_method(rb_cSellers, "substitution=", rb_Sellers_substitution_set, 1);
    rb_define_method(rb_cSellers, "deletion", rb_Sellers_deletion, 0);
    rb_define_method(rb_cSellers, "deletion=", rb_Sellers_deletion_set, 1);
    rb_define_method(rb_cSellers, "insertion", rb_Sellers_insertion, 0);
    rb_define_method(rb_cSellers, "insertion=", rb_Sellers_insertion_set, 1);
    rb_define_method(rb_cSellers, "reset_weights", rb_Sellers_reset_weights, 0);
    rb_define_method(rb_cSellers, "match", rb_Sellers_match, 1);
    rb_define_method(rb_cSellers, "search", rb_Sellers_search, 1);
    rb_define_method(rb_cSellers, "similar", rb_Sellers_similar, 1);

    /* Hamming */
    rb_cHamming = rb_define_class_under(rb_mAmatch, "Hamming", rb_cObject);
    rb_define_alloc_func(rb_cHamming, rb_Hamming_s_allocate);
    rb_define_method(rb_cHamming, "initialize", rb_Hamming_initialize, 1);
    rb_define_method(rb_cHamming, "pattern", rb_General_pattern, 0);
    rb_define_method(rb_cHamming, "pattern=", rb_General_pattern_set, 1);
    rb_define_method(rb_cHamming, "match", rb_Hamming_match, 1);
    rb_define_method(rb_cHamming, "similar", rb_Hamming_similar, 1);
    rb_define_method(rb_mAmatchStringMethods, "hamming_similar", rb_str_hamming_similar, 1);

    /* Pair Distance Metric / Dice Coefficient */
    rb_cPairDistance = rb_define_class_under(rb_mAmatch, "PairDistance", rb_cObject);
    rb_define_alloc_func(rb_cPairDistance, rb_PairDistance_s_allocate);
    rb_define_method(rb_cPairDistance, "initialize", rb_PairDistance_initialize, 1);
    rb_define_method(rb_cPairDistance, "pattern", rb_PairDistance_pattern, 0);
    rb_define_method(rb_cPairDistance, "pattern=", rb_PairDistance_pattern_set, 1);
    rb_define_method(rb_cPairDistance, "match", rb_PairDistance_match, -1);
    rb_define_alias(rb_cPairDistance, "similar", "match");
    rb_define_method(rb_mAmatchStringMethods, "pair_distance_similar", rb_str_pair_distance_similar, -1);

    /* Longest Common Subsequence */
    rb_cLongestSubsequence = rb_define_class_under(rb_mAmatch, "LongestSubsequence", rb_cObject);
    rb_define_alloc_func(rb_cLongestSubsequence, rb_LongestSubsequence_s_allocate);
    rb_define_method(rb_cLongestSubsequence, "initialize", rb_LongestSubsequence_initialize, 1);
    rb_define_method(rb_cLongestSubsequence, "pattern", rb_General_pattern, 0);
    rb_define_method(rb_cLongestSubsequence, "pattern=", rb_General_pattern_set, 1);
    rb_define_method(rb_cLongestSubsequence, "match", rb_LongestSubsequence_match, 1);
    rb_define_method(rb_cLongestSubsequence, "similar", rb_LongestSubsequence_similar, 1);
    rb_define_method(rb_mAmatchStringMethods, "longest_subsequence_similar", rb_str_longest_subsequence_similar, 1);

    /* Longest Common Substring */
    rb_cLongestSubstring = rb_define_class_under(rb_mAmatch, "LongestSubstring", rb_cObject);
    rb_define_alloc_func(rb_cLongestSubstring, rb_LongestSubstring_s_allocate);
    rb_define_method(rb_cLongestSubstring, "initialize", rb_LongestSubstring_initialize, 1);
    rb_define_method(rb_cLongestSubstring, "pattern", rb_General_pattern, 0);
    rb_define_method(rb_cLongestSubstring, "pattern=", rb_General_pattern_set, 1);
    rb_define_method(rb_cLongestSubstring, "match", rb_LongestSubstring_match, 1);
    rb_define_method(rb_cLongestSubstring, "similar", rb_LongestSubstring_similar, 1);
    rb_define_method(rb_mAmatchStringMethods, "longest_substring_similar", rb_str_longest_substring_similar, 1);

    /* Jaro */
    rb_cJaro = rb_define_class_under(rb_mAmatch, "Jaro", rb_cObject);
    rb_define_alloc_func(rb_cJaro, rb_Jaro_s_allocate);
    rb_define_method(rb_cJaro, "initialize", rb_Jaro_initialize, 1);
    rb_define_method(rb_cJaro, "pattern", rb_Jaro_pattern, 0);
    rb_define_method(rb_cJaro, "pattern=", rb_Jaro_pattern_set, 1);
    rb_define_method(rb_cJaro, "ignore_case", rb_Jaro_ignore_case, 0);
    rb_define_method(rb_cJaro, "ignore_case=", rb_Jaro_ignore_case_set, 1);
    rb_define_method(rb_cJaro, "match", rb_Jaro_match, 1);
    rb_define_alias(rb_cJaro, "similar", "match");
    rb_define_method(rb_mAmatchStringMethods, "jaro_similar", rb_str_jaro_similar, 1);

    /* Jaro-Winkler */
    rb_cJaroWinkler = rb_define_class_under(rb_mAmatch, "JaroWinkler", rb_cObject);
    rb_define_alloc_func(rb_cJaroWinkler, rb_JaroWinkler_s_allocate);
    rb_define_method(rb_cJaroWinkler, "initialize", rb_JaroWinkler_initialize, 1);
    rb_define_method(rb_cJaroWinkler, "pattern", rb_JaroWinkler_pattern, 0);
    rb_define_method(rb_cJaroWinkler, "pattern=", rb_JaroWinkler_pattern_set, 1);
    rb_define_method(rb_cJaroWinkler, "ignore_case", rb_JaroWinkler_ignore_case, 0);
    rb_define_method(rb_cJaroWinkler, "ignore_case=", rb_JaroWinkler_ignore_case_set, 1);
    rb_define_method(rb_cJaroWinkler, "scaling_factor", rb_JaroWinkler_scaling_factor, 0);
    rb_define_method(rb_cJaroWinkler, "scaling_factor=", rb_JaroWinkler_scaling_factor_set, 1);
    rb_define_method(rb_cJaroWinkler, "match", rb_JaroWinkler_match, 1);
    rb_define_alias(rb_cJaroWinkler, "similar", "match");
    rb_define_method(rb_mAmatchStringMethods, "jarowinkler_similar", rb_str_jarowinkler_similar, 1);

    id_split = rb_intern("split");
    id_to_f = rb_intern("to_f");
}
