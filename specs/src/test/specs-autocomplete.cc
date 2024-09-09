#include <string.h>
#include <iostream>
#include <filesystem>
#include "processing/Config.h"

int CompleteIfUnambiguous(std::string& incomplete, std::string& prevToken, std::string& line)
{
    std::string ret;
    if (prevToken=="-f" || 0==strcasecmp("specfile", prevToken.c_str())) {
        std::filesystem::path specPath(getFullSpecPath());

        for (auto const& dir_entry : std::filesystem::directory_iterator{specPath}) {
            auto fname = dir_entry.path().stem().string();
            std::cerr << fname << '\n';
            if (0 == fname.compare(0, incomplete.size(), incomplete)) {
                if (ret.size()) return 0;  // already found a candidate - this is ambiguous
                ret = fname;
            }
        }
        std::cerr << incomplete << " - " << line << " - " << ret << "\n";
        if (ret.size()) std::cout << ret;
        return 0;
    }
    
    return 0;
}

int CompleteUncertain(std::string& incomplete, std::string& prevToken, std::string& line)
{
    std::string ret;
    if (prevToken=="-f" || 0==strcasecmp("specfile", prevToken.c_str())) {
        std::filesystem::path specPath(getFullSpecPath());

        for (auto const& dir_entry : std::filesystem::directory_iterator{specPath}) {
            auto fname = dir_entry.path().stem().string();
            std::cerr << fname << '\n';
            if (0 == fname.compare(0, incomplete.size(), incomplete)) {
                if (ret.size()) {
                    ret = ret + " " + fname;
                } else {
                    ret = fname;
                }
            }
        }
        std::cerr << incomplete << " - " << line << " - " << ret << "\n";
        if (ret.size()) std::cout << ret;
        return 0;
    }
    
    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 4) return 0;

    std::string incomplete(argv[2]);
    std::string prevToken(argv[3]);
    char* safe = getenv("COMP_LINE");
    std::string line(safe ? safe : "");
    safe = getenv("COMP_POINT");
    auto cursorPos = safe ? std::stoul(safe) : line.length();
    safe = getenv("COMP_TYPE");
    char type = safe ? char(std::stoi(safe)) : '\t';

    // Only auto-complete when in the last position
    if (cursorPos < line.length()) return 0;

    switch (type) {
        case '\t': 
            return CompleteIfUnambiguous(incomplete, prevToken, line);
        case '?':
            return CompleteUncertain(incomplete, prevToken, line);
        default:
            break;
    }
    return 0;
}
