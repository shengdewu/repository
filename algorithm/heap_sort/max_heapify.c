#include <stdio.h>

int parent_index(int pos);
int left_index(int pos);
int right_index(int pos);

int max_heapify(int *A, int i, int heapSize){
	int l = left_index(i);
	int r = right_index(i);
	int largest;
	
	if((l < heapSize) && (A[l] >  A[i])){
		largest = l;
	}
	else{
		largest = i;
	}
	
	if((r < heapSize) && (A[r] >  A[largest])){
		largest = r;
	}
	
	if(largest != i){
		int temp = 0;
		temp = A[i];
		A[i] = A[largest];
		A[largest] = temp;
		max_heapify(A, largest, heapSize);
	}
		
}

int parent_index(int pos){
	if(0 == pos)
		return pos;
	return ((pos-1)/2);
}

int left_index(int pos){
	return ((pos * 2) + 1);
}

int right_index(int pos){
	return ((pos+1) * 2);
}