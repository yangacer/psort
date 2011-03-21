#ifndef _RCMP_H_
#define _RCMP_H_

#include "GAISUtils/record.h"
#include <vector>
#include <string>

struct record_comparator
{
	
	void
	set_key_preference(char const** first_key, char const** last_key);

	bool
	operator()(record const &lhs, record const &rhs) const;

private:
	std::vector<std::string> keys_;
		
};

#endif
