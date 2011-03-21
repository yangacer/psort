#include "rcmp.h"



void
record_comparator::set_key_preference(char const **first_key, char const **last_key)
{
	while(first_key != last_key){
		keys_.push_back(*first_key);
		++first_key;
	}
}

bool
record_comparator::operator()(record const &lhs, record const &rhs) const
{
	typedef std::vector<std::string>::const_iterator iterator;
	iterator iter = keys_.begin();
	int cond(0);
	while(iter != keys_.end()){
		if(0 < (cond = lhs.compare(iter->c_str(), rhs))) 
			return false;
		else if(0 > cond)
			return true;
		++iter;	
	}
	
}

