
void swap(int* a, int* b);
int partition(int* A, int begin, int end);

void quick_sort(int*	A, int begin, int end){
	if( begin >= end ){
		return;
	}
	
	int pInt = partition(A, begin, end);
	quick_sort(A, begin, pInt-1);
	quick_sort(A, pInt+1, end);
}


int partition(int* A, int begin, int end){
	int key = A[end];
	int retIndex = begin - 1;
	int index;
	for(index = begin; index < end; index++){
		if(A[index] < key){
			retIndex += 1;
			swap(&A[retIndex], &A[index]);
		}
	}
	retIndex +=1 ;
	swap(&A[retIndex], &A[end]);
	
	return retIndex;
}

void swap(int* a, int* b){
	int temp = *a;
	*a = *b;
	*b = temp;
}