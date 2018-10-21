#ifndef SPECS2016__UTILS_ERRORREPORTING__H
#define SPECS2016__UTILS_ERRORREPORTING__H

#include <string>

class SpecsException : public std::exception {
public:
	explicit SpecsException(const char* _fn, unsigned int _ln, const char* _msg):
		fn(_fn), ln(_ln), msg(_msg) {}
	explicit SpecsException(const char* _fn, unsigned int _ln, std::string& _msg):
		fn(_fn), ln(_ln), msg(_msg) {}
	virtual const char* what() const throw ();
protected:
	const char*  fn;
	std::string  msg;
	unsigned int ln;
};

#define MYTHROW(s) SpecsException(__FILE__, __LINE__, s)

#endif
