#ifndef _PMGR_H
#define _PMGR_H

#include <vector>
#include <string>
#include <iosfwd>
//#include <fstream>
//#include "GAISUtils/record.h"
//#include "GAISUtils/rschema.h"

class irfstream;
struct record;
struct rschema;

struct partition_mgr
{
	typedef std::fstream PartHandle;

	/** Sample input file to generate pivots
	 *  @param input_file
	 *  @param sche Record schema for reading record accordingly
	 */
	void
	sampling(irfstream &is, rschema const& sche);

	/** Dispatch record to a partition
	 */
	void
	dispatch(record const& r);

	/** Status dump
	 */
	void
	report();

	// Configurations
	partition_mgr&
	mem_limit(unsigned int size);

	partition_mgr&
	mem_reserve(unsigned int percentage);
	
	// Configuration getter
	unsigned int
	mem_limit() const;

	unsigned int
	mem_reserve() const;

private:
	unsigned int mem_limit_;
	unsigned int mem_reserve_;
	unsigned int part_cnt_;
	std::vector<record> *pivots_;
	std::vector<std::string> *part_names_;
	std::vector<PartHandle> *part_handles_;
};

#endif
