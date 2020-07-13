#ifndef SPECS2016__UTILS__ALU_FUNCTIONS_H
#define SPECS2016__UTILS__ALU_FUNCTIONS_H

#include <unordered_map>
#include <memory>
#include "utils/SpecString.h"
#include "utils/alu.h"

#define ALUFUNC_REGULAR      0x00
#define ALUFUNC_STATISTICAL  0x01
#define ALUFUNC_FREQUENCY    0x02
#define ALUFUNC_EXTERNAL     0x80

// function name, number of arguments, whether it needs lines from input
#define ALU_FUNCTION_LIST            \
	H(Math Functions,9) \
	X(abs,            1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns the absolute value of x","Will return an int or a float depending on the type of the argument.") \
	X(pow,            2, ALUFUNC_REGULAR,     false,  \
			"(x,y) - Returns x to the power of y","Will return an int if both arguments are whole numbers, or a float otherwise.\nThe return value will be int even if the arguments were calculated as floats.") \
	X(sqrt,           1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns the square root of x","Always returns a float.") \
	H(Recasting Functions,16) \
	X(c2u,            1, ALUFUNC_REGULAR,     false,  \
			"(s) - Returns the value of the string s re-cast as an unsigned integer.","A length of 1-8 characters is supported.\nA longer length will cause a runtime error.") \
	X(c2f,            1, ALUFUNC_REGULAR,     false,  \
			"(s) - Returns the value of the string s re-cast as a floating point number","The supported lengths are those of the float, double, and long double types.\nAny other length will cause a runtime error.") \
	X(c2d,            1, ALUFUNC_REGULAR,     false,  \
			"(s) - Returns the value of the string s re-cast as a signed integer.","A length of 1-8 characters is supported.\nA longer length will cause a runtime error.") \
	X(frombin,        1, ALUFUNC_REGULAR,     false,  \
			"(op) - Re-casts op as a 64-bit number.","A length of 1-8 characters is supported.\nA longer length will cause a runtime error.") \
	X(tobine,         2, ALUFUNC_REGULAR,     false,  \
			"(op,bits) - Returns a 'bits'-bit binary representation of 'op'.","The unsigned integer in op is converted.\nThe output field has length of bits bits.\nValid values for bits are 8, 16, 32, and 64.") \
	X(tobin,          1, ALUFUNC_REGULAR,     false,  \
			"(op) - Returns a binary representation of the unsigned integer in 'op'.","The field length is automatically determined by the value of x,\nbut will be 1, 2, 4, or 8 characters in length.") \
	X(string,         1, ALUFUNC_REGULAR,     false,  \
			"(x) - Converts x to a string.","") \
	H(State Functions,11) \
	X(first,          0, ALUFUNC_REGULAR,     false,  \
			"() - Returns TRUE (1) if this is the first line.","") \
	X(recno,          0, ALUFUNC_REGULAR,      true,  \
			"() - Returns the record number of the current record.","Increments with every READ or READSTOP.") \
	X(number,         0, ALUFUNC_REGULAR,      true,  \
			"() - Returns the number of times this specification has restarted","Does not increment with READ or READSTOP. Otherwise similar to recno().") \
	X(eof,            0, ALUFUNC_REGULAR,     false,  \
			"() - Returns TRUE (1) if this is the run-out phase.","") \
	X(break,          1, ALUFUNC_REGULAR,     false,  \
			"(fid) - Returns TRUE (1) if the break for field-identifier 'fid' is established, or FALSE (0) otherwise.","") \
	H(Record Functions,16) \
	X(record,         0, ALUFUNC_REGULAR,      true,  \
			"() - Returns the entire record.","Equivalent to the @@ pseudo-variable.") \
	X(length,         1, ALUFUNC_REGULAR,     false,  \
			"(s) - Returns the length of the string s","") \
	X(wordcount,      0, ALUFUNC_REGULAR,      true,  \
			"() - Returns the number of words in the current record.","Result depends on the current word separator.") \
	X(wordstart,      1, ALUFUNC_REGULAR,      true,  \
			"(i) - Returns the position in the current record where word i begins.","") \
	X(wordlen,        1, ALUFUNC_REGULAR,      true,  \
			"(i) - Returns the length of word i in the current record.","") \
	X(wordend,        1, ALUFUNC_REGULAR,      true,  \
			"(i) - Returns the position in the current record where word i ends.","") \
	X(word,           1, ALUFUNC_REGULAR,      true,  \
			"(i) - Returns the i-th word in the current record.","") \
	X(wordrange,      2, ALUFUNC_REGULAR,      true,  \
			"(i,j) - Returns the substring of the current record from the start of the i-th word to the end of the j-th word.","") \
	X(fieldcount,     0, ALUFUNC_REGULAR,      true,  \
			"() - Returns the number of fields in the current record.","Result depends on the current field separator.") \
	X(fieldindex,     1, ALUFUNC_REGULAR,      true,  \
			"(i) - Returns the position in the current record where field i begins.","") \
	X(fieldlength,    1, ALUFUNC_REGULAR,      true,  \
			"(i) - Returns the length of field i in the current record.","") \
	X(fieldend,       1, ALUFUNC_REGULAR,      true,  \
			"(i) - Returns the position in the current record where field i ends.","") \
	X(field,          1, ALUFUNC_REGULAR,      true,  \
			"(i) - Returns the i-th field in the current record.","") \
	X(fieldrange,     2, ALUFUNC_REGULAR,      true,  \
			"(i,j) - Returns the substring of the current record from the start of the i-th field to the end of the j-th field.","") \
	X(range,          2, ALUFUNC_REGULAR,      true,  \
			"(i,j) - Returns the substring of the current record from position i to position j (inclusive).","") \
	H(Time Functions,32) \
	X(tf2mcs,         2, ALUFUNC_REGULAR,     false,  \
			"(timeString,formatString) - Parses timeString as if it's formatted as indicated by formatString and returns the time value.","Time value is specified in microseconds since the Unix epoch.\nThe time format uses the conventions of strftime, plus %xf for fractional seconds.") \
	X(mcs2tf,         2, ALUFUNC_REGULAR,     false,  \
			"(timeValue,formatString) - Formats timeValue using the format in formatString and returns the result.","Time value is specified in microseconds since the Unix epoch.\nThe time format uses the conventions of strftime, plus %xf for fractional seconds.") \
	X(tf2s,           2, ALUFUNC_REGULAR,     false,  \
			"(timeString,formatString) - Parses timeString as if it's formatted as indicated by formatString and returns the time value.","Time value is specified in seconds since the Unix epoch.\nThe time format uses the conventions of strftime, plus %xf for fractional seconds.") \
	X(s2tf,           2, ALUFUNC_REGULAR,     false,  \
			"(timeValue,formatString) - Formats timeValue using the format in formatString and returns the result.","Time value is specified in seconds since the Unix epoch.\nThe time format uses the conventions of strftime, plus %xf for fractional seconds.") \
	H(String Functions,40) \
	X(substr,         3, ALUFUNC_REGULAR,     false,  \
			"([str],[start],[length]) - Returns the substring of 'str' with length 'length' and starting from position 'start'.","If elided, 'str' defaults to the current record.\nIf elided, 'start' defaults to position 1. A negative value counts from the end of 'str'\nIf elided, 'length' defaults to the length of the string. The length can overflow and the result would be truncated at the end of 'str'.") \
	X(pos,            2, ALUFUNC_REGULAR,     false,  \
			"(needle,[haystack]) - Returns the position of the first occurrence of 'needle' within 'haystack'","If elided, 'haystack' defaults to the current record.") \
	X(lastpos,        2, ALUFUNC_REGULAR,     false,  \
			"(needle,[haystack]) - Returns the position of the last occurrence of 'needle' within 'haystack'","If elided, 'haystack' defaults to the current record.") \
	X(includes,       5, ALUFUNC_REGULAR,     false,  \
			"([haystack],needle1...) - Returns TRUE (1) if 'haystack' includes any of the needles, or FALSE (0) otherwise.","If elided, 'haystack' defaults to the current record.\nOne needle is required, but up to four may be specified.") \
	X(includesall,    5, ALUFUNC_REGULAR,     false,  \
			"([haystack],needle1...) - Returns TRUE (1) if 'haystack' includes all of the needles, or FALSE (0) otherwise.","If elided, 'haystack' defaults to the current record.\nOne needle is required, but up to four may be specified.") \
	X(rmatch,         3, ALUFUNC_REGULAR,     false,  \
			"([haystack],exp,[mFlags]) - Returns TRUE (1) if the regular expression 'exp' matches 'haystack', or FALSE (0) otherwise.","If elided, 'haystack' defaults to the current record.\nSee manual for more information about regular expressions, including setting the syntax with\n--regexType and the effects of the matchFlags.") \
	X(rsearch,        3, ALUFUNC_REGULAR,     false,  \
			"([haystack],exp,[mFlags]) - Returns TRUE (1) if the regular expression 'exp' matches some substring of 'haystack', or FALSE (0) otherwise.","If elided, 'haystack' defaults to the current record.\nSee manual for more information about regular expressions, including setting the syntax with\n--regexType and the effects of the matchFlags.") \
	X(rreplace,       4, ALUFUNC_REGULAR,     false,  \
			"([haystack],exp,fmt,[mFlags]) - Returns the string 'haystack', with all matches of regular expression 'exp' replaced by 'fmt'.","If elided, 'haystack' defaults to the current record.\nSee manual for more information about regular expressions, including setting the syntax with\n--regexType and the effects of the matchFlags.") \
	X(left,           2, ALUFUNC_REGULAR,     false,  \
			"([str],length) - Returns the substring of 'str' with length 'length' that starts at the beginning.","If elided, 'str' defaults to the current record.\nThe length can overflow and the result would just be all of 'str'.") \
	X(right,          2, ALUFUNC_REGULAR,     false,  \
			"([str],length) - Returns the substring of 'str' with length 'length' that ends at the end of 'str'.","If elided, 'str' defaults to the current record.\nThe length can overflow and the result would just be all of 'str'.") \
	X(center,         2, ALUFUNC_REGULAR,     false,  \
			"([str],length) - Returns the substring of 'str' with length 'length' centered within 'str'.","If elided, 'str' defaults to the current record.\nThe length can overflow and the result would just be all of 'str'.") \
	X(centre,         2, ALUFUNC_REGULAR,     false,  \
			"([str],length) - Returns the substring of 'str' with length 'length' centred within 'str'.","If elided, 'str' defaults to the current record.\nThe length can overflow and the result would just be all of 'str'.") \
	X(x2d,            2, ALUFUNC_REGULAR,     false,  \
			"(hex,[length]) - Returns the decimal value of the hex string in 'hex'.","If 'length' is missing or non-positive, the result is the unsigned value of the entire 'hex' string.\nOtherwise, the first 'length' characters are considered (up to 16), and the result is signed based on the first hex digit.") \
	X(d2x,            1, ALUFUNC_REGULAR,     false,  \
			"(dec) - Returns the hex value of the number in 'dec'.","") \
	X(x2ch,           1, ALUFUNC_REGULAR,     false,  \
			"(hex) - Returns a string with each character the ASCII encoding of two hex digits from 'hex'.","") \
	X(c2x,            1, ALUFUNC_REGULAR,     false,  \
			"(str) - Retuns a hex string with the ASCII value for each character in 'str'.","") \
	X(ucase,          1, ALUFUNC_REGULAR,     false,  \
			"(str) - Returns the string in 'str' converted to uppercase.","") \
	X(lcase,          1, ALUFUNC_REGULAR,     false,  \
			"(str) - Returns the string in 'str' converted to lowercase.","") \
	X(bswap,          1, ALUFUNC_REGULAR,     false,  \
			"(str) - Returns a byte-swapped copy of 'str'.","") \
	X(substitute,     4, ALUFUNC_REGULAR,     false,  \
			"(haystack,needle,subst,[max]) - Returns 'haystack' where at most 'max' occurrences of 'needle' have been replaced by 'subst'","If 'max' is omitted, only 1 occurrence is replaced.\nIf 'max' is 'U', all occurrences are replaced.") \
	X(sfield,         3, ALUFUNC_REGULAR,     false,  \
			"(str,n,[sep]) - Returns the n-th field of 'str' if the field separator is 'sep'.","'sep' defaults to a tab.") \
	X(lvalue,         2, ALUFUNC_REGULAR,     false,  \
			"(str,[sep]) - Return the left hand part of 'str' separated by 'sep'.","'sep' defaults to an equals sign.") \
	X(rvalue,         2, ALUFUNC_REGULAR,     false,  \
			"(str,[sep]) - Return the right hand part of 'str' separated by 'sep'.","'sep' defaults to an equals sign.") \
	X(sword,          3, ALUFUNC_REGULAR,     false,  \
			"(str,n,[sep]) - Returns the n-th word of 'str' if the word separator is 'sep'.","'sep' defaults to a tab.") \
	X(abbrev,         3, ALUFUNC_REGULAR,     false,  \
			"(str,s,[len]) - Returns TRUE (1) if 's' is a prefix of 'str', or FALSE (0) otherwise.","If 'len' is specified, only the first 'len' characters of 's' are considered.") \
	X(compare,        3, ALUFUNC_REGULAR,     false,  \
			"(s1,s2,[pad]) - Returns the index of the first mis-matched charcter between s1 and s2.","If the strings are not equal in length, the 'pad' character is used to pad the shorter one.\nIf the 'pad' character is not specified, it defaults to a space.") \
	X(copies,         2, ALUFUNC_REGULAR,     false,  \
			"(s,n) - Returns the string 's' duplicated n times.","") \
	X(delstr,         3, ALUFUNC_REGULAR,     false,  \
			"(str,start,[length]) - Returns the string 'str' with the substring defined by 'start' and 'length' deleted.","If 'length' is unspecified or zero, the rest of the string is deleted.") \
	X(delword,        3, ALUFUNC_REGULAR,     false,  \
			"(str,start,length) - Returns the string 'str' with the word range defined by 'start' and 'length' deleted.","If 'length' is unspecified or zero, the rest of the string is deleted.") \
	X(find,           2, ALUFUNC_REGULAR,     false,  \
			"(str,phrase) - Returns the word number of the first occurrence of 'phrase' in 'str'.","Returns 0 if phrase is not found.\nMultiple blanks between words are treated as one in comparisons.") \
	X(index,          3, ALUFUNC_REGULAR,     false,  \
			"(haystack,needle,[start]) - Returns the character position of needle within string haystack.","Returns 0 if needle is not found.\nIf positive, start tells where in haystack to initiate the search. It defaults to 1 if not specified.\nThe standard pos function should be used instead of index if possible.") \
	X(insert,         5, ALUFUNC_REGULAR,     false,  \
			"(str,target,pos,len,pad) - Inserts 'str' into 'target' at position 'pos' and truncated or padded with 'pad' characters to length 'len'.","With default or zero values, 'pos' inserts 'str' at the start of target, and the length of the string is kept as is.\nThe pad defaults to a space.") \
	X(justify,        3, ALUFUNC_REGULAR,     false,  \
			"(str,len,pad) - Evenly justifies words within 'str'.","The length argument 'len' specifies the length of the returned string,\nwhile 'pad' specifies what padding (by default a space) to insert if necessary.") \
	X(overlay,        5, ALUFUNC_REGULAR,     false,  \
			"(str1,str2,start,length,pad) - Returns a copy of 'str2', partially or fully overwritten by 'str1'.","'start' specifies the starting position of the overlay.\n'length' truncates or pads 'str1' prior to the operation, using 'pad' as the padding character.") \
	X(reverse,        1, ALUFUNC_REGULAR,     false,  \
			"(str) - Returns a copy of the string 'str' with its characters reversed.","") \
	H(Statistical Functions,29) \
	X(sum,            1, ALUFUNC_STATISTICAL, false,  \
			"(fid) - Returns the sum of all values of field identifier 'fid' that have been seen so far.","Only provides information relevant to the entire data set during the run-out cycle.") \
	X(min,            1, ALUFUNC_STATISTICAL, false,  \
			"(fid) - Returns the minimum of all values of field identifier 'fid' that have been seen so far.","Only provides information relevant to the entire data set during the run-out cycle.") \
	X(max,            1, ALUFUNC_STATISTICAL, false,  \
			"(fid) - Returns the maximum of all values of field identifier 'fid' that have been seen so far.","Only provides information relevant to the entire data set during the run-out cycle.") \
	X(average,        1, ALUFUNC_STATISTICAL, false,  \
			"(fid) - Returns the numerical average of all values of field identifier 'fid' that have been seen so far.","'fid' is used as a random variable.\nOnly provides information relevant to the entire data set during the run-out cycle.") \
	X(variance,       1, ALUFUNC_STATISTICAL, false,  \
			"(fid) - Returns the variance of all values of field identifier 'fid' that have been seen so far.","'fid' is used as a random variable.\nOnly provides information relevant to the entire data set during the run-out cycle.") \
	X(stddev,         1, ALUFUNC_STATISTICAL, false,  \
			"(fid) - Returns the standard deviation of all values of field identifier 'fid' that have been seen so far.","'fid' is used as a random variable.\nOnly provides information relevant to the entire data set during the run-out cycle.") \
	X(stderrmean,     1, ALUFUNC_STATISTICAL, false,  \
			"(fid) - Returns the standard error of all values of field identifier 'fid' that have been seen so far.","'fid' is used as a random variable.\nOnly provides information relevant to the entire data set during the run-out cycle.") \
	X(present,        1, ALUFUNC_REGULAR,     false,  \
			"(fid) - Returns TRUE (1) if the field identifier 'fid' is set, or FALSE (0) otherwise.","All field identifiers are reset at every run of the specification. This function will\nreturn FALSE until the field identifier has been set within this run.") \
	X(fmap_nelem,     1, ALUFUNC_FREQUENCY,   false,  \
			"(fid) - Returns the number of discrete values of field identifier 'fid'.","Only provides information relevant to the entire data set during the run-out cycle.") \
	X(fmap_nsamples,  1, ALUFUNC_FREQUENCY,   false,  \
			"(fid) - Returns the number of samples taken of field identifier 'fid'.","Only provides information relevant to the entire data set during the run-out cycle.") \
	X(fmap_count,     2, ALUFUNC_FREQUENCY,   false,  \
			"(fid,elem) - Returns the number of occurences of 'elem' in field identifier 'fid'.","Only provides information relevant to the entire data set during the run-out cycle.") \
	X(fmap_frac,      2, ALUFUNC_FREQUENCY,   false,  \
			"(fid,elem) - Returns the fraction of values in field identifier 'fid' that are equal to 'elem'.","Only provides information relevant to the entire data set during the run-out cycle.") \
	X(fmap_pct,       2, ALUFUNC_FREQUENCY,   false,  \
			"(fid,elem) - Returns the percentage of values in field identifier 'fid' that are equal to 'elem'.","Only provides information relevant to the entire data set during the run-out cycle.") \
	X(fmap_common,    1, ALUFUNC_FREQUENCY,   false,  \
			"(fid) - Returns the most common value of field identifier 'fid'.","Only provides information relevant to the entire data set during the run-out cycle.") \
	X(fmap_rare,      1, ALUFUNC_FREQUENCY,   false,  \
			"(fid) - Returns the least common value of field identifier 'fid'.","Only provides information relevant to the entire data set during the run-out cycle.") \
	X(fmap_sample,    2, ALUFUNC_FREQUENCY,   false,  \
			"(fid,elem) - Notes an occurence of the value in 'elem' for field identifier 'fid', and returns the number of occurences so far.","This is the only one of the fmap_* functions that modifies the frequency map.\nIt also affects the other statistics functions.") \
	X(fmap_dump,      4, ALUFUNC_FREQUENCY,   false,  \
			"(fid,fmt,order,pct) - Returns a multi-line string with the frequency map of field identifier 'fid'.","Only provides information relevant to the entire data set during the run-out cycle.\nFormat can be 'txt' or '0' for a textual table; 'lin' for a table with lines, and 'csv' or 'json' for those formats.\nOrder is 's'/'sa' to sort by ascending value, or 'sd' for descending, 'c'/'ca' for sorting by ascending count, or 'cd' for descending.\n'pct' adds a percentage column if true.") \
	H(Advanced Math Functions,14) \
	X(rand,           1, ALUFUNC_REGULAR,     false,  \
			"([limit]) - Returns a random integer up to (but not including) 'limit'.","If 'limit' is omitted, returns a floating point number between 0 and 1.") \
	X(floor,          1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns the largest integer smaller than 'x'.","") \
	X(round,          2, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns the closest integer to 'x'.","") \
	X(ceil,           1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns the smallest integer larger than 'x'.","") \
	X(sin,            1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns sin(x) where 'x' is in radians.","") \
	X(cos,            1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns cos(x) where 'x' is in radians.","") \
	X(tan,            1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns tan(x) where 'x' is in radians.","") \
	X(arcsin,         1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns arcsin(x) where the result is in radians.","") \
	X(arccos,         1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns arccos(x) where the result is in radians.","") \
	X(arctan,         1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns arctan(x) where the result is in radians.","") \
	X(dsin,           1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns sin(x) where 'x' is in degrees.","") \
	X(dcos,           1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns cons(x) where 'x' is in degrees.","") \
	X(dtan,           1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns tan(x) where 'x' is in degrees.","") \
	X(arcdsin,        1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns arcsin(x) where the result is in degrees.","") \
	X(arcdcos,        1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns arccos(x) where the result is in degrees.","") \
	X(arcdtan,        1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns arctan(x) where the result is in degrees.","") \
	X(exp,            1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns exponent function e^x.","") \
	X(log,            2, ALUFUNC_REGULAR,     false,  \
			"(x,[base]) - Returns logarithm of 'x'.","If base is omitted, retuns the natural logarithm of 'x'.") \
	H(Other Functions,34) \
	X(bitand,         2, ALUFUNC_REGULAR,     false,  \
			"(s1,s2) - Returns a bit-wise AND of the two strings s1 and s2.","If the strings are not equal in length, the result has the length of the shorter one.\nIf an operand is not a string, it is converted to a decimal string representation.") \
	X(bitor,          2, ALUFUNC_REGULAR,     false,  \
			"(s1,s2) - Returns a bit-wise OR of the two strings s1 and s2.","If the strings are not equal in length, the result has the length of the shorter one.\nIf an operand is not a string, it is converted to a decimal string representation.") \
	X(bitxor,         2, ALUFUNC_REGULAR,     false,  \
			"(s1,s2) - Returns a bit-wise XOR of the two strings s1 and s2.","If the strings are not equal in length, the result has the length of the shorter one.\nIf an operand is not a string, it is converted to a decimal string representation.") \
	X(sign,           1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns -1/0/1 for negative/zero/positive x.","") \
	X(space,          3, ALUFUNC_REGULAR,     false,  \
			"(str,[len],[pad]) - Formats 'str' by replacing internal blanks with 'len' occurrences of the 'pad' character.","'len' defaults to 1. 'pad' defaults to a space.") \
	X(strip,          3, ALUFUNC_REGULAR,     false,  \
			"(str,[option],[char]) - Returns 'str' stripped on leading and/or trailing blanks, or whatever 'char' is specified.","'option' can be L/T/B for stripping leading blanks, trailing blanks, or both.\n'char' defaults to a space character; 'option' defaults to Both.") \
	X(subword,        3, ALUFUNC_REGULAR,     false,  \
			"(str,start,[len]) - Returns the substring of 'str' that begins at blank-delimited word 'start' for 'len' words.","If length is omitted, it defaults to the remainder of the string.") \
	X(translate,      4, ALUFUNC_REGULAR,     false,  \
			"(str,[tout],[tin],[pad]) - Returns a translated copy of 'str'.","Characters are translated according to the input translation table 'tin' and its output\nequivalent, 'tout'. If both 'tin' and 'tout' are not coded, all characters in 'str'\nare translated to uppercase. If 'tout' is shorter than 'tin', it is padded with\nthe 'pad' character or its default, blanks.") \
	X(verify,         4, ALUFUNC_REGULAR,     false,  \
			"(str,ref,[option],[start]) - Verifies that all characters in the string 'str' are also in the reference string 'ref'.","Returns the position of the first character in 'str' that is not in the reference string, or 0 if all the characters are in the reference string.\n'option' can be 'N' (the default) if all characters should match, or 'M' if all characters should not match.\n'start' allows you to skip some characters of 'str' and defaults to 1.") \
	X(wordindex,      2, ALUFUNC_REGULAR,     false,  \
			"(str,n) - Returns the starting position of the n-th blank-delimited word in 'str'.","") \
	X(wordlength,     2, ALUFUNC_REGULAR,     false,  \
			"(str,n) - Returns the length of the n-th blank-delimited word in 'str'.","") \
	X(wordpos,        3, ALUFUNC_REGULAR,     false,  \
			"(phrase,str,[start]) - If 'phrase' is a substring of 'str', returns the word number position at which it begins. Otherwise returns 0.","'start' defaults to 1 and is the index of the word where the searching starts.") \
	X(words,          1, ALUFUNC_REGULAR,     false,  \
			"(str) - Returns the number of blank-delimited words in 'str'.","") \
	X(xrange,         2, ALUFUNC_REGULAR,     false,  \
			"(starts,end) - Returns a string composed of all the characters between 'start' and 'end' inclusive.","'start' defaults to 0x00, and 'end' defaults to 0xff.") \
	X(fmt,            5, ALUFUNC_REGULAR,     false,  \
			"(val,fmt,digits,dec,sep) - Returns a string representation of the floating point number 'val'.","The 'fmt' argument can be omitted, or it can begin with 'f' for a fixed number of digits after the\ndecimal point, or 's' for scientific notation. When omitted, the 'digits' argument sets the total\nnumber of digits displayed. The 'dec' argument sets the character used for the decimal point (defaul\nis a period), while the 'sep' argument sets the character used as thousands separator (default is none).") \
	X(next,           0, ALUFUNC_REGULAR,     false,  \
			"() - Returns the column of the next character to print if a spec unit specifies the NEXT position.","") \
	X(rest,           0, ALUFUNC_REGULAR,     false,  \
			"() - Returns the number of columns from the current position to the end of the line.","") \
	X(countocc,       2, ALUFUNC_REGULAR,     true,   \
			"(needle,[haystack]) - Returns the number of times since the start of this run that this particular needle has been found in haystacks.", "The counter is incremented if the needle is found in 'haystack'.\nIf 'haystack' is omitted, it defaults to the input line.") \
	X(countocc_get,   1, ALUFUNC_REGULAR,     false,  \
			"(needle) - Returns the number of times since the start of this run that this particular needle has been found in haystacks.", "") \
	X(countocc_dump,  3, ALUFUNC_REGULAR,     false,  \
			"(fmt,sOrder,showPct) - Returns a multi-line string with the dump of occurrences found through 'countocc'.","Only provides information relevant to the entire data set during the run-out cycle.\nFormat can be 'txt' or '0' for a textual table; 'lin' for a table with lines, and 'csv' or 'json' for those formats.\nOrder is 's'/'sa' to sort by ascending value, or 'sd' for descending, 'c'/'ca' for sorting by ascending count, or 'cd' for descending.\n'pct' adds a percentage column if true.") \
	H(Misc Functions,20) \
	X(conf,           2, ALUFUNC_REGULAR,     false,  \
			"(key,[default]) - Returns the configuration string for 'key'.","If the string is not defined, returns the default value.\nIf that is not defined, returns NaN.") \
	X(defined,        1, ALUFUNC_REGULAR,     false,  \
			"(s) - Returns TRUE (1) if the string 's' is a configured string, or FALSE (0) otherwise.","")

#define ALU_DEBUG_FUNCTION_LIST                       \
	X(testfunc,       4, ALUFUNC_REGULAR,     false,  \
			"","")

#define ALU_PSEUDO_FUNCTION_LIST     \
	X(break)                         \
	X(sum)                           \
	X(min)                           \
	X(max)                           \
	X(average)                       \
	X(variance)                      \
	X(stddev)                        \
	X(stderrmean)                    \
	X(present)                       \
	X(fmap_nelem)                    \
	X(fmap_nsamples)                 \
	X(fmap_count)                    \
	X(fmap_frac)                     \
	X(fmap_pct)                      \
	X(fmap_common)                   \
	X(fmap_rare)                     \
	X(fmap_sample)                   \
	X(fmap_dump)                     \

#define ALUFUNC0(nm)	PValue AluFunc_##nm();
#define ALUFUNC1(nm)	PValue AluFunc_##nm(PValue);
#define ALUFUNC2(nm)	PValue AluFunc_##nm(PValue, PValue);
#define ALUFUNC3(nm)	PValue AluFunc_##nm(PValue, PValue, PValue);
#define ALUFUNC4(nm)	PValue AluFunc_##nm(PValue, PValue, PValue, PValue);
#define ALUFUNC5(nm)	PValue AluFunc_##nm(PValue, PValue, PValue, PValue, PValue);

#define X(fn,argc,flags,rl,shorthelp,longhelp) ALUFUNC##argc(fn)
#define H(hdr,len)
ALU_FUNCTION_LIST
#ifdef DEBUG
ALU_DEBUG_FUNCTION_LIST
#endif
#undef X
#undef H

typedef PValue (*AluFunc0)();
typedef PValue (*AluFunc1)(PValue op1);
typedef PValue (*AluFunc2)(PValue op1, PValue op2);
typedef PValue (*AluFunc3)(PValue op1, PValue op2, PValue op3);
typedef PValue (*AluFunc4)(PValue op1, PValue op2, PValue op3, PValue op4);
typedef PValue (*AluFunc5)(PValue op1, PValue op2, PValue op3, PValue op4, PValue op5);

#define MAX_FUNC_OPERANDS 5

enum fmap_format {
	/* Leave a gap because low numbers are the string width */
	fmap_format__textualJustified,
	fmap_format__textualJustifiedLines = 1024,
	fmap_format__csv,
	fmap_format__json,
};

enum fmap_sortOrder {
	fmap_sortOrder__byStringAscending,
	fmap_sortOrder__byStringDescending,
	fmap_sortOrder__byCountAscending,
	fmap_sortOrder__byCountDescending,
};

typedef std::unordered_map<std::string, ALUInt> freqMapImpl;

typedef std::pair<const std::string, ALUInt> freqMapPair;

class frequencyMap {
public:
	void             note(std::string& s);
	ALUInt           nelem()     { return map.size(); }
	ALUInt           operator[](std::string& s) {return map[s];}
	ALUInt           count()     { return counter; }
	std::string      mostCommon();
	std::string      leastCommon();
	std::string      dump(fmap_format f, fmap_sortOrder o, bool includePercentage);
private:
	freqMapImpl      map;
	ALUInt           counter;
};
typedef std::shared_ptr<frequencyMap> PFrequencyMap;

class stateQueryAgent {
public:
	virtual unsigned int getWordCount() = 0;
	virtual unsigned int getFieldCount() = 0;
	virtual int     getWordStart(int idx) = 0;
	virtual int     getWordEnd(int idx) = 0;
	virtual int     getFieldStart(int idx) = 0;
	virtual int     getFieldEnd(int idx) = 0;
	virtual PSpecString getFromTo(int from, int to) = 0;
	// virtual int     getWordStart(ALUInt idx) { return getWordStart(int(idx)); }
	// virtual int     getWordEnd(ALUInt idx) { return getWordEnd(int(idx)); }
	// virtual int     getFieldStart(ALUInt idx) { return getFieldStart(int(idx)); }
	// virtual int     getFieldEnd(ALUInt idx) { return getFieldEnd(int(idx)); }
	// virtual PSpecString getFromTo(ALUInt from, ALUInt to) { return getFromTo(int(from), int(to)); }
	virtual PSpecString currRecord() = 0;
	virtual bool    isRunIn() = 0;
	virtual bool    isRunOut() = 0;
	virtual ALUInt  getRecordCount() = 0;
	virtual ALUInt  getIterationCount() = 0;
	virtual bool    breakEstablished(char id) = 0;
	virtual PAluValueStats valueStatistics(char id) = 0;
	virtual PFrequencyMap  getFrequencyMap(char id) = 0;
	virtual bool    fieldIdentifierIsSet(char id) = 0;
};

class positionGetter {
public:
	virtual size_t pos() = 0;
};

void setStateQueryAgent(stateQueryAgent* qa);
void setPositionGetter(positionGetter* pGetter);

void aluFunc_help_builtin();
bool aluFunc_help_one_builtin(std::string& funcName);

#endif
