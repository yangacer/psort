#ifndef _RCMP_H_
#define _RCMP_H_

#include "GAISUtils/record.h"
#include <vector>
#include <string>
#include <functional>

// extern int rcmp_cp_cnt;

template<class F>
struct FunctorWrapper : public std::binary_function<
	typename F::first_argument_type,
	typename F::second_argument_type,
	typename F::result_type >
{
	FunctorWrapper(F& functor):f_(&functor){}

	typename F::result_type 
	operator()(typename F::first_argument_type x, typename F::second_argument_type y) const
	{ return (*f_)(x,y); }

	F const* f_;	
};

struct record_comparator : public std::binary_function<record const&, record const&, bool>
{
	/*
	record_comparator():keys_(), orders_(){}	
	record_comparator(record_comparator const &cp)
	:keys_(cp.keys_), orders_(cp.orders_)
	{
		rcmp_cp_cnt++;
		//fprintf(stderr, "rcmp cp cnt:%d\n", rcmp_cp_cnt);
	}
	*/

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
