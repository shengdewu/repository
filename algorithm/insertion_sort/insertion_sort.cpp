#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

//≤Â»Î≈≈–Ú
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

	for(cnt=1; cnt<length; cnt++){
		int key = tech[cnt];
		int pre = cnt - 1;
		while((pre >= 0) && (tech[pre] > key)){
			tech[pre + 1] = tech[pre];
			pre -= 1;
		}
		tech[pre+1] = key;
	}
	
	printf("sort:");
	for(cnt=0; cnt<length; cnt++){
		printf("%d,",tech[cnt]);
	}
	
		
	free(tech);
	tech = NULL;
	return 0;
}
