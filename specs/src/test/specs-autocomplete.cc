#include <string.h>
#include <iostream>
#include <vector>
#include <filesystem>
#include "processing/Config.h"
#include "utils/platform.h"

typedef std::vector<std::string> StringVector;

void GetFilesByPrefix(StringVector& sv, const char* path, std::string& prefix)
{
    std::filesystem::path specPath(path);
    
    for (auto const& dirEnt : std::filesystem::directory_iterator(specPath)) {
        if (!dirEnt.is_regular_file()) continue;
        auto fname = dirEnt.path().stem().string();
        if (prefix.empty() || 0 == fname.compare(0, prefix.size(), prefix)) {
            sv.push_back(fname);
        }
    }
}

static void getFilenameVector(StringVector& sv, std::string& incomplete, std::string& prevToken)
{
    if (prevToken=="-f" || 0==strcasecmp("--specfile", prevToken.c_str())) {
        GetFilesByPrefix(sv, getFullSpecPath(), incomplete);
        if (sv.empty()) {
            GetFilesByPrefix(sv, ".", incomplete);
        }        
    } else if (prevToken=="-i"  || prevToken=="-c" ||
            0==strcasecmp("--inFile", prevToken.c_str()) ||
            0==strcasecmp("--config", prevToken.c_str()) ||
            (prevToken.length()==5 && 0==strncasecmp("--is", prevToken.c_str(),4))) {
        GetFilesByPrefix(sv, ".", incomplete);
    }
}

int CompleteUncertain(std::string& incomplete, std::string& prevToken, std::string& line)
{
    StringVector sv;

    getFilenameVector(sv, incomplete, prevToken);

    bool bFirst = true;
    for (auto const& s : sv) {
       if (bFirst) {
            std::cout << s;
            bFirst = false;
        } else std::cout << ' ' << s;
    }
        
    if (!bFirst) std::cout << "\n";

    return 0;
}

int CompleteIfUnambiguous(std::string& incomplete, std::string& prevToken, std::string& line)
{
    StringVector sv;

    getFilenameVector(sv, incomplete, prevToken);
        
    if (sv.size()==1) {
        std::cout << sv[0] << "\n";
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
            std::cerr << "\nType: <" << type << "> (" << int(type) << ")\n";  // TODO Remove before merge
            break;
    }
    return 0;
}
