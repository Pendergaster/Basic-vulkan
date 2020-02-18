/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#define DEFAULT_DYNAMICARRAY_SIZE 6
static void*
dynamicarray_create(const u32 size) {

    u32 allocSize = size * DEFAULT_DYNAMICARRAY_SIZE;
    void* ret = malloc(allocSize + sizeof(u32) * 2);
    if(!ret) return 0;
    u32* s = (u32*)ret;
    s[0] = allocSize;
    s[1] = 0;
    return &s[2];
}

static void
dynamicarray_dispose(void* arr) {

    u32* start = (u32*)arr;
    start -= 2;
    free(start);
}

#define dynamicarray_push_back(ARRAY,VALUE) _push_dynamicArray(&PTR_CAST(void*,(ARRAY)), \
        (VALUE),sizeof *(VALUE))

static void
_push_dynamicArray(void** arr,const void* val,const size_t size) {

    u32* sizes = ((u32*)(*arr)) - 2;
    if(sizes[0] < ((sizes[1] + size))) {
        sizes[0] *= 2;
        sizes = realloc(sizes,sizes[0] + (sizeof(u32) * 2));
        if(!sizes){
            *arr = 0;
            return;
        }
    }
    u8* start = &PTR_CAST(u8,sizes[2]);
    memcpy(&start[sizes[1]],val,size);
    sizes[1] += size;
    *arr = start;
}

#define dynamicarray_size(ARRAY) _dynamicarray_size(ARRAY, sizeof *(ARRAY))

static u32
_dynamicarray_size(void* const arr,size_t size) {

    u32* sizes = (u32*)arr;
    sizes -= 2;
    return sizes[1] / size;
}

#endif /* DYNAMICARRAY_H */
