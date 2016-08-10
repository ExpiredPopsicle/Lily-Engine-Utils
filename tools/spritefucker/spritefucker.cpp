#include <iostream>

#include <lilyengine/utils.h>

#include "usagetext.h"

void showHelp(const char *argv0, bool error)
{
    std::ostream *out = error ? &std::cerr : &std::cout;
    (*out) << ExPop::stringReplace<char>("$0", argv0, std::string(usageText, usageText_len)) << std::endl;
}

int main(int argc, char *argv[])
{
    std::vector<std::string> paramNames = { };
    std::vector<ExPop::ParsedParameter> params;
    parseCommandLine(argc, argv, paramNames, params);

    for(size_t i = 0; i < params.size(); i++) {
        if(params[i].name == "help") {
            showHelp(argv[0], false);
            return 0;
        }
    }

    return 0;
}



