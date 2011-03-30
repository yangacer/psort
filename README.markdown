1. Multiple Keys and Compare Methods
====
	-k field_name field_type compare_method [-k ...]

	e.g. ./psort -k '@U:' 'STR' '>' -k '@s:' 'UNIT' '<'

This configuration will let psort compare records' "@U:" 
fields by string comparison in decending order. 
If the field of two records are equal, then psort will 
compare "@s:" fields by numerical comparison in accending 
order.

__Notice__ the order of -k options is the preference of keys.

See section - Supported Type - for more information about 
field type.

2. Memory Limitation and Reservation
====
	-M integer[k|m|g] -r percentage

	e.g. -M 100m -r 20

The 100m argument indicates that the maximum memroy usage
is 100 MB. The -r option let psort preserve 20% of 100m
in the first phase, to reduce probability of repartition.

3. Supported Types
====
	UNIT 	- unsigned integer
	INT	- integer
	LLONG	- long long integer
	DOUBLE  - double float
	STR 	- string
	STRREF*	- string reference for eliminating copy

*Internal type supported by psort but not GAISUtils::record.

__Notice__ the STR type introduces greater data movement due
to copy semantic maintainence. In case of psort, __STRREF__ is 
recommanded.

4. Output - Pivot File and Part File
====
After partitioning input data, a pivot file - pivot.file
will be genterated. Assume given command is

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
	# pivot record #1
	@
	@U:bcc
	@s:123

This pivot file corresponds to three .part files - 
0.part, 1.part, and 2.part.

Records partitioned to 0.part are less than pivot
record #0. To records contained by 1.part, they are 
greater or equal to pivot record #0. Vice versa, 
record in 2.part are greater than pivot record #1.

5. Input
====
	./psort [-f file]
If [-f file] given, then psort read input data from file.
(*)Otherwise, it read input data from standard input.

(*)__Current version does not support stdin source__

6. Record Begin Pattern
====
	./psort -b '@\n@GAIS_Rec:\n'
Argument follows -b option will be used as record begin
pattern. psort translates "\n" string to '\n' character
automatically.

