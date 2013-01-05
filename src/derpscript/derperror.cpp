#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdarg>
using namespace std;

#include "derplexer.h"
#include "derperror.h"

namespace ExPop {

    std::string derpSprintf(const char *format, ...) {

        char buf[2048];

        va_list args;
        va_start(args, format);

        // FIXME: Possibly unsupported by C++ earlier than C++11?
        vsnprintf(buf, sizeof(buf) - 1, format, args);

        va_end(args);

        return string(buf);
    }

    unsigned int derpSafeLineNumber(const std::vector<DerpToken*> &tokens, unsigned int i) {

        if(!tokens.size()) {
            return 0;
        }

        if(i >= tokens.size()) {
            return tokens[tokens.size() - 1]->lineNumber;
        }

        return tokens[i]->lineNumber;
    }

    std::string derpSafeFileName(const std::vector<DerpToken*> &tokens, unsigned int i) {

        if(!tokens.size()) {
            return "";
        }

        if(i >= tokens.size()) {
            return tokens[tokens.size() - 1]->fileName;
        }

        return tokens[i]->fileName;
    }

    DerpErrorState::DerpErrorState(void) {

        lastLineNumber  = 0;

    }

    void DerpErrorState::setFileAndLine(
        PooledString::Ref fileName,
        unsigned int lineNumber) {

        lastLineNumber = lineNumber;
        lastFileName = fileName;
        lastFileName_direct = "";
    }

    void DerpErrorState::setFileAndLineDirect(
        const std::string &fileName,
        unsigned int lineNumber) {

        lastLineNumber = lineNumber;
        lastFileName.clear();
        lastFileName_direct = fileName;
    }

    void DerpErrorState::addError(
        const std::string &error) {

        string fileNameDeref =
            lastFileName.isNull() ? lastFileName_direct : lastFileName;

        errors.push_back(
            DerpError(
                fileNameDeref,
                lastLineNumber,
                error));

    }

    unsigned int DerpErrorState::getNumErrors(void) {

        return errors.size();

    }

    void DerpErrorState::reset(void) {
        errors.clear();
        lastLineNumber = 0;
        lastFileName.clear();
        lastFileName_direct = "";
    }

    std::string DerpErrorState::getAllErrorText(void) {

        ostringstream out;
        for(unsigned int i = 0; i < errors.size(); i++) {
            out << getErrorText(i) << endl;
        }

        return out.str();
    }

    std::string DerpErrorState::getErrorText(
        unsigned int errorNum) {

        if(errorNum >= errors.size()) {
            return "";
        }

        ostringstream out;

        out <<
            errors[errorNum].fileName << ":" <<
            errors[errorNum].lineNumber << ": error: " <<
            errors[errorNum].errorText;

        return out.str();
    }

    DerpErrorState::DerpError::DerpError(
        const std::string &fileName,
        unsigned int lineNumber,
        const std::string &errorText) {

        this->fileName = fileName;
        this->lineNumber = lineNumber;
        this->errorText = errorText;
    }

}

