#include <cstring>
#include <cmath>
#include <string>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>

#include "strref.h"
#include "rcmp.h"
#include "pmgr.h"
#include "GAISUtils/record.h"
#include "GAISUtils/rstream.h"
#include "GAISUtils/rserialize.h"

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
	
	irfstream irs; //("@\n@GAIS_Rec:\n", 13, stdin);

	unsigned int MAXMEM = 10 MB;
	unsigned int RESERVE = 10;
	unsigned int FILE_SIZE = 0; // zero for unknow size
	char const* FILENAME = 0;

	// ------------ parse arguments and configure -------------
	for(int i=1; i< argc; ++i){
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
			// get file size
			struct stat statbuf;
			if(0 < stat(argv[i+1], &statbuf)){
				perror("psort(stat):");
				exit(1);
			}
			FILE_SIZE = statbuf.st_size;
			FILENAME = argv[i+1];
			
			i++;
		}
	} // ------------- End of command parsing -----------------
	
	if(FILENAME == 0){
		printf("-f file\n");
		exit(1);
	}

	// Setup status
	printf("Record begin pattern: %s\n", irs.begin_pattern());

	// configure partition manager
	partition_mgr pmgr;
	pmgr.mem_limit(MAXMEM).mem_reserve(RESERVE);
	
	// configure irfstream
	unsigned int streambuf_size = 
		(MAXMEM > 100) ? 
			MAXMEM - MAXMEM / 100 * RESERVE :
			MAXMEM - MAXMEM * RESERVE / 100;

	char *streambuf = new char[streambuf_size];
	if(0 == streambuf){
		perror("psort(memory low):");
		exit(1);
	}
	if(0 == irs.rdbuf()->pubsetbuf(streambuf, streambuf_size) ){
		perror("psort(setbuf):");
		exit(1);
	}
	irs.open(FILENAME, std::ios::binary | std::ios::in);
	if(!irs.is_open()){
		fprintf(stderr, "Open file (%s) failed\n", FILENAME);
		usage();
		exit(1);
	}
	
	// configure rschema
	schema.define_field("_raw", "STRREF");

	// estimate partition count
	unsigned int part_count = (FILE_SIZE == 0) ? 20 : FILE_SIZE / streambuf_size;
	if(part_count * streambuf_size < FILE_SIZE)
		part_count++;

	printf(	"MAXMEM: %lu\t"
		"RESERVE: %lu\t"
		"STREAM SIZE: %lu\n", MAXMEM, RESERVE, streambuf_size);

	// ------------- sampling stage (integrate into pmgr latter ---------------
	char *pivotsBuf(0);
	unsigned int pivotsBufSize(0);
	unsigned int recSize(0);
	char const* recData(0);
	record rec;
	schema.make(rec);
	std::vector<record> pivots;

	while(1){
		recSize = irs.getrecord(&recData);
		
		// find fields and create record
		pivots.push_back(rec);
		fromGAISRecord(*(pivots.end()-1), recData, recSize);
		pivotsBufSize += referenced_count(*(pivots.end()-1));
		
		// store pointer to full record if one-pass sort is satisfied
		(pivots.end()-1)->get<str_ref>("_raw").assign(recData, recSize);

		if(irs.fail()){
			// input data size < MAXMEM
			if(irs.rdbuf()->in_avail() + 1 < streambuf_size){
				// sort and output
				sort(pivots.begin(), pivots.end(), rcmp);
				for(unsigned int i=0; i<pivots.size();++i)
					std::cout<<pivots[i].get<str_ref>("_raw");	
				
				return 0;
			}
			break;
		}
		
	}

	// copy data located in irs to pivotsBuf and change str_ref.data_
	if(pivotsBufSize){
		pivotsBuf = new char[pivotsBufSize+1];
		unsigned int copied(0);
		for(unsigned int i=0; i<pivots.size();++i)
			copied += cp_chg_referenced(pivotsBuf + copied, pivots[i]);
		// verify the last record completeness
		irs.clear();
		irs.ignore(irs.rdbuf()->in_avail());
		if(!irs.eof())
			pivots.pop_back();
		std::cout<<str_ref(pivotsBuf, pivotsBufSize)<<"\n";
	}
	
	sort(pivots.begin(), pivots.end(), rcmp);
	
	// undefine _raw field for output pivots
	schema.undefine_field("_raw");
	
	std::ofstream pvfile("pv.file", std::ios::binary);
	for(int i=0;i<pivots.size();++i){
		pvfile<<irs.begin_pattern();
		toGAISRecord(pivots[i], pvfile);
	}

	// ------------------ partition stage -------------------

	std::vector<std::ofstream*> fouts(pivots.size()+1);
	std::vector<record> in_mem_rec;

	// reset irs
	irs.clear();
	irs.seekg(0, std::ios::beg);
	irs.research();
	
	// configure partition
	int fname_digits = (int)ceil(log10((double)fouts.size()));
	std::stringstream cvt;
	for(int i=0; i<fouts.size(); i++){
		cvt<<std::setw(fname_digits)<<std::setfill('0')<<i<<".part";
		fouts[i] = new std::ofstream(cvt.str().c_str(), std::ios::binary);
		if(!fouts[i]->is_open()){
			perror("psort(open partition):");
			exit(1);
		}
		cvt.str("");
	}

	// dispatch record to partition
	unsigned long long readCnt(0);
	schema.define_field("_raw", "STRREF");
	schema.make(rec);
	while(1){
		recSize = irs.getrecord(&recData);
		if(!irs.fail()){
			// find fields and create record
			in_mem_rec.push_back(rec);
			fromGAISRecord(*(in_mem_rec.end()-1), recData, recSize);

			// store pointer to full record if one-pass sort is satisfied
			(in_mem_rec.end()-1)->get<str_ref>("_raw").assign(recData, recSize);
			
			readCnt += recSize;
		}else{
			// dispatch in-buffer records
			std::vector<record>::iterator iter = in_mem_rec.begin(), upper;
			for(;iter != in_mem_rec.end();++iter){
				upper = std::upper_bound(pivots.begin(), pivots.end(), *iter, rcmp);
				(*fouts[upper - pivots.begin()]) << 
					irs.begin_pattern()<<
					(iter->get<str_ref>("_raw"));
			}
			in_mem_rec.clear();
			irs.clear();
			irs.ignore(readCnt);
			readCnt = 0;
			break;	
		}
	}

	// free stage
	for(int i=0; i< fouts.size(); ++i){
		fouts[i]->close();
		delete fouts[i];
		fouts[i] = 0;
	}

	if(pivotsBuf){
		delete [] pivotsBuf;	
	}

	// ----------- misc test --------------------------
	
	return 0;
}
