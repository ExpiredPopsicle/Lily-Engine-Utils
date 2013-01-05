#pragma once

#include <vector>
#include <string>

namespace ExPop {

    class DerpExecNode;
    class DerpVM;
    class DerpToken;
    class DerpErrorState;

    DerpExecNode *derpParseText(
        DerpVM *vm, const std::string &str,
        DerpErrorState &errorState,
        const std::string &fileName);
}


