1. Multiple Keys and Compare Methods
====
	-k field_name compare_method [-k ...]

	e.g. ./psort -k '@U:' 'STR' -k '@s:' 'UNIT'

	This configuration will let psort compare records with "@U:" 
	field in lexicalgraphical order, so call string comparison. 
	If this field of two records are equal, then psort will 
	compare "@s:" fields numerically. Notice the order of -k 
	options is the preference of keys.

	See supported Type for more information about compare 
	methods.

2. Memory Limitation and Reservation
====
	-M integer[k|m|g] -r percentage

	e.g. -M 100m -r 20

	The 100m argument indicates that the maximum memroy usage
	is 100 MB. The -r option let psort preserve 20% of 100m
	in the first phase, to reduce probability of repartition.

3. Supported Types

	UNIT 	- unsigned integer
	INT	- integer
	LLONG	- long long integer
	DOUBLE  - double float
	STR 	- string
	STRREF*	- string reference for eliminating copy

	*Internal type supported by psort but not GAISUtils::record

4. Pivot File

	After partitioning input data, a pivot file will be 
	genterated. Assume given command is

	./psort -k '@U:' 'STR' -k '@s:' 'UNIT'

	Format of pivot recored is as follows:

	@
	@U:string value
	@s:integer value
	@_LTpart:less than partition file
	@_GEpart:greater-or-equal=to partition file


	e.g. Let pivot file contains two pivot records such as:

	# pivot record #0
	@
	@U:abc
	@s:123
	@_LTpart:000.part
	@_GEpart:001.part
	# pivot record #1
	@
	@U:bcc
	@s:123
	@_LTpart:001.part
	@_GEpart:002.part

	Records partitioned to 000.part are less than pivot
	record #0. To records contained by 001.part, they are 
	greater or equal to pivot record #0.

