#ifndef _STRREF_H
#define _STRREF_H

#include <iosfwd>

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

std::ostream &operator<<(std::ostream& os, str_ref const& sref);

#endif
