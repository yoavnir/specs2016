#ifndef SPECS2016__UTILS_ERRORREPORTING__H
#define SPECS2016__UTILS_ERRORREPORTING__H

#include <string>

class SpecsException : public std::exception {
public:
	explicit SpecsException(const char* _fn, unsigned int _ln, const char* _msg, bool _abend = false):
		fn(_fn), msg(_msg), ln(_ln), bIsAbend(_abend) {}
	explicit SpecsException(const char* _fn, unsigned int _ln, std::string& _msg, bool _abend = false):
		fn(_fn), msg(_msg), ln(_ln), bIsAbend(_abend) {}
	using std::exception::what;  // avoid warning about overloading
	virtual const char* what(bool concise = false) const noexcept;
	virtual const bool  isAbend() const noexcept;
protected:
	const char*  fn;
	std::string  msg;
	unsigned int ln;
	bool         bIsAbend;
};

SpecsException _mythrow(const char* _fn, unsigned int _ln, const char* _msg, bool _abend);
SpecsException _mythrow(const char* _fn, unsigned int _ln, std::string& _msg, bool _abend);

#define MYTHROW(s) throw _mythrow(__FILE__, __LINE__, s, false);
	

#define MYABEND(s) throw _mythrow(__FILE__, __LINE__, s, true);

#define MYASSERT(cond) { \
	if (!(cond)) { \
		std::string _assert_err = std::string("Failed assertion: ") + #cond; \
		MYTHROW(_assert_err); \
	} \
}

#define MYASSERT_NOT_NULL_WITH_DESC(ptr,desc) { if (nullptr==ptr) { \
	std::string _assert_err = std::string(#ptr) + " is not set for " + desc; \
	MYTHROW(_assert_err); } }

#define MYASSERT_NOT_NULL(ptr) { if (nullptr==ptr) { \
	std::string _assert_err = std::string(#ptr) + " is not set"; \
	MYTHROW(_assert_err); } }

#define MYASSERT_WITH_MSG(cond,s) { if (!(cond)) {MYTHROW(s); } }

class ConversionException : public SpecsException {
public:
	explicit ConversionException(const char* _fn, unsigned int _ln,
			const char* _srcf, const char* _dstf, std::string& _src,
			const char* reason = nullptr) : SpecsException(_fn, _ln, "") {
		if (reason) {
			msg = "Cannot convert <" + _src + "> from format <" + _srcf + "> to format <" + _dstf + ">: " + reason;
		} else {
			msg = "Cannot convert <" + _src + "> from format <" + _srcf + "> to format <" + _dstf + ">";
		}
	}
};

#define CONVERSION_EXCEPTION(s,sf,df) throw ConversionException(__FILE__, __LINE__, sf, df, s);
#define CONVERSION_EXCEPTION_EX(s,sf,df,res) throw ConversionException(__FILE__, __LINE__, sf, df, s, res);
#endif
