#ifndef _RCMP_H_
#define _RCMP_H_

#include "GAISUtils/record.h"
#include <vector>
#include <string>

struct record_comparator
{
	
	void
	set_key_preference(char const** first_key, char const** last_key, 
		bool* first_order, bool* last_order);

	void
	add_key(char const *field_name, bool order = true);
	
	void
	flip_order(char const *field_name);

	bool
	operator()(record const &lhs, record const &rhs) const;
	
	void
	reset();
private:
	std::vector<std::string> keys_;
	std::vector<bool> orders_;
		
};

#endif
