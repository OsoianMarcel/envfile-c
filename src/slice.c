#import "slice.h"

Slice_t* slice_create(int cap) {
    Slice_t* slice = malloc(sizeof(Slice_t));
    slice->cap = cap;
    slice->len = 0;
    slice->items = malloc(sizeof(int) * cap);

    return slice;
}

void slice_push(Slice_t* slice, int n) {
    // Check if the slice is full.
    if (slice->cap == slice->len) {
        // Double the slice capacity.
        int newCap = slice->cap * 2;
        int* oldPtr = slice->items;
        slice->items = realloc(slice->items, sizeof(int) * newCap);
        if (slice->items == NULL) {
            log_error("Unable to increase the slice capacity.");
            slice->items = oldPtr;
            return;
        }
        slice->cap = newCap;
        log_debug("Info: New slice cap: %d/%d", slice->cap, slice->len);
    }

    slice->items[slice->len++] = n;
    log_debug("items addr: %p", slice->items);
}

void slice_print(Slice_t* slice) {
    log_info("Slice print");
    log_info("cap: %d; len: %d;", slice->cap, slice->len);
    log_info("items addr: %p", slice->items);
    for (int i = 0; i < slice->len; i++) {
        log_info("item: %d", slice->items[i]);
    }
    log_info("/Slice print");
}

void slice_free(Slice_t* slice) {
    free(slice->items);
    free(slice);
}