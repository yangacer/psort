#include "pmgr.h"

void
partition_mgr::sampling(char const* input_file, rschema const& sche)
{}

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

