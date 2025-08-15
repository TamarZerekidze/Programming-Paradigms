#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert(elemSize > 0); assert(numBuckets > 0); 
	assert(hashfn != NULL); assert(comparefn != NULL);
	h->num_buck = numBuckets;
	h->num_elem = 0;
	h->hashfn = hashfn;
	h->comparefn = comparefn;
	h->freefn = freefn;
	h->arr = malloc(numBuckets*sizeof(vector));

	for(int i = 0; i < numBuckets; i++){
		VectorNew(&(h->arr[i]),elemSize,h->freefn,4);
	}
}

void HashSetDispose(hashset *h)
{
	for(int i = 0; i < h->num_buck; i++){
		VectorDispose(&(h->arr[i]));
	}
	free(h->arr);
	
}

int HashSetCount(const hashset *h)
{ return h->num_elem; }

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert(mapfn != NULL);
	for (int i = 0; i < h->num_buck; i++){
		VectorMap(&(h->arr[i]),mapfn,auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	assert(elemAddr != NULL);
	int hashIndex = h->hashfn(elemAddr, h->num_buck);
	assert ( hashIndex >= 0 && hashIndex < h->num_buck);
	int pos = VectorSearch(&(h->arr[hashIndex]), elemAddr,h->comparefn,0,false);
	if(pos == -1){
		VectorAppend(&(h->arr[hashIndex]),elemAddr);
		h->num_elem++;
	} else {
		VectorReplace(&(h->arr[hashIndex]),elemAddr,pos);
	}

}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{ 
	assert(elemAddr != NULL);
	int hashIndex = h->hashfn(elemAddr,h->num_buck);
	assert ( hashIndex >= 0 && hashIndex < h->num_buck);
	//vector* v = (char*)h->arr + hashIndex*sizeof(vector);
	int pos = VectorSearch(&(h->arr[hashIndex]), elemAddr,h->comparefn,0,false);
	if(pos == -1) return NULL;
	return VectorNth(&(h->arr[hashIndex]), pos);

}
