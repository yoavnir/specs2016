#ifndef SPECS2016__UTILS_ERRORREPORTING__H
#define SPECS2016__UTILS_ERRORREPORTING__H

#include <string>

class SpecsException : public std::exception {
public:
	explicit SpecsException(const char* _fn, unsigned int _ln, const char* _msg):
		fn(_fn), ln(_ln), msg(_msg) {}
	explicit SpecsException(const char* _fn, unsigned int _ln, std::string& _msg):
		fn(_fn), ln(_ln), msg(_msg) {}
	virtual const char* what(bool concise = false) const throw ();
protected:
	const char*  fn;
	std::string  msg;
	unsigned int ln;
};

#define MYTHROW(s) throw SpecsException(__FILE__, __LINE__, s);

#define MYASSERT(cond) { if (!(cond)) { \
	std::string _assert_err = std::string("Failed assertion: ") + #cond; \
	MYTHROW(_assert_err); \
	} \
	}

#define MYASSERT_WITH_MSG(cond,s) { if (!(cond)) {MYTHROW(s); } }

class ConversionException : public SpecsException {
public:
	explicit ConversionException(const char* _fn, unsigned int _ln,
			const char* _srcf, const char* _dstf, std::string& _src) : SpecsException(_fn, _ln, "") {
		msg = "Cannot convert <" + _src + "> from format <" + _srcf + "> to format <" + _dstf + ">";
	}
};

#define CONVERSION_EXCEPTION(s,sf,df) throw ConversionException(__FILE__, __LINE__, sf, df, s);
#endif
