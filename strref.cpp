#include "strref.h"
#include <cstring>
#include <algorithm>
#include <functional>
#include <ostream>

str_ref::str_ref()
: data_(0), size_(0)
{}

str_ref::str_ref(char const *data, unsigned int size)
: data_(data), size_(size)
{}

str_ref::str_ref(char const* data_beg, char const* data_end)
: data_(data_beg), size_(data_end - data_beg)
{}

str_ref&
str_ref::assign(char const* data, unsigned int size)
{	data_ = data; size_ = size;	}

str_ref&
str_ref::assign(char const* data_beg, char const* data_end)
{	data_ = data_beg; size_ = data_end - data_beg;	}


bool 
str_ref::operator>(str_ref const& rhs) const
{
	return std::lexicographical_compare(rhs.data_, rhs.data_ + rhs.size_, data_, data_ + size_);
}

bool 
str_ref::operator<(str_ref const& rhs) const
{

	return std::lexicographical_compare(data_, data_ + size_, rhs.data_, rhs.data_ + rhs.size_);
}

bool 
str_ref::operator>=(str_ref const& rhs) const
{	return !(*this<rhs);	}

bool 
str_ref::operator<=(str_ref const& rhs) const
{	return !(*this>rhs);	}

bool 
str_ref::operator==(str_ref const& rhs) const
{
	return rhs.size_ == size_ && strncmp(data_, rhs.data_, size_);	
}

bool 
str_ref::operator!=(str_ref const& rhs) const
{
	return !(*this == rhs);	
}

// -------------- comparison between std::string
bool 
str_ref::operator>(std::string const& rhs) const
{
	return std::lexicographical_compare(rhs.begin(), rhs.end(), data_, data_ + size_);
}

bool 
str_ref::operator<(std::string const& rhs) const
{

	return std::lexicographical_compare(data_, data_ + size_, rhs.begin(), rhs.end());
}

bool 
str_ref::operator>=(std::string const& rhs) const
{	return !(*this<rhs);	}

bool 
str_ref::operator<=(std::string const& rhs) const
{	return !(*this>rhs);	}

bool 
str_ref::operator==(std::string const& rhs) const
{
	return rhs.size() == size_ && strncmp(data_, rhs.c_str(), size_);	
}

bool 
str_ref::operator!=(std::string const& rhs) const
{
	return !(*this == rhs);	
}

//std::istream &operator>>(std::istream& is, str_ref const& sref)
//{ return is;}

std::ostream &operator<<(std::ostream& os, str_ref const& sref)
{
	os.write(sref.data_, sref.size_);	
	return os;
}

unsigned int
referenced_count(record const& r)
{
	record::const_iterator iter = r.const_begin();
	unsigned int size(0);
	field<str_ref> *p(0);
	while(iter != r.const_end()){
		p = dynamic_cast<field<str_ref>*>(*iter);
		if(p)
			size += p->val_.size_;
		++iter;
	}
	return size;
}

unsigned int
cp_chg_referenced(char *buf, record &r)
{
	record::iterator iter = r.begin();
	unsigned int size(0);
	field<str_ref> *p(0);
	while(iter != r.end()){
		p = dynamic_cast<field<str_ref>*>(*iter);
		if(p){
			memcpy(buf + size, p->val_.data_, p->val_.size_);
			(*iter)->fromString(buf+size, p->val_.size_);
			size += p->val_.size_;
		}
		++iter;
	}
	return size;
}
