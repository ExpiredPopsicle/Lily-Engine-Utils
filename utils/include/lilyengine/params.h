#pragma once

#include <vector>
#include <string>

namespace ExPop
{
    /// Output structure for parameter values.
    struct ParsedParameter
    {
        /// Short or long. For -f and --file kind of things. Empty for
        /// normal loose parameters (like source files on a compiler).
        std::string name;

        /// Can be empty. For --file=foo, etc. For parameters that are
        /// not parameters to other parameters (like source files on a
        /// compiler) this field is full and the name field is empty.
        std::string value;
    };

    /// Parse a command line. paramsWithParameters is a list of command
    /// line parameters that take an extra parameter and will eat an extra
    /// token to stick into the ParsedParameter::value field. Do not
    /// include the "-" or "--" in the paramsWithParameters list. Handling
    /// unknown parameters or errors is an exercise left to the caller.
    /// Parameters not preceded with "-" or "--" will end up as a
    /// ParsedParameter with an empty name and a filled-in value. Returns
    /// true on success, false on malformed command line (missing params,
    /// too many params, etc).
    bool parseCommandLine(
        int argc, char *argv[],
        const std::vector<std::string> &paramsWithParameters,
        std::vector<ParsedParameter> &outputParameters);

    // Usage example:
    // ----------------------------------------------------------------------
    // int main(int argc, char *argv[])
    // {
    //     std::vector<std::string> paramsWithParameters = {
    //         "foo", "bar", "a"
    //     };
    //     std::vector<ParsedParameter> params;
    //     parseCommandLine(argc, argv, paramsWithParameters, params);
    //
    //     int verbosity = 0;
    //     std::string fooSetting;
    //     std::string barSetting;
    //     std::string aSetting;
    //
    //     for(size_t i = 0; i < params.size(); i++) {
    //
    //         // "--a something", "--a=something", "-asomething", etc
    //         if(params[i] == "a") {
    //             aSetting = params[i].value;
    //         }
    //
    //         // "--foo something", "--foo=something", etc
    //         if(params[i] == "foo") {
    //             fooSetting = params[i].value;
    //         }
    //
    //         // "--bar something", "--bar=something", etc
    //         if(params[i] == "bar") {
    //             barSetting = params[i].value;
    //         }
    //
    //         // This will correctly handle -vvvvv and so on.
    //         if(params[i] == "v" || params[i] == "verbose") {
    //             verbosity++;
    //         }
    //     }
    //
    //     ...
    // }
    // ----------------------------------------------------------------------

}
