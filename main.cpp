#include <cstring>
#include <string>
#include <cstdio>
#include <algorithm>

#include "strref.h"
#include "rcmp.h"
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
	field_factory::Instance().Register("STRREF", create_field<str_ref>());
	rschema schema;
	
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
				schema.define_field(argv[i+1], argv[i+2]);
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
		// reserve
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

	try{

		// sort testing
		record proto;
		schema.make(proto);
		std::vector<record> rec(3, proto);

		rec[0].get<str_ref>("@U:").assign("acer", 4);
		rec[0].get<unsigned int>("@s:") = 1234;

		rec[1].get<str_ref>("@U:").assign("acer", 4);
		rec[1].get<unsigned int>("@s:") = 1235;

		rec[2].get<str_ref>("@U:").assign("ace", 3);
		rec[2].get<unsigned int>("@s:") = 1234;

		
		
		record_comparator rcmp;
		char const* keys[3] = {"@U:", "@s:", 0};
		rcmp.set_key_preference(&keys[0], &keys[2]);
		
		std::sort(rec.begin(), rec.end(), rcmp);

		for(int i=0;i<rec.size();++i){
			printf("%s\t%d\n", 
					rec[i].get<str_ref>("@U:").data_, 
					rec[i].get<unsigned int>("@s:")); 

		}

	}catch(char const* msg){
		printf("%s\n", msg);
	}

	char const* teststr = "abcab";
	str_ref x(teststr, 3),
		y(teststr+3, 2);

	printf("x <  y: %d\n", x < y);
	printf("x <= y: %d\n", x <= y);
	
	printf("x >  y: %d\n", x > y);
	printf("x >= y: %d\n", x >= y);

	printf("x == y: %d\n", x == y);
	printf("x != y: %d\n", x != y);


}
