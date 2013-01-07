#pragma once

#include <vector>
#include <string>
#include <map>

#include "pooledstring.h"

namespace ExPop {

    /// DerpErrorState handles errors both internally and exterally.
    /// If you're in a C++ function that was called from a piece of
    /// script code, it'll be available to you, pre-set with
    /// information about the last line that was being executed from
    /// the script. All you have to do to throw an error is call
    /// addError() with your error message and return a NULL
    /// DerpObject reference.
    class DerpErrorState {
    public:

        DerpErrorState(void);

        /// Add an error to the list of errors. File and line number
        /// information is updated internally, so if you call this
        /// from a callback it will still know where in the source
        /// file the problem happened.
        void addError(const std::string &error);

        /// Get the number of errors. The VM will generally bail out
        /// after one error, but it's possible for them to stack up if
        /// errors are being added externally.
        unsigned int getNumErrors(void);

        /// Clear out the error state. If something caused an error,
        /// you should do this after handling the error. If you do
        /// not, an assert may be triggered if the DerpVM is destroyed
        /// before this object due to the remaining pooled string
        /// references.
        void reset(void);

        /// Get all the errors in a nicely formatted block of text.
        std::string getAllErrorText(void);

        /// Get the error text for an individual error.
        std::string getErrorText(unsigned int errorNum);

        // Internal stuff follows...

        /// Used internally to set the location in the file so that
        /// the next error has correct information, even if we travel
        /// into some weird land where that information isn't present
        /// (function callbacks, etc).
        void setFileAndLine(
            PooledString::Ref fileName,
            unsigned int lineNumber);

        /// Used internally as setFileAndLine, but during the parsing
        /// stage before the file names are stored in string pools.
        /// Does the same thing, but this is slower so we don't want
        /// to do it all the time during execution.
        void setFileAndLineDirect(
            const std::string &fileName,
            unsigned int lineNumber);

    private:

        class DerpError {
        public:

            DerpError(
                const std::string &fileName,
                unsigned int lineNumber,
                const std::string &errorText);

            unsigned int lineNumber;
            std::string fileName;
            std::string errorText;
        };

        unsigned int lastLineNumber;
        PooledString::Ref lastFileName;
        std::string lastFileName_direct;
        std::vector<DerpError> errors;

    };
}


