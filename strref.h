#ifndef _STRREF_H
#define _STRREF_H

#include <iosfwd>
#include <cstring>
#include "GAISUtils/record.h"

#define SIX_BOOL_OP_DECL(C,X) bool operator X (C const& rhs)const

struct str_ref
{
	str_ref();
	str_ref(char const* data, unsigned int size);
	str_ref(char const* data_beg, char const* data_end);
	
	SIX_BOOL_OP_DECL(str_ref,>);
	SIX_BOOL_OP_DECL(str_ref,<);
	SIX_BOOL_OP_DECL(str_ref,>=);
	SIX_BOOL_OP_DECL(str_ref,<=);
	SIX_BOOL_OP_DECL(str_ref,==);
	SIX_BOOL_OP_DECL(str_ref,!=);
	
	str_ref&
	assign(char const* data, unsigned int size);

	str_ref&
	assign(char const* data_beg, char const* data_end);

	char const* data_;
	unsigned int size_;
};

std::istream &operator>>(std::istream& is, str_ref const& sref);

// std::ostream &operator<<(std::ostream& os, str_ref const& sref);

template<>
class field<str_ref> : public absField, public Loki::SmallObject<>
{
private: // Client never creates field object directly
	friend class record;
	template<class T1> friend struct create_field;

	typedef str_ref value_type;

	field()
	: val_()
	{}

	field(field const &cp)
	: val_(cp.val_)
	{}

	~field()
	{}

	field* 
	Clone() const
	{ return new field(*this); }

	int 
	compare(absField const* rhs) const throw(char const*)
	{
		field* p = 
			dynamic_cast<field*>(
				const_cast<absField*>(rhs)
			);

		if(0 == p)
			return -2;
			//throw "Compare between different type";
		if(val_ < p->val_)
			return -1;
		if(val_ > p->val_)
			return 1;
		return 0;
	}
	
	bool 
	fromString(char const *cstr, size_t size)
	{
		val_.data_ = cstr;
		val_.size_ = size;
		return true;
	}

	bool 
	fromString(char const *str)
	{
		val_.data_ = str;
		val_.size_ = std::strlen(str);
		return true;
	}


	std::string 
	toString() const 
	{
		return std::string(val_.data_, val_.size_);
	}

	str_ref val_;
};
#endif
