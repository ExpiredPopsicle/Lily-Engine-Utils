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

#include <lilyengine/params.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

// 12:34:48             +Mal | Let's see what's up today for the texture processing tool... I     │
//                           | think it's parsing command line parameters. Ick.                   │
// 12:36:45             Chev | doesn't sound too hard but that depends on the number of params I  │
//                           | guess                                                              │
// 12:37:07             +Mal | Well, I started it the other day but realized the approach was     │
//                           | completely wrong.                                                  │
// 12:37:11             +Mal | Because of weird rules.                                            │
// 12:37:23             +Mal | I'd like it to follow the conventions of other unixy tools.        │
// 12:37:50             +Mal | Like... ignore parameters that are not filenames after the "--"    │
//                           | parameter.                                                         │
// 12:37:58             Chev | right                                                              │
// 12:38:01             +Mal | And long parameters (--param) vs short (-p)                        │
// 12:38:10             +Mal | And how you can combine the short ones (-wtf)                      │
// 12:38:24             +Mal | But some of them take extra parameters (-f foo)                    │
// 12:38:42             +Mal | And you can even reduce them to a single param (-ffoo) when they   │
//                           | do that.                                                           │
// 12:38:56             +Mal | So is -wtffoo equivalent to -wt -f foo ?                           │
// 12:38:58             Chev | hurgh                                                              │
// 12:39:07             +Mal | Or is it -w -t -f -o -o                                            │
// 12:39:14             +Mal | Yeah                                                               │
// 12:39:28             +Mal | I *think* that's all the stuff I have to consider.                 │
// 12:39:42             +Mal | But I only realized that after getting halfway done with an        │
//                           | implementation that wouldn't support most of those.                │
// 12:39:48             +Mal | Oh wait. One more.                                                 │
// 12:39:55             +Mal | "-f=foo" style params.                                             │
// 12:40:20             Chev | I'm lost there                                                     │
// 12:40:47             +Mal | Err...                                                             │
// 12:40:54             +Mal | --file=something                                                   │
// 12:41:08             +Mal | Which is equivalent to --file something                            │
// 12:41:21             Chev | oh                                                                 │
// 12:41:44             Chev | right, why have a single syntax when you could make it ambiguous   │
// 12:42:03             +Mal | I just experimented with tar, and apparently -f=foo will make an   │
//                           | archive called "=foo" but --file=foo will make an archive called   │
//                           | "foo".                                                             │
// 12:42:22             +Mal | So I guess the = thingy doesn't work for the short form.           │
// 12:42:34             +Mal | Grreat.                                                            │
// 12:42:56             +Mal | I'm going to copy+paste this conversation into my program as a     │
//                           | comment now that I think I've covered everything.                  │
namespace ExPop
{

    bool parseCommandLine(
        int argc, char *argv[],
        const std::vector<std::string> &paramsWithParameters,
        std::vector<ParsedParameter> &outputParameters)
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
            string param = vecParams[0];
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
                            cerr << "Too many parameters for --" << outputParam.name << endl;
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
                                    cerr << "Missing parameter for --" << outputParam.name << endl;
                                    parseError = true;
                                }
                            }
                        }

                        outputParameters.push_back(outputParam);
                    }

                } else {

                    // Short-form paramter(s).

                    string shortParams = param.substr(1, param.size() - 1);

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
                                    cerr << "Missing parameter for -" << outputParam.name << endl;
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


