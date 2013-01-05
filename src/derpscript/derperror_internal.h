#pragma once

namespace ExPop {

    class DerpToken;

    // Internal error helpers.

    std::string derpSprintf(const char *format, ...);

    unsigned int derpSafeLineNumber(
        const std::vector<DerpToken*> &tokens,
        unsigned int i);

    std::string derpSafeFileName(
        const std::vector<DerpToken*> &tokens,
        unsigned int i);
}

