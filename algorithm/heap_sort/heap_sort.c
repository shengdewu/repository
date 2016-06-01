#include <stdio.h>
#include "max_heapify.h"
void heap_built(int* A, int length);

void heap_sort(int* A, int length){
	
	heap_built(A, length);
	printf("build:");
	int cnt;
	for(cnt=0; cnt<length; cnt++){
		printf("%d,",A[cnt]);
	}
	
	int index;
	for(index=length; index > 1; index--){
		//exchange A[0] with A[index]
		A[0] = A[0] + A[index];
		A[index] = A[0] - A[index];
		A[0] = A[0] - A[index];	
		length -= 1;
		max_heapify(A, 0, length);
	}
}

void heap_built(int* A, int length){
	int index;
	for(index = length/2; index >= 0; index --)
		max_heapify(A, index, length);	
}
