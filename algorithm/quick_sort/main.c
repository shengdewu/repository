#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "quick_sort.h"

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

	quick_sort(tech, 0, length-1);
	
	printf("sort:");
	for(cnt=0; cnt<length; cnt++){
		printf("%d,",tech[cnt]);
	}
	
		
	free(tech);
	tech = NULL;
	return 0;
}
