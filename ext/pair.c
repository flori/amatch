#include "pair.h"

#define DEBUG 0

static int predict_length(VALUE tokens)
{
    int i, l, result;
    for (i = 0, result = 0; i < RARRAY_LEN(tokens); i++) {
        VALUE t = rb_ary_entry(tokens, i);
        l = (int) RSTRING_LEN(t) - 1;
        if (l > 0) result += l;
    }
    return result;
}

PairArray *PairArray_new(VALUE tokens)
{
    int i, j, k, len = predict_length(tokens); 
    PairArray *pair_array = ALLOC(PairArray);
    Pair *pairs = ALLOC_N(Pair, len);
    MEMZERO(pairs, Pair, len);
    pair_array->pairs = pairs;
    pair_array->len = len;
    for (i = 0, k = 0; i < RARRAY_LEN(tokens); i++) {
        VALUE t = rb_ary_entry(tokens, i);
        char *string = RSTRING_PTR(t);
        for (j = 0; j < RSTRING_LEN(t) - 1; j++) {
            pairs[k].fst = string[j];
            pairs[k].snd = string[j + 1];
            pairs[k].status = PAIR_ACTIVE;
            k++;
        }
    }
    return pair_array;
}

void pair_array_reactivate(PairArray *self)
{
    int i;
    for (i = 0; i < self->len; i++) { 
        self->pairs[i].status = PAIR_ACTIVE;
    }
}

double pair_array_match(PairArray *self, PairArray *other)
{
    int i, j, matches = 0;
    int sum = self->len + other->len;
    if (sum == 0) return 1.0;
    for (i = 0; i < self->len; i++) {
        for (j = 0; j < other->len; j++) {
#if DEBUG
            pair_print(self->pairs[i]);
            putc(' ', stdout);
            pair_print(other->pairs[j]);
            printf(" -> %d\n", pair_equal(self->pairs[i], other->pairs[j]));
#endif
            if (pair_equal(self->pairs[i], other->pairs[j])) {
                matches++;
                other->pairs[j].status = PAIR_INACTIVE;
                break;
            }
        }
    }
    return ((double) (2 * matches)) / sum;
}

void pair_print(Pair pair)
{
    printf("%c%c (%d)", pair.fst, pair.snd, pair.status);
}

void pair_array_destroy(PairArray *pair_array)
{
    if (pair_array->pairs) {
        xfree(pair_array->pairs);
    }
    xfree(pair_array);
}
