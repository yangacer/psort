#include "rcmp.h"

int count__ = 0;

void
record_comparator::set_key_preference(
	char const **first_key, char const **last_key,
	bool* first_order, bool* last_order)
{
	while(first_key != last_key){
		keys_.push_back(*first_key);
		orders_.push_back(*first_order);
		++first_order;
		++first_key;
	}
	
}

void
record_comparator::add_key(char const *field_name, bool order)
{
	keys_.push_back(field_name);
	orders_.push_back(order);
}

void
record_comparator::flip_order(char const *field_name)
{
	unsigned int i(0);
	while(i != keys_.size()){
		if(keys_[i] == field_name){
			orders_[i] = ~orders_[i];
			break;	
		}
		++i;
	}
}

bool
record_comparator::operator()(record const &lhs, record const &rhs) const
{
	
	unsigned int i = 0;
	int cond(0);
	while( i != keys_.size() ){
		if(0 < (cond = lhs.compare(keys_[i].c_str(), rhs))) 
			return orders_[i] ? false : true;
		else if(0 > cond)
			return orders_[i] ? true : false;
		++i;
	}
	return false;
}

void
record_comparator::reset()
{
	keys_.clear();
	orders_.clear();
}

// ------------- fast_rec_cmp Impl --------------

void
fast_rec_cmp::cache(rschema const &rs)
{
	cache_.~record();
	kIdx_.clear();
	rs.make(cache_);
	for(int i=0; i< keys_.size(); ++i)
		kIdx_.push_back(rs.find(keys_[i].c_str()));	
	
}

bool
fast_rec_cmp::operator()(record const &lhs, record const &rhs) const
{
	// record::const_iterator LIter, RIter;
	int cond(0), i=0;
	
	// bool isSame = lhs.isSameSchema(cache_) && rhs.isSameSchema(cache_);
	
	while(i != kIdx_.size()){
		// LIter = lhs.const_begin() + kIdx_[i];
		// RIter = rhs.const_begin() + kIdx_[i];
		if(0 < (cond = lhs[kIdx_[i]]->compare(rhs[kIdx_[i]], true)) ) 
			return orders_[i] ? false : true;
		else if(0 > cond)
			return orders_[i] ? true : false;
		++i;		
	}
	return false;
}

