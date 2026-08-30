#include <iomanip>
#include <iostream>
#include <sstream>
#include <regex>
#include <cstring>
#include <memory>
#include "utils/platform.h"
#include "utils/aluRegex.h"
#include "utils/lruCache.h"

uint64_t regexCacheSearches = 0;
uint64_t regexCacheSets = 0;
uint64_t matchFlagsCacheSets = 0;
bool     g_RegexCacheDisabled = false;
bool     g_bWarnAboutGrammars = true;

typedef std::shared_ptr<std::regex> PRegEx;

static std::regex_constants::syntax_option_type g_regexType = std::regex_constants::ECMAScript;
static std::string gs_regexType = "";

lruCache<std::string, std::regex> g_regexCache(100,150);

void disableRegexCache() {
	g_RegexCacheDisabled = true;
}

void dumpRegexStats() {
	if (regexCacheSearches > 0) {
		std::ostringstream oss;
		oss.setf( std::ios::fixed, std:: ios::floatfield );
		oss.precision(3);

		oss << "Regular Expression stats:\n";

		// Total number of searches
		oss << "\tRegular Expression calls: " << regexCacheSearches << "\n";

		// regex cache hits
		double percentage = double(100.0) - (double(100 * regexCacheSets) / double(regexCacheSearches));
		oss << "\tRegex Cache Hits: " << percentage << "%\n";

		// match flag cache hits
		percentage = double(100.0) - (double(100 * matchFlagsCacheSets) / double(regexCacheSearches));
		oss << "\tMatch Flags Cache Hits: " << percentage << "%\n";

		std::cerr << oss.str();
	}
}

// REGEX_GRAMMARS names the std::regex implementation family for the current
// build platform ("linux", "mac", or "windows"); it is always defined by
// setup.py (POSIX builds) or the .vcxproj files (Windows builds). The
// fallback below only matters for some hypothetical build that skips both.
// It arrives on the command line as a bare (unquoted, since the shell strips
// the quotes) token, so it must go through STRINGIFY()+dequote(), the same
// convention used for GITTAG/LITERAL_PLATFORM in Config.cc and specs.cc.
#ifndef REGEX_GRAMMARS
#define REGEX_GRAMMARS unknown
#endif
#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

// Grammars whose std::regex implementation is currently known to diverge
// from the C++ standard's required matching semantics on a given platform.
// This is not about a grammar being "unsupported" - it compiles and runs
// fine everywhere - it is about known-incorrect *results*:
//   - Linux (libstdc++): "extended", "awk", and "egrep" do not implement
//     POSIX leftmost-longest matching for alternation (GCC bug 61424,
//     open since 2014).
//   - macOS (libc++) and Windows (MSVC STL, current toolchains) are
//     conformant for every grammar.
static bool isKnownBuggyGrammar(const char* grammarName) {
	static const std::string platform = dequote(STRINGIFY(REGEX_GRAMMARS));
	if (platform == "linux") {
		return 0 == strcasecmp(grammarName,"extended")
		    || 0 == strcasecmp(grammarName,"awk")
		    || 0 == strcasecmp(grammarName,"egrep");
	}
	return false;
}

void setRegexWarn(std::string& s) {
	g_bWarnAboutGrammars = !(s=="0" || 0==strcasecmp(s.c_str(),"false") || 0==strcasecmp(s.c_str(),"no"));
}

void setRegexType(std::string& s) {
	bool bWarnUnsupportedOption = false;
	bool bWarnBuggyGrammar = false;
	g_regexType = std::regex_constants::ECMAScript;
	gs_regexType = s;
	std::unique_ptr<char, decltype(&free)> st_guard(strdup(s.c_str()), free);
	char* st = st_guard.get();
	char* st_ctx = st;
	char* p = strtok_r(st,",", &st_ctx);
	while (p) {
		bWarnUnsupportedOption = false;
		bWarnBuggyGrammar = false;
		if (0 == strcasecmp(p,"icase")) {
			g_regexType |= std::regex_constants::icase;
		} else if (0 == strcasecmp(p,"nosubs")) {
			g_regexType |= std::regex_constants::nosubs;
			bWarnUnsupportedOption = true;
		} else if (0 == strcasecmp(p,"optimize")) {
			g_regexType |= std::regex_constants::optimize;
		} else if (0 == strcasecmp(p,"collate")) {
			g_regexType |= std::regex_constants::collate;
		} else if (0 == strcasecmp(p,"ECMAScript")) {
			g_regexType = std::regex_constants::ECMAScript;
		} else if (0 == strcasecmp(p,"basic")) {
			g_regexType = std::regex_constants::basic;
			bWarnBuggyGrammar = isKnownBuggyGrammar(p);
		} else if (0 == strcasecmp(p,"extended")) {
			g_regexType = std::regex_constants::extended;
			bWarnBuggyGrammar = isKnownBuggyGrammar(p);
		} else if (0 == strcasecmp(p,"awk")) {
			g_regexType = std::regex_constants::awk;
			bWarnBuggyGrammar = isKnownBuggyGrammar(p);
		} else if (0 == strcasecmp(p,"grep")) {
			g_regexType = std::regex_constants::grep;
			bWarnBuggyGrammar = isKnownBuggyGrammar(p);
		} else if (0 == strcasecmp(p,"egrep")) {
			g_regexType = std::regex_constants::egrep;
			bWarnBuggyGrammar = isKnownBuggyGrammar(p);
		} else {
			std::string err = "Invalid regular expression syntax option type: " + std::string(p);
			MYTHROW(err);
		}
		if (g_bWarnAboutGrammars) {
			if (bWarnUnsupportedOption) {
				std::cerr << "\nWarning: syntax option '" << p << "' is not supported on this platform\n";
			}
			if (bWarnBuggyGrammar) {
				std::cerr << "\nWarning: the '" << p << "' regular expression grammar has known matching bugs on this platform\n";
			}
		}
		p = strtok_r(nullptr, ",", &st_ctx);
	}
}

PRegEx regexCalculator(std::string& s)
{
	regexCacheSearches++;
	PRegEx pRet = g_RegexCacheDisabled ? nullptr : g_regexCache.get(s);
	if (!pRet) {
		try {
			pRet = std::make_shared<std::regex>(s,g_regexType);
		} catch (std::regex_error& e) {
			std::string err = "Invalid regular expression <" + s + "> : " + e.what();
			MYTHROW(err);
		}
		if (!g_RegexCacheDisabled) {
			g_regexCache.set(s,pRet);
			regexCacheSets++;
		}
	}

	return pRet;
}

lruCache<std::string, std::regex_constants::match_flag_type> g_matchFlagsCache(100,150);

std::regex_constants::match_flag_type getMatchFlags(std::string* sFlags)
{
	if (!sFlags || sFlags->size()==0) {
		return std::regex_constants::match_default;
	} else {
		std::string str = *sFlags;
		auto pFlags = g_matchFlagsCache.get(str);
		if (!pFlags) {
			static std::regex_constants::match_flag_type ret;
			ret = std::regex_constants::match_default;

			std::unique_ptr<char, decltype(&free)> st_guard(strdup(str.c_str()), free);
			char* st = st_guard.get();
			char* st_ctx = st;
			char* p = strtok_r(st,",", &st_ctx);
			while (p) {
				if (0 == strcasecmp(p,"default")) {
					ret |= std::regex_constants::match_default;
				} else if (0 == strcasecmp(p,"not_bol")) {
					ret |= std::regex_constants::match_not_bol;
				} else if (0 == strcasecmp(p,"not_eol")) {
					ret |= std::regex_constants::match_not_eol;
				} else if (0 == strcasecmp(p,"not_bow")) {
					ret |= std::regex_constants::match_not_bow;
				} else if (0 == strcasecmp(p,"not_eow")) {
					ret |= std::regex_constants::match_not_eow;
				} else if (0 == strcasecmp(p,"any")) {
					ret |= std::regex_constants::match_any;
				} else if (0 == strcasecmp(p,"not_null")) {
					ret |= std::regex_constants::match_not_null;
				} else if (0 == strcasecmp(p,"continuous")) {
					ret |= std::regex_constants::match_continuous;
				} else if (0 == strcasecmp(p,"prev_avail")) {
					ret |= std::regex_constants::match_prev_avail;
				} else if (0 == strcasecmp(p,"sed")) {
					ret |= std::regex_constants::format_sed;
				} else if (0 == strcasecmp(p,"no_copy")) {
					ret |= std::regex_constants::format_no_copy;
				} else if (0 == strcasecmp(p,"first_only")) {
					ret |= std::regex_constants::format_first_only;
				} else {
					std::string err = "Invalid regular expression match option type: " + std::string(p);
					MYTHROW(err);
				}
				p = strtok_r(nullptr, ",", &st_ctx);
			}

			pFlags = std::make_shared<std::regex_constants::match_flag_type>(ret);
			g_matchFlagsCache.set(str,pFlags);
			matchFlagsCacheSets++;
		}
		return *pFlags;
	}
}

bool regexMatch(std::string* pStr, PValue pExp, std::string* pFlags)
{
	std::string sExp = pExp->getStr();
	PRegEx pRE = regexCalculator(sExp);

	try {
		bool bRet = std::regex_match(*pStr, *pRE, getMatchFlags(pFlags));
		return bRet;
	} catch (std::regex_error& e) {
		auto err = std::string("Error running regular expression match : ") + e.what()
				+ "\nString: " + *pStr + "\nExpression: " + sExp;
		if (pFlags) err += "\nFlags: " + *pFlags;
		MYTHROW(err);
	}
}

bool regexSearch(std::string* pStr, PValue pExp, std::string* pFlags)
{
	std::string sExp = pExp->getStr();
	PRegEx pRE = regexCalculator(sExp);
	try {
		bool bRet = std::regex_search(*pStr, *pRE, getMatchFlags(pFlags));
		return bRet;
	} catch (std::regex_error& e) {
		auto err = std::string("Error running regular expression search : ") + e.what()
				+ "\nString: " + *pStr + "\nExpression: " + sExp;
		if (pFlags) err += "\nFlags: " + *pFlags;
		MYTHROW(err);
	}

}

std::string regexReplace(std::string* pStr, PValue pExp, std::string& fmt, std::string* pFlags)
{
	std::string sExp = pExp->getStr();
	PRegEx pRE = regexCalculator(sExp);
	try {
		std::string sRet = std::regex_replace(*pStr, *pRE, fmt, getMatchFlags(pFlags));
		return sRet;
	} catch (std::regex_error& e) {
		auto err = std::string("Error running regular expression replace : ") + e.what()
				+ "\nString: " + *pStr + "\nExpression: " + sExp + "\nFormat: " + fmt;
		if (pFlags) err += "\nFlags: " + *pFlags;
		MYTHROW(err);
	}


}
