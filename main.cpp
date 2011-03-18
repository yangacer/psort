#include <cstring>
#include <string>
#include <cstdio>

#include "GAISUtils/record.h"

#define GB *(1024*1024*1024)
#define MB *(1024*1024)
#define KB *(1024)

void 
usage()
{}

int 
main(int argc, char ** argv)
{
	// initiate field factory
	init_field_factory();
	record proto;
	
	// configuration variables
	unsigned int MAXMEM = 10 MB;
	unsigned int RESERVE = 10;

	// parse arguments and configure
	for(int i=0; i< argc; ++i){
		// key 
		if(0 == strcmp(argv[i], "-k")){
			if(argc <= i+2){
				usage();
				exit(1);
			}
			try{
				proto.define_field(argv[i+1], argv[i+2]);
			}catch(char *msg){
				printf("psort: %s\n", msg);
				usage();
				exit(1);
			}
			i+=2;

		}
		// memory
		else if(0 == strcmp(argv[i], "-M")){
			if(argc <= i+1){
				usage();
				exit(1);
			}
			char *memSize = argv[i+1];
			MAXMEM = strtoul(memSize, 0, 10);
			MAXMEM = (MAXMEM==0)?10 MB : MAXMEM;
			printf("Maximum Memory: %d", MAXMEM);
			switch(tolower(memSize[strlen(memSize)-1]))
			{
			case 'g':
				MAXMEM *= 1 GB;
				printf(" G");
			break;
			case 'm':
				MAXMEM *= 1 MB;
				printf(" M");
			break;
			case 'k':
				MAXMEM *= 1 KB;
				printf(" K");
			break;
			}
			printf("B\n");
			i+=1;
		}else if(0 == strcmp(argv[i], "-r")){
			if(argc <= i+1){
				usage();
				exit(1);
			}
			RESERVE = atoi(argv[i+1]);
			RESERVE = (RESERVE <= 0 || RESERVE >= 100)?10 : RESERVE;
			printf("Reserved for partition: %d%%\n", RESERVE);
		}
	}
}
