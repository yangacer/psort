#include <cstring>
#include <cmath>
#include <string>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <limits>
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
	
	irfstream irs("@\n", 2, stdin, std::ios::in | std::ios::binary, BUFSIZ); 
	irs.begin_pattern("@\n",2);

	unsigned int MAXMEM = 10 MB;
	unsigned int RESERVE = 10;
	unsigned int FILE_SIZE = 0; // zero for unknow size
	char const* FILENAME = 0;
	char const* PVFILENAME = "pv.file";

	// ------------ parse arguments and configure -------------
	for(int i=1; i< argc; ++i){
		// key 
		if(0 == strcmp(argv[i], "-k")){
			if(argc <= i+2){
				usage();
				exit(1);
			}
			try{
				if(0 == strcmp(argv[i+2],"STR"))
					schema.define_field(argv[i+1], "STRREF");
				else 
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
			switch(tolower(memSize[strlen(memSize)-1]))
			{
			case 'g':
				MAXMEM *= 1 GB;
			break;
			case 'm':
				MAXMEM *= 1 MB;
			break;
			case 'k':
				MAXMEM *= 1 KB;
			break;
			}
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
		}else if(0 == strcmp(argv[i], "-p")){
			if(argc <= i+1){
				usage();
				exit(1);
			}
			PVFILENAME = argv[i+1];
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

	char *streambuf = new char[MAXMEM];
	if(0 == streambuf){
		perror("psort(memory low):");
		exit(1);
	}
	if(0 == irs.rdbuf()->pubsetbuf(streambuf, streambuf_size) ){
		perror("psort(setbuf):");
		exit(1);
	}
	if(FILE_SIZE){
		irs.open(FILENAME, std::ios::binary | std::ios::in);
		if(!irs.is_open()){
			fprintf(stderr, "Open file (%s) failed\n", FILENAME);
			usage();
			exit(1);
		}
	}

	// estimate partition count
	unsigned int pivots_count = FILE_SIZE / streambuf_size;
	if(pivots_count * streambuf_size < FILE_SIZE)
		pivots_count++;

	// in-memory sortable checking
	if(streambuf_size > FILE_SIZE)
		pivots_count = std::numeric_limits<unsigned int>::max();

	printf(	"PIVOTS_CNT: %lu\t"
		"MAXMEM: %lu\t"
		"RESERVE: %lu\t"
		"STREAM SIZE: %lu\n", pivots_count, MAXMEM, RESERVE, streambuf_size);

	// ------------- sampling stage (integrate into pmgr latter ---------------
	unsigned int recSize(0);
	char const* recData(0);
	unsigned int ignoreCnt(0);
	record rec;
	schema.make(rec);
	std::vector<record> pivots;
	std::string pivotsBuf;
	
	while(1){
		recSize = irs.getrecord(&recData);
		
		if(!irs.fail()){
			// find fields and create record
			pivots.push_back(rec);
			record& last(*(pivots.end()-1));
			fromGAISRecord(last, recData, recSize);
			// copy strref data to buffer
			cp_strref(pivotsBuf, last);
			ignoreCnt += recSize;
		}else if(irs.rdbuf()->in_avail() < streambuf_size -1){
			break;	
		}
		
		// ignore all followed in-buffer records
		while(1){
			recSize = irs.getrecord(&recData);
			if(!irs.fail())
				ignoreCnt += recSize;
			else{
				irs.clear();
				irs.seekg(ignoreCnt, std::ios::cur);
				irs.research();
				ignoreCnt = 0;
				break;
			}
		}	
		
	}
	
	rebuild_ref(pivotsBuf, &*pivots.begin(), &*pivots.end());

	std::cout<<pivots.size()<<" pivots generated\n";

	std::sort(pivots.begin(), pivots.end(), FunctorWrapper<record_comparator>(rcmp));
	
	std::ofstream pvfile(PVFILENAME, std::ios::binary);
	for(int i=0;i<pivots.size();++i){
		pvfile<<irs.begin_pattern();
		toGAISRecord(pivots[i], pvfile);
	}
	pvfile.close();
	printf("Pivots file generated\n");

	// ------------------ partition stage -------------------

	std::vector<std::ofstream*> fouts(pivots.size()+1);
	std::vector<std::string> fnames;
	std::vector<record> in_mem_rec;

	// setup new buffer size to irs
	irs.close();
	streambuf_size = MAXMEM;
	irs.rdbuf()->pubsetbuf(streambuf, streambuf_size);
	irs.open(FILENAME, std::ios::binary);

	// configure partition
	int fname_digits = (int)ceil(log10((double)fouts.size()));
	std::stringstream cvt;
	for(int i=0; i<fouts.size(); i++){
		cvt<<std::setw(fname_digits)<<std::setfill('0')<<i<<".part";
		fouts[i] = new std::ofstream(cvt.str().c_str(), std::ios::binary | std::ios::trunc | std::ios::out);
		if(!fouts[i]->is_open()){
			perror("psort(open partition)");
			exit(1);
		}
		fnames.push_back(cvt.str());
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
			record &last(*(in_mem_rec.end()-1));
			fromGAISRecord(last, recData, recSize);

			// store pointer to full record if one-pass sort is satisfied
			last.get<str_ref>("_raw").assign(recData, recSize);
			
			readCnt += recSize + irs.psize();
		}else{
			// dispatch in-buffer records
			std::vector<record>::iterator iter = in_mem_rec.begin(), upper;
			for(;iter != in_mem_rec.end();++iter){
				upper = std::upper_bound(pivots.begin(), pivots.end(), *iter, 
					FunctorWrapper<record_comparator>(rcmp));
				
				// write to file
				(*fouts[upper - pivots.begin()]) << 
					irs.begin_pattern()<<
					(iter->get<str_ref>("_raw"));
			}

			// flush stream
			in_mem_rec.clear();
			irs.clear();
			irs.seekg(readCnt, std::ios::cur);
			irs.research();
			readCnt = 0;
			// test successive fail
			recSize = irs.getrecord(&recData);
			if(irs.fail()){
				if(recSize && irs.rdbuf()->in_avail() + 1 < streambuf_size){
					record tmp(rec);
					fromGAISRecord(tmp, recData, recSize);
					tmp.get<str_ref>("_raw").assign(recData, recSize);
					upper = std::upper_bound(pivots.begin(), pivots.end(), tmp, rcmp);
					
					// write to file
					(*fouts[upper - pivots.begin()]) << 
						irs.begin_pattern()<<
						tmp.get<str_ref>("_raw");

					// output size report
					bool overflowFlag = false;
					for(int i=0;i<fouts.size();++i){
						double fraction = (size_t)fouts[i]->tellp()/(double)MAXMEM;
						if(fraction > 1){
							printf("%s:\t" "%12lu\t" "%f overflowed.", 
									fnames[i].c_str(), 
									(size_t)fouts[i]->tellp(), 
									fraction);
							overflowFlag = true;
						}
					}
					if(overflowFlag){
						fprintf(stderr, "Some partition exceeds max memory"
							" limitation and require repartition\n");
						return 0;
					}				
					
					fprintf(stderr, "Partition done.\n");
				}else{
					fprintf(stderr, "Record size exceeds max memory limitation\n");
					exit(1);
				}
				break;	
			}else{
				// find fields and create record
				in_mem_rec.push_back(rec);
				record &last(*(in_mem_rec.end()-1));
				fromGAISRecord(last, recData, recSize);

				// store pointer to full record if one-pass sort is satisfied
				last.get<str_ref>("_raw").assign(recData, recSize);

				readCnt += recSize + irs.psize();
	
			}
		}
	}

	// ------------- internal sorting stage --------
	
	fprintf(stderr, "We assume no repartition in current version!\n");
	
	// close and free part files
	for(int i=0; i< fouts.size(); ++i){
		fouts[i]->close();
		delete fouts[i];
		fouts[i] = 0;
	}
	fouts.clear();
	
	// cleanup structures will be used
	irs.close();
	in_mem_rec.clear();

	std::ofstream output("output.file", std::ios::out | std::ios::trunc | std::ios::binary);

	// read records
	for(int i=0; i<fnames.size(); ++i){
		irs.open(fnames[i].c_str(), std::ios::binary);
		if(!irs.is_open()){
			perror("psort(open part file)");
			exit(1);
		}
		while(1){
			recSize = irs.getrecord(&recData);
			// skip null file
			if(!irs.rdbuf()->in_avail())
				break;
			in_mem_rec.push_back(rec);
			record &last(*(in_mem_rec.end()-1));
			fromGAISRecord(last, recData, recSize);
			
			// store pointer to full record if one-pass sort is satisfied
			last.get<str_ref>("_raw").assign(recData, recSize);

			if(irs.fail()){
				// sort and output
				std::stable_sort(in_mem_rec.begin(), in_mem_rec.end(), 
					FunctorWrapper<record_comparator>(rcmp));

				std::vector<record>::iterator iter = in_mem_rec.begin();
				while(iter != in_mem_rec.end()){
					output<<irs.begin_pattern()<<
						iter->get<str_ref>("_raw");
					++iter;	
				}
				in_mem_rec.clear();
				break;	
			}

		}
		irs.close();
	}
	output.close();

	// ------------- free stage ---------------------
	
	

	// ----------- misc test --------------------------
	
	return 0;
}
