#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "max_heapify.h"

int main(int argc, char *argv[])
{
	int length = argc-1;
	int *tech = NULL;
	if(length <= 0)
		return 0;

	tech = (int*)malloc(length*sizeof(int));
	int cnt;
	for(cnt = 0; cnt<length; cnt++){
		tech[cnt] = atoi(argv[cnt+1]);
	}

	int index;
	for(index = length/2; index >= 0; index --)
		max_heapify(tech, index, length);
	
	printf("sort:");
	for(cnt=0; cnt<length; cnt++){
		printf("%d,",tech[cnt]);
	}
	
		
	free(tech);
	tech = NULL;
	return 0;
}
