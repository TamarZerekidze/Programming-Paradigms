#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(elemSize > 0); assert(initialAllocation >= 0);
    
    v->log_len = 0;
    if(initialAllocation == 0) initialAllocation = 4;
    v->alloc_len = initialAllocation;
    v->elem_size = elemSize;
    v->freeFn = freeFn;
    v->first = malloc(v->alloc_len * v->elem_size);
    assert(v->first != NULL);
}

void VectorDispose(vector *v)
{
    if(v->freeFn == NULL) {free(v->first);return;}
    for (int i = 0; i < v->log_len; i++){
        v->freeFn((char*) v->first + i*(v->elem_size));
    }
    free(v->first);
}

int VectorLength(const vector *v)
{ return v->log_len; }

void *VectorNth(const vector *v, int position)
{ 
  //  assert(position >= 0 && position < v->log_len);
    void* res = (char*) v->first + position*(v->elem_size);
    return res; 
    }

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    void* ptr = VectorNth(v,position);
    if(v->freeFn) v->freeFn(ptr);
    memcpy(ptr,elemAddr,v->elem_size);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    if (v->log_len == v->alloc_len ){
        v->alloc_len *= 2;
        v->first = realloc(v->first,(v->alloc_len)*(v->elem_size));
        assert(v->first != NULL);
    }
    assert (position >=0 && position <= v->log_len);

    void* src = (char*)v->first + position*(v->elem_size);
    void* dst = (char*)src + v->elem_size;
   if( v->log_len > position) memmove(dst,src,(v->log_len - position)*v->elem_size);
    memcpy(src,elemAddr,v->elem_size);
    v->log_len++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
     VectorInsert(v,elemAddr,v->log_len);
}

void VectorDelete(vector *v, int position)
{
    assert( position >= 0 && position < v->log_len);
    void* pos = VectorNth(v,position);
    if(v->freeFn) v->freeFn(pos);
    if(position != (v->log_len - 1)){
    memmove(pos,(char*)pos + v->elem_size ,(v->log_len - position - 1)*v->elem_size);
    }
    v->log_len--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert (compare !=NULL);
    qsort(v->first,v->log_len,v->elem_size,compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn != NULL);
    for (int i = 0; i < v->log_len; i++){
        mapFn(VectorNth(v,i), auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{ 
    assert (searchFn != NULL && key != NULL);
    assert(startIndex >= 0 && startIndex <= v->log_len);
    void* curr;
    void* ptr = VectorNth(v,startIndex);
    size_t sz = v->log_len - startIndex;
    if(isSorted){
        curr = bsearch(key,ptr,sz, v->elem_size, searchFn);
    } else {
        curr = lfind(key,ptr,&sz, v->elem_size, searchFn);
    }
    if (curr == NULL) return kNotFound;
    return (((char*)curr - (char*)v->first) / v->elem_size);
 } 
