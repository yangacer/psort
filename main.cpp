#include <cstring>
#include <string>
#include <cstdio>
#include <algorithm>

#include "strref.h"
#include "rcmp.h"
#include "pmgr.h"
#include "GAISUtils/record.h"
#include "GAISUtils/rstream.h"

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

	// Main components within default setting
	rschema schema;
	record_comparator rcmp;

	irfstream irs("@\n@GAIS_Rec:\n", 13, stdin);
	unsigned int MAXMEM = 10 MB;
	unsigned int RESERVE = 10;

	// ------------ parse arguments and configure -------------
	for(int i=0; i< argc; ++i){
		// key 
		if(0 == strcmp(argv[i], "-k")){
			if(argc <= i+2){
				usage();
				exit(1);
			}
			try{
				schema.define_field(argv[i+1], argv[i+2]);
				// key order checking
				if(argc > i+3){
					if(argv[i+3][0] == '<'){
						rcmp.add_key(argv[i+1], true);
						i++;
					}else if(argv[i+3][0] == '>'){
						rcmp.add_key(argv[i+1], false);
						i++;
					}else{
						rcmp.add_key(argv[i+1]);	
					}
				}
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
			// printf("Maximum Memory: %d", MAXMEM);
			switch(tolower(memSize[strlen(memSize)-1]))
			{
			case 'g':
				MAXMEM *= 1 GB;
				// printf(" G");
			break;
			case 'm':
				MAXMEM *= 1 MB;
				// printf(" M");
			break;
			case 'k':
				MAXMEM *= 1 KB;
				// printf(" K");
			break;
			}
			// printf("B\n");
			i+=1;
		// reserve
		}else if(0 == strcmp(argv[i], "-r")){
			if(argc <= i+1){
				usage();
				exit(1);
			}
			RESERVE = atoi(argv[i+1]);
			RESERVE = (RESERVE <= 0 || RESERVE >= 100)?10 : RESERVE;
			i++;
			// printf("Reserved for partition: %d%%\n", RESERVE);
		}
		// record begin pattern
		else if(0 == strcmp(argv[i], "-b")){
			if(argc <= i+1){
				usage();
				exit(1);
			}
			// translate "\n" to '\n'
			std::string pat(argv[i+1]);
			char const* NL = "\\n";
			std::string::iterator nl_beg = pat.begin();
			while(pat.end() != (nl_beg = std::search(nl_beg, pat.end(), NL, NL+2))){
				pat.replace(nl_beg, nl_beg+2, 1, '\n');
				nl_beg++;
			}
			irs.begin_pattern(pat.c_str(), pat.size());
			i++;
		}
		// input source
		else if(0 == strcmp(argv[i], "-f")){
			if(argc <= i+1){
				usage();
				exit(1);
			}
			irs.close();
			irs.open(argv[i+1], std::ios::binary | std::ios::in);
			if(!irs.is_open()){
				fprintf(stderr, "Open file (%s) failed\n", argv[i+1]);
				usage();
				exit(1);
			}
			i++;
		}
	} // ------------- End of command parsing -----------------
	
	// Setup status
	printf("Record begin pattern: %s\n", irs.begin_pattern());

	// configure partition manager
	partition_mgr pmgr;
	pmgr.mem_limit(MAXMEM).mem_reserve(RESERVE);
	
	try{

		// sort testing
		record proto;
		schema.make(proto);
		std::vector<record> rec(3, proto);

		rec[0].fromString("@U:", "acer", 4);
		rec[0].fromString("@s:", "1235");


		rec[1].fromString("@U:", "acer", 4);
		rec[1].fromString("@s:", "1234");

		rec[2].fromString("@U:", "ace", 3);
		rec[2].fromString("@s:", "12");
		
		/*
		record_comparator rcmp;
		char const* keys[3] = {"@U:", "@s:", 0};
		bool orders[3] = { true, false, false };
		
		rcmp.set_key_preference(&keys[0], &keys[2], &orders[0], &orders[2]);
		*/

		std::sort(rec.begin(), rec.end(), rcmp);

		for(int i=0;i<rec.size();++i){
			printf( rec[i].toString("@U:").c_str() );
			printf("\t");
			printf( rec[i].toString("@s:").c_str() );
			printf("\n");
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
