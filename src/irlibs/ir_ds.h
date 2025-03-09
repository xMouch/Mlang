#pragma once

#include "ir_types.h"
#include "ir_memory.h"

#ifndef IR_ASSERT
#define IR_ASSERT(ASSERT)
#define IR_NOT_NULL(PTR)
#define IR_INVALID_CASE
#define IR_SOFT_ASSERT(ASSERT)
#endif


struct Dyn_Array_Header
{
    msi length;
    msi capacity;
    Heap_Allocator* heap;
};

inline
Dyn_Array_Header* arr_header(void* arr)
{
    return (Dyn_Array_Header*) (arr) - 1;
}

inline
void* arr_growth_helper(void* arr, msi elem_size, msi min_cap)
{
    IR_NOT_NULL(arr);
    Dyn_Array_Header* header = arr_header(arr);
    IR_ASSERT(header->length < 0xffffffffffffff && "ARRAY length underrun?");
    if(min_cap < header->length)
    {
        return arr;
    }
    msi optimal_size = sizeof(Dyn_Array_Header) + elem_size * min_cap + sizeof(Heap_Partition);
    optimal_size = u64_max(u64_get_nearest_higher_or_equal_pow2(optimal_size), 1 << header->heap->min_exp);
    msi optimal_cap = (optimal_size - sizeof(Dyn_Array_Header) - sizeof(Heap_Partition))/elem_size;
    if(optimal_cap > header->capacity)
    {
        if(DYN_REALLOC(header, optimal_size, header->heap))
        {
            header->capacity=optimal_cap;
        }
        else
        {
            IR_ASSERT(false && "Array growth failed");
        }
        
        return header+1;
    }
    return arr;
}

inline 
void* arr_maybe_growth_helper(void* arr, msi elem_size, msi grow_by)
{
    IR_NOT_NULL(arr); 
    Dyn_Array_Header* header = arr_header(arr);
    msi min_cap = header->length + grow_by; 
    if(min_cap > header->capacity)
    {
        return arr_growth_helper(arr, elem_size, min_cap);
    }
    else
    {
        return arr;
    }
}


inline 
void* init_dyn_arr_helper(void* arr, msi elem_size, msi init_cap, Heap_Allocator* heap)
{
    IR_ASSERT(arr == nullptr);
    msi optimal_size = sizeof(Dyn_Array_Header) + elem_size * init_cap + sizeof(Heap_Partition);
    optimal_size = u64_max(u64_get_nearest_higher_or_equal_pow2(optimal_size), 1 <<heap->min_exp);
    u8* result = (u8*)DYN_ALLOC(optimal_size, heap);
    
    if(result)
    {
        msi optimal_cap = (optimal_size - sizeof(Dyn_Array_Header) - sizeof(Heap_Partition))/elem_size;
        result += sizeof(Dyn_Array_Header);
        *arr_header(result)={0,optimal_cap, heap};
    }
    IR_SOFT_ASSERT(result && "Could not initialize array because the heap allocation failed!");
    return result;
}

inline
b8 arr_del_n_helper(void* arr, msi elem_size, msi index, msi n)
{
    Dyn_Array_Header* header = arr_header(arr);
    IR_ASSERT(index + n <= header->length);
    b8 result = false;
    msi size = (header->length - (index + n)) * elem_size;
    result = copy_buffer(IR_WRAP_INTO_BUFFER((((u8*)arr)) + ((index + n) * elem_size), size), 
                         IR_WRAP_INTO_BUFFER(((u8*)arr) + (index * elem_size), size));
    IR_ASSERT(result);
    header->length -= n;
    
    return result;
}

inline
void* arr_insert_n_helper(void* arr, msi elem_size, msi index, msi n)
{
    
    Dyn_Array_Header* header = arr_header(arr);
    if(index >= header->length)
    {
        arr = arr_maybe_growth_helper(arr, elem_size, index - header->length + n);
    }
    else
    {
        arr = arr_maybe_growth_helper(arr, elem_size, n);
    }
    header = arr_header(arr);
    u8* start = ((u8*)arr) + (index*elem_size);
    msi size = (header->length - index) * elem_size;
    msi offset = n * elem_size;
    
    if(index >= header->length)
    {
        zero_buffer(IR_WRAP_INTO_BUFFER(((u8*)arr)+(header->length * elem_size),
                                        (index - header->length) * elem_size));
        header->length = index + n;
    }
    else
    {
        copy_buffer_reverse(IR_WRAP_INTO_BUFFER(start, size), IR_WRAP_INTO_BUFFER(start+offset, size));
        header->length+=n;
    }
    
    
    return arr;    
}

//TODO(Michael) Complete Documentation
/* DOCUMENTATION MSL DYNAMIC ARRAYS
 *
 * Declare an empty dynamic array of type T
 *  T* foo = nullptr;
 *  ARR_INIT(foo, init_cap, heap);
 * 
 * Access ith item in the array
 *  T item = foo[i];
 *
 *  Macro API
 *
 *  ARR_INIT:
 *     
 *     void ARR_INIT(T* arr, msi init_cap, Heap_Allocator* heap)
 *      Initializes arr with the heap and a start capacity
 *      The given heap allocator will be used for all further operations
 *
 * 
*/
#define ARR_INIT(arr, init_cap, heap_ptr) (arr = ((typeof(arr))init_dyn_arr_helper(arr, sizeof(*(arr)), init_cap, heap_ptr)))
#define ARR_FREE(arr) (heap_free(arr_header(arr), arr_header(arr)->heap) ? ((arr)=nullptr, true) : ((arr)=nullptr, false))
#define ARR_DEL_ALL(arr) (arr_header(arr)->length=0)
#define ARR_LEN_S(arr) ((s64)arr_header((arr))->length)
#define ARR_LEN(arr)  ((msi)arr_header((arr))->length)
#define ARR_PUSH(arr, elem)  (((arr) = (typeof(arr))arr_maybe_growth_helper((arr), sizeof(*(arr)), 1)) ? \
&((arr)[arr_header((arr))->length++] = (elem)) : nullptr)
#define ARR_POP(arr) (arr_header((arr))->length--, (arr)[arr_header((arr))->length])
#define ARR_DEL(arr, i) (arr_del_n_helper((arr), sizeof(*(arr)), (i), 1))
#define ARR_DEL_N(arr, i, n) (arr_del_n_helper((arr), sizeof(*(arr)), (i), (n)))
#define ARR_DEL_SWAP(arr, i) ((arr)[(i)] = (arr)[--arr_header((arr))->length])
#define ARR_CAP(arr) (arr_header((arr))->capacity)
#define ARR_SET_CAP(arr, n) ((arr) = (typeof(arr))arr_growth_helper((arr), sizeof(*(arr)), (n)))
#define ARR_INS_N(arr, i, n) ((arr) = (typeof(arr))arr_insert_n_helper((arr), sizeof(*(arr)), (i), (n)))
#define ARR_INS(arr, i, elem) ((arr) = ((typeof(arr))arr_insert_n_helper((arr), sizeof(*(arr)), (i), 1)), &((arr)[(i)] = (elem)))
#define ARR_ADD_N_PTR(arr, n) ((arr) = ((typeof(arr))arr_maybe_growth_helper((arr), sizeof(*(arr)), (n))),\
arr_header((arr))->length+=(n), &(arr)[arr_header((arr))->length-(n)])
#define ARR_ADD_N_INDEX(arr, n) ((arr) = ((typeof(arr))arr_maybe_growth_helper((arr), sizeof(*(arr)), (n))),\
arr_header((arr))->length+=(n), arr_header((arr))->length-(n))
#define ARR_LAST(arr) ((arr)[arr_header((arr))->length-1])





struct Bucket_Array_Header
{
    msi length;
    msi bucket_size;
    u8** buckets;
    Heap_Allocator* heap;
};


inline
Bucket_Array_Header* ba_header(void* ba)
{
    return (Bucket_Array_Header*)ba;
}


inline
void* init_ba_helper(void* ba, msi elem_size, msi bucket_size, Heap_Allocator* heap)
{
    IR_ASSERT(ba==nullptr);
    IR_ASSERT(bucket_size!=0);
    IR_ASSERT(heap!=nullptr);
    Bucket_Array_Header* result = (Bucket_Array_Header*)DYN_ALLOC(sizeof(Bucket_Array_Header), heap);
    
    if(result)
    {
        result->length = 0;
        result->bucket_size = bucket_size;
        result->heap = heap;
        result->buckets = nullptr;
        
        
        ARR_INIT(result->buckets, 1, heap);
        if(result->buckets == nullptr)
        {
            IR_SOFT_ASSERT(result->buckets && "Could not initialize bucket list because the heap allocation failed!");
            DYN_FREE(result, heap);
            result = nullptr;   
        }
        else
        {
            u8* first_bucket = (u8*)DYN_ALLOC(elem_size * bucket_size, heap);
            if(!first_bucket)
            {
                IR_SOFT_ASSERT(first_bucket && "Could not initialize bucket list because the heap allocation failed!");
                ARR_FREE(result->buckets);
                DYN_FREE(result, heap);
                result = nullptr;
            }
            else
            {
                ARR_PUSH(result->buckets, first_bucket); 
            }
        }
    }
    IR_SOFT_ASSERT(result && "Could not initialize bucket list because the heap allocation failed!");
    return result;
}

inline
b8 free_ba_helper(void* ba)
{
    Bucket_Array_Header* header = ba_header(ba);
    b8 result = true;
    for(msi i = 0; i < ARR_LEN(header->buckets); ++i)
    {
        result = DYN_FREE(header->buckets[i], header->heap);
        IR_ASSERT(result && "Bucket List Free is failing to free the individual buckets!");
    }
    result = ARR_FREE(header->buckets);
    IR_ASSERT(result && "Bucket List Free is failing to free the list of bucket pointers!");
    
    result = DYN_FREE(header, header->heap);
    IR_ASSERT(result && "Bucket List Free is failing to free the header!");
        
    return result;
}

inline
b8 ba_maybe_growth_helper(void* ba, msi elem_size)
{
    IR_NOT_NULL(ba);
    Bucket_Array_Header* header = ba_header(ba);
    
    if(header->length == (ARR_LEN(header->buckets) * header->bucket_size))
    {
        u8* new_bucket = (u8*)DYN_ALLOC(elem_size * header->bucket_size, header->heap);
        if(!new_bucket)
        {
            IR_SOFT_ASSERT(false && "Bucket List failed allocate new space for elements!");
            return false;
        }
        else
        {
            if(!ARR_PUSH(header->buckets, new_bucket))
            {
                IR_SOFT_ASSERT(false && "Bucket List failed to grow bucket pointer list!");
                return false;
            }
        }
    }
    return true;
}

inline 
void* ba_get_helper(void* ba, msi elem_size, msi index)
{
    IR_NOT_NULL(ba);
    Bucket_Array_Header* header = ba_header(ba);
    //IR_ASSERT(index>=header->length && "Out of bounds access to bucket list!");
    msi bucket_index = index/header->bucket_size;
    msi in_bucket_index = index%header->bucket_size;
    
    return &header->buckets[bucket_index][in_bucket_index*elem_size];
}

#define BA_INIT(ba, bucket_size, heap_ptr)(ba = ((typeof(ba))init_ba_helper(ba, sizeof(*(ba)), (bucket_size), (heap_ptr))))
#define BA_DEL_ALL(ba)(ba_header(ba)->length=0)
#define BA_FREE(ba)(free_ba_helper((ba)) ? ((ba)=nullptr, true) : ((ba)=nullptr, false))
#define BA_LEN_S(ba)((s64)ba_header((ba))->length)
#define BA_LEN(ba)((msi)ba_header((ba))->length)
#define BA_GET(ba, index) ((typeof((ba)))ba_get_helper((ba), sizeof(*(ba)), (index)))
#define BA_PUSH(ba, elem)((ba_maybe_growth_helper((ba), sizeof(*(ba)))) ? \
&(*((typeof((ba)))ba_get_helper(ba, sizeof((*ba)), ba_header(ba)->length++)) = (elem)) : nullptr)
#define BA_LAST(ba) ((typeof(*(ba)))ba_get_helper((ba), sizeof(*(ba)), ba_header(ba)->length-1))