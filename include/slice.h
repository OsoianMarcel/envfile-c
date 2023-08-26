#ifndef SLICE_H
#define SLICE_H

#include <stdlib.h>
#include "logging.h"

struct Slice_s {
    int cap;
    int len;
    int *items;
};
typedef struct Slice_s Slice_t;

extern Slice_t* slice_create(int cap);
extern void slice_push(Slice_t* slice, int n);
extern void slice_print(Slice_t* slice);
extern void slice_free(Slice_t* slice);

#endif