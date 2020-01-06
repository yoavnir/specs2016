#include <iomanip>
#include <iostream>
#include <sstream>
#include <regex>
#include <string.h>
#include "utils/aluRegex.h"
#include "utils/lruCache.h"

uint64_t regexCacheSearches = 0;
uint64_t regexCacheSets = 0;
uint64_t matchFlagsCacheSets = 0;

static std::regex_constants::syntax_option_type g_regexType = std::regex_constants::ECMAScript;

lruCache<std::string, std::regex> g_regexCache(100,150);

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

void setRegexType(std::string& s) {
	g_regexType = std::regex_constants::ECMAScript;
	char* st = strdup(s.c_str());
	char* p = strtok(st,",");
	while (p) {
		if (0 == strcasecmp(p,"icase")) {
			g_regexType |= std::regex_constants::icase;
		} else if (0 == strcasecmp(p,"nosubs")) {
			g_regexType |= std::regex_constants::nosubs;
		} else if (0 == strcasecmp(p,"optimize")) {
			g_regexType |= std::regex_constants::optimize;
		} else if (0 == strcasecmp(p,"collate")) {
			g_regexType |= std::regex_constants::collate;
		} else if (0 == strcasecmp(p,"ECMAScript")) {
			g_regexType |= std::regex_constants::ECMAScript;
		} else if (0 == strcasecmp(p,"basic")) {
			g_regexType |= std::regex_constants::basic;
		} else if (0 == strcasecmp(p,"extended")) {
			g_regexType |= std::regex_constants::extended;
		} else if (0 == strcasecmp(p,"awk")) {
			g_regexType |= std::regex_constants::awk;
		} else if (0 == strcasecmp(p,"grep")) {
			g_regexType |= std::regex_constants::grep;
		} else if (0 == strcasecmp(p,"egrep")) {
			g_regexType |= std::regex_constants::egrep;
		} else {
			std::string err = "Invalid regular expression syntax option type: " + std::string(p);
			MYTHROW(err);
		}
		p = strtok(NULL, ",");
	}
	free(st);
}

std::regex* regexCalculator(std::string& s)
{
	regexCacheSearches++;
	std::regex* pRet = g_regexCache.get(s);
	if (!pRet) {
		pRet = new std::regex(s,g_regexType);
		g_regexCache.set(s,pRet);
		regexCacheSets++;
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
		std::regex_constants::match_flag_type* pFlags = g_matchFlagsCache.get(str);
		if (!pFlags) {
			static std::regex_constants::match_flag_type ret;
			ret = std::regex_constants::match_default;

			char* st = strdup(str.c_str());
			char* p = strtok(st,",");
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
				p = strtok(NULL, ",");
			}
			free(st);

			pFlags = new std::regex_constants::match_flag_type(ret);
			g_matchFlagsCache.set(str,pFlags);
			matchFlagsCacheSets++;
		}
		return *pFlags;
	}
}

bool regexMatch(ALUValue* pStr, ALUValue* pExp, std::string* pFlags)
{
	std::string sStr = pStr->getStr();
	std::string sExp = pExp->getStr();
	std::regex* pRE = regexCalculator(sExp);

	return std::regex_match(sStr, *pRE, getMatchFlags(pFlags));
}

bool regexSearch(ALUValue* pStr, ALUValue* pExp, std::string* pFlags)
{
	std::string sStr = pStr->getStr();
	std::string sExp = pExp->getStr();
	std::regex* pRE = regexCalculator(sExp);
	return std::regex_search(sStr, *pRE, getMatchFlags(pFlags));
}

std::string regexReplace(ALUValue* pStr, ALUValue* pExp, std::string& pFmt, std::string* pFlags)
{
	std::string sStr = pStr->getStr();
	std::string sExp = pExp->getStr();
	std::regex* pRE = regexCalculator(sExp);
	return std::regex_replace(sStr, *pRE, pFmt, getMatchFlags(pFlags));

}
