#include "pmgr.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <fstream>

#include "GAISUtils/rschema.h"
#include "GAISUtils/rstream.h"

void
partition_mgr::sampling(irfstream &is, rschema const& sche)
{
	/*
	struct stat stbuf;
	if(0 > stat(input_file, &stbuf)){
		perror("partition_mgr(sampling): ");
		return;
	}

	unsigned long long size = stbuf.st_size;
	unsigned int mem_avail = mem_limit_ - mem_limit_ * mem_reserve_ / 100;
	printf( "input size: %llu\n"
		"memory limitation: %lu\n"
		"memory reservation: %lu%%\n"
		"memory available: %lu\n"
		"partition cnt: %llu\n", 
		size, mem_limit_, mem_reserve_, mem_avail, size / mem_avail );
	*/	
}

void
partition_mgr::dispatch(record const &r)
{}

void
partition_mgr::report()
{}

partition_mgr&
partition_mgr::mem_limit(unsigned int size)
{ mem_limit_ = size; return *this; }

partition_mgr&
partition_mgr::mem_reserve(unsigned int percentage)
{ mem_reserve_ = percentage; return *this; }

unsigned int
partition_mgr::mem_limit() const
{ return mem_limit_; }

unsigned int
partition_mgr::mem_reserve() const
{ return mem_reserve_; }

