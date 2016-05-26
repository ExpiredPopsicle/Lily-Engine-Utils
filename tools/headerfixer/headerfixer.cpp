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

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
using namespace std;

#include <lilyengine/utils.h>

#include "mainheader.h"
#include "usagetext.h"

using namespace ExPop;

std::string linePrefix = "//";
std::string headerMarker = "-------------------------- END HEADER -------------------------------------";

bool strCompareIgnoreWindowsBullshit(const string &str1, const string &str2)
{
	size_t i = 0;
	size_t j = 0;

	// Go through until one of the strings ends.

	while(i < str1.size() && j < str2.size()) {

		if(str1[i] == '\r') {
			i++;
			continue;
		}

		if(str2[j] == '\r') {
			j++;
			continue;
		}

		if(str1[i] != str2[j]) return false;

		i++;
		j++;
	}

	// Reached the end of at least one of the strings. Go through the
	// end of the other and make sure there's nothing but '\r'.

	while(i < str1.size() || j < str2.size()) {

		if(i < str1.size()) {
			if(str1[i] != '\r') return false;
			i++;
		}

		if(j < str2.size()) {
			if(str2[j] != '\r') return false;
			j++;
		}

	}

	// If we made it this far, we're good.

	return true;
}

void stripOldHeader(vector<string> &lines)
{
	int headerEndLine = -1;
	bool foundHeader = false;

	for(unsigned int i = 0; i < lines.size(); i++) {

		if(foundHeader) {

			// Found the header, pull out extra whitespace.

			if(strCompareIgnoreWindowsBullshit(lines[i], "")) {

				headerEndLine = i;

			} else {

				// Done. Found some code.

				break;
			}

		} else if(strCompareIgnoreWindowsBullshit(lines[i], linePrefix + " " + headerMarker)) {

			// Still looking for header.

			headerEndLine = i;
			foundHeader = true;
		}

	}

	if(headerEndLine != -1) {
		lines.erase(lines.begin(), lines.begin() + headerEndLine + 1);
	}
}

vector<string> loadLinesFromFile(const string &fileName)
{
	int length;
	char *rawContents = FileSystem::loadFile(fileName, &length, true);
	vector<string> lines;

    if(rawContents) {
        stringTokenize(rawContents, "\n", lines, true);
        delete[] rawContents;
    }

	return lines;
}

vector<string> loadLinesFromBuffer(const char *buf)
{
	vector<string> lines;
	stringTokenize(buf, "\n", lines, true);
    return lines;
}

void showHelp(const char *argv0)
{
    cout << stringReplace<char>("$0", argv0, std::string(usageText, usageText_len)) << endl;
}

int main(int argc, char *argv[])
{
    // Parse params.
    std::vector<std::string> paramNames = { "header", "prefix" };
    std::vector<ParsedParameter> params;
    parseCommandLine(argc, argv, paramNames, params);

    std::string headerFilename;
    std::vector<std::string> filenames;
    bool stripMode;

    for(size_t i = 0; i < params.size(); i++) {
        if(params[i].name == "help") {
            showHelp(argv[0]);
            return 0;
        } else if(params[i].name == "dumpheader") {
            cout << mainHeader << endl;
            return 0;
        } else if(params[i].name == "header") {
            headerFilename = params[i].value;
        } else if(params[i].name == "prefix") {
            linePrefix = params[i].value;
        } else if(params[i].name == "strip") {
            stripMode = true;
        } else {
            if(!params[i].name.size()) {
                filenames.push_back(params[i].value);
            } else {
                cerr << "Unknown parameter: " << params[i].name << endl;
            }
        }
    }

    // If we didn't get any parameters at all, dump a help screen and
    // exit.
	if(argc < 2) {
        showHelp(argv[0]);
		return 1;
	}

	vector<string> headerLines;
    if(!headerFilename.size()) {
        headerLines = loadLinesFromBuffer(mainHeader);
    } else {
        headerLines = loadLinesFromFile(headerFilename);
    }

	ostringstream headerConvertedStr;

	for(unsigned int j = 0; j < headerLines.size(); j++) {

        // Make distinction for empty strings here to deal with
        // leading whitespace.
        if(headerLines[j].size()) {
            headerConvertedStr << linePrefix << " " << headerLines[j] << endl;
        } else {
            headerConvertedStr << linePrefix << endl;
        }
	}

	for(int i = 0; i < filenames.size(); i++) {

		string fileName = filenames[i];

		if(!FileSystem::fileExists(fileName)) {
			cout << "File " << fileName << " does not exist!" << endl;
			return 1;
		}

        cout << "Updating " << fileName << "..." << endl;

        ostringstream outputStr;

        vector<string> fileLines = loadLinesFromFile(fileName);

        stripOldHeader(fileLines);

        if(!stripMode) {

            // Add header.
            outputStr << headerConvertedStr.str();

            // Header end. (With extra line for niceness.)
            outputStr << linePrefix << " " << headerMarker << endl << endl;
        }

        for(unsigned int j = 0; j < fileLines.size(); j++) {

            // Add each line.
            outputStr << fileLines[j] << endl;
        }

        string finalOut = outputStr.str();

        //FileSystem::renameFile(fileName, fileName + ".bak");
        FileSystem::saveFile(fileName, finalOut.c_str(), finalOut.size(), false);

	}

	return 0;
}
