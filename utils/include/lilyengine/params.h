// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2016 Clifford Jolly
//     http://expiredpopsicle.com
//     expiredpopsicle@gmail.com
//
// ---------------------------------------------------------------------------
//
//   This software is provided 'as-is', without any express or implied
//   warranty. In no event will the authors be held liable for any
//   damages arising from the use of this software.
//
//   Permission is granted to anyone to use this software for any
//   purpose, including commercial applications, and to alter it and
//   redistribute it freely, subject to the following restrictions:
//
//   1. The origin of this software must not be misrepresented; you must
//      not claim that you wrote the original software. If you use this
//      software in a product, an acknowledgment in the product
//      documentation would be appreciated but is not required.
//
//   2. Altered source versions must be plainly marked as such, and must
//      not be misrepresented as being the original software.
//
//   3. This notice may not be removed or altered from any source
//      distribution.
//
// -------------------------- END HEADER -------------------------------------

// Command line parameter parsing module. Helps to not have to write
// this crap over and over again.

// +Mal | Let's see what's up today for the texture processing tool... I
//      | think it's parsing command line parameters. Ick.
// Chev | doesn't sound too hard but that depends on the number of params I
//      | guess
// +Mal | Well, I started it the other day but realized the approach was
//      | completely wrong.
// +Mal | Because of weird rules.
// +Mal | I'd like it to follow the conventions of other unixy tools.
// +Mal | Like... ignore parameters that are not filenames after the "--"
//      | parameter.
// Chev | right
// +Mal | And long parameters (--param) vs short (-p)
// +Mal | And how you can combine the short ones (-wtf)
// +Mal | But some of them take extra parameters (-f foo)
// +Mal | And you can even reduce them to a single param (-ffoo) when they
//      | do that.
// +Mal | So is -wtffoo equivalent to -wt -f foo ?
// Chev | hurgh
// +Mal | Or is it -w -t -f -o -o
// +Mal | Yeah
// +Mal | I *think* that's all the stuff I have to consider.
// +Mal | But I only realized that after getting halfway done with an
//      | implementation that wouldn't support most of those.
// +Mal | Oh wait. One more.
// +Mal | "-f=foo" style params.
// Chev | I'm lost there
// +Mal | Err...
// +Mal | --file=something
// +Mal | Which is equivalent to --file something
// Chev | oh
// Chev | right, why have a single syntax when you could make it ambiguous
// +Mal | I just experimented with tar, and apparently -f=foo will make an
//      | archive called "=foo" but --file=foo will make an archive called
//      | "foo".
// +Mal | So I guess the = thingy doesn't work for the short form.
// +Mal | Grreat.
// +Mal | I'm going to copy+paste this conversation into my program as a
//      | comment now that I think I've covered everything.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <iomanip>
#include <iostream>
#include <vector>
#include <string>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

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
        std::vector<ParsedParameter> &outputParameters,
        std::ostream &errorOut = std::cerr);

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

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    inline bool parseCommandLine(
        int argc, char *argv[],
        const std::vector<std::string> &paramsWithParameters,
        std::vector<ParsedParameter> &outputParameters,
        std::ostream &errorOut)
    {
        // First get everything into a std::vector that we can work with.
        std::vector<std::string> vecParams;
        for(int i = 1; i < argc; i++) {
            vecParams.push_back(argv[i]);
        }

        bool parseError = false;

        // This lets us keep track of whether or not we've seen the
        // "--" parameter, indicating that further parameters should
        // not be parsed as special. Example use case: Filenames that
        // start with '-', which would be seen as functional
        // parameters. This is pretty common with GNU tools, so I'm
        // doing it here too.
        bool doneWithSpecialParameters = false;

        // Now continue pulling stuff off the front (slow, whatever)
        // and handling them in order until we're out of parameters.
        while(vecParams.size()) {

            // This is pretty inefficient, but I don't think anyone
            // will ever have a command line big enough for it to
            // matter.
            std::string param = vecParams[0];
            vecParams.erase(vecParams.begin());

            if(param.size() && param[0] == '-' && !doneWithSpecialParameters) {

                // This is some kind of special parameter.

                if(param.size() > 1 && param[1] == '-') {

                    // Long-form parameter or end-of-parameters.

                    if(param.size() == 2) {

                        // We got the '--' parameter, indicating the
                        // end of any functional special parameters.
                        doneWithSpecialParameters = true;

                    } else {

                        ParsedParameter outputParam;

                        outputParam.name = param.substr(2, param.size() - 2);

                        // Find an '=' if it exists, and fill in the value
                        // field with it. Also chop down the name to not
                        // have the value or '='.
                        for(size_t k = 0; k < outputParam.name.size(); k++) {
                            if(outputParam.name[k] == '=') {
                                outputParam.value = outputParam.name.substr(k + 1, outputParam.name.size() - k - 1);
                                outputParam.name = outputParam.name.substr(0, k);
                                break;
                            }
                        }

                        // See if we actually need a parameter or not. If
                        // we didn't then whatever we just got with the
                        // '=' is a parse error.
                        bool needsParameter = false;
                        for(size_t j = 0; j < paramsWithParameters.size(); j++) {
                            if(outputParam.name == paramsWithParameters[j]) {
                                needsParameter = true;
                                break;
                            }
                        }

                        if(outputParam.value.size() && !needsParameter) {
                            errorOut << "Too many parameters for --" << outputParam.name << std::endl;
                            parseError = true;
                        }

                        // If we didn't find something with '=', and we DO
                        // need a parameter, then just eat the next input
                        // entirely.
                        if(!outputParam.value.size() && needsParameter) {

                            // Eat next input if needed.
                            if(needsParameter) {
                                if(vecParams.size()) {
                                    outputParam.value = vecParams[0];
                                    vecParams.erase(vecParams.begin());
                                } else {
                                    errorOut << "Missing parameter for --" << outputParam.name << std::endl;
                                    parseError = true;
                                }
                            }
                        }

                        outputParameters.push_back(outputParam);
                    }

                } else {

                    // Short-form paramter(s).

                    std::string shortParams = param.substr(1, param.size() - 1);

                    for(size_t i = 0; i < shortParams.size(); i++) {
                        char tmp[2] = { shortParams[i], 0 };
                        ParsedParameter outputParam;
                        outputParam.name = tmp;

                        // Now see if we're going to need a parameter to
                        // this parameter.
                        bool needsParameter = false;
                        for(size_t j = 0; j < paramsWithParameters.size(); j++) {
                            if(outputParam.name == paramsWithParameters[j]) {
                                needsParameter = true;
                                break;
                            }
                        }

                        if(needsParameter) {

                            if(i < shortParams.size() - 1) {

                                // Just eat the rest of this input.
                                outputParam.value = shortParams.substr(i+1, shortParams.size() - (i+1));
                                i = shortParams.size();

                            } else {

                                // Last thing in this input. Eat the next param entirely!
                                if(vecParams.size()) {
                                    outputParam.value = vecParams[0];
                                    vecParams.erase(vecParams.begin());
                                } else {
                                    errorOut << "Missing parameter for -" << outputParam.name << std::endl;
                                    parseError = true;
                                }

                            }

                        }

                        outputParameters.push_back(outputParam);

                    }

                }


            } else {

                // This is a normal parameter.
                ParsedParameter outputParam;
                outputParam.value = param;
                outputParameters.push_back(outputParam);
            }

        }

        return !parseError;
    }
}

