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

// GraphicalConsole internal implementation.

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

#pragma once

namespace ExPop
{

    inline std::vector<std::string> graphicalConsoleFileTreeGenerator(
        const std::string &path,
        const std::string &matchExtension = "",
        bool includeDirectories = false)
    {
        std::vector<std::string> ret;

        std::vector<std::string> subdirs;
        ExPop::FileSystem::getSubdirectories(path, subdirs);

        for(size_t i = 0; i < subdirs.size(); i++) {

            std::vector<std::string> subdirContents =
                graphicalConsoleFileTreeGenerator(
                    path + "/" + subdirs[i],
                    matchExtension,
                    includeDirectories);

            if(includeDirectories) {
                ret.push_back(subdirs[i]);
            }

            for(size_t k = 0; k < subdirContents.size(); k++) {
                ret.push_back(subdirs[i] + "/" + subdirContents[k]);
            }
        }

        std::vector<std::string> files;
        ExPop::FileSystem::getNondirectories(path, files);

        for(size_t k = 0; k < files.size(); k++) {

            if(matchExtension.size() && !ExPop::stringEndsWith("." + matchExtension, files[k])) {
                continue;
            }

            ret.push_back(files[k]);
        }

        return ret;
    }

    inline std::function<std::vector<std::string>()> graphicalConsoleFileTreeGeneratorGenerator(
        const std::string &path,
        const std::string &matchExtension,
        bool includeDirectories)
    {
        return
            std::bind(
                &graphicalConsoleFileTreeGenerator,
                std::string(path),
                std::string(matchExtension),
                includeDirectories);
    }

    template<typename T>
    inline void GraphicalConsole::setContextualAutocompleteGenerator(
        const std::string &commandOrCvarName,
        size_t parameterNumber,
        std::function<std::vector<T>()> function)
    {
        std::basic_string<uint32_t> utf32Name =
            ExPop::stringUTF8ToUTF32(commandOrCvarName);

        ContextualAutocompleter_Base *&autoCompleterPtr =
            contextualAutocompleters[utf32Name][parameterNumber];

        if(autoCompleterPtr) {
            delete autoCompleterPtr;
        }

        ContextualAutocompleter<T> *realCompleter = new ContextualAutocompleter<T>;
        realCompleter->func = function;

        autoCompleterPtr = realCompleter;
    }

    inline void GraphicalConsole::clearContextualAutocompleteGenerator(
        const std::string &commandOrCvarName,
        size_t parameterNumber)
    {
        std::basic_string<uint32_t> utf32Name =
            ExPop::stringUTF8ToUTF32(commandOrCvarName);

        auto a = contextualAutocompleters.find(utf32Name);
        if(a != contextualAutocompleters.end()) {
            auto b = a->second.find(parameterNumber);
            if(b != a->second.end()) {
                delete b->second;
                a->second.erase(b);
            }
            if(a->second.size() == 0) {
                contextualAutocompleters.erase(a);
            }
        }
    }

    inline GraphicalConsole::ContextualAutocompleter_Base::~ContextualAutocompleter_Base()
    {
    }

    template<typename T>
    inline GraphicalConsole::ContextualAutocompleter<T>::~ContextualAutocompleter<T>()
    {
    }

    inline std::vector<std::string> GraphicalConsole::ContextualAutocompleter_Base::getResultsAsStrings()
    {
        return std::vector<std::string>();
    }

    template<typename T>
    inline std::vector<std::string> GraphicalConsole::ContextualAutocompleter<T>::getResultsAsStrings()
    {
        std::vector<T> realValues = func();
        std::vector<std::string> strings;

        for(size_t i = 0; i < realValues.size(); i++) {
            std::ostringstream ostr;
            ostr << realValues[i];
            strings.push_back(ostr.str());
        }

        return strings;
    }


    template<typename T>
    inline std::vector<std::basic_string<uint32_t> > graphicalConsoleFindCompletions(
        const std::map<std::basic_string<uint32_t>, T> &inputMap,
        const std::basic_string<uint32_t> &inputPartialString,
        std::basic_string<uint32_t> &outputLongestMatch)
    {
        std::vector<std::basic_string<uint32_t> > completionList;

        auto startOfRange = inputMap.lower_bound(inputPartialString);

        if(startOfRange == inputMap.end()) {
            // Nothing found.
            outputLongestMatch = inputPartialString;
            return completionList;
        }

        outputLongestMatch = startOfRange->first;

        // Check to see if our first entry is even something
        // completable from this.
        if(outputLongestMatch.substr(0, inputPartialString.size()) != inputPartialString) {
            outputLongestMatch.clear();
        }

        auto i = startOfRange;
        while(i != inputMap.end() &&
            i->first.substr(0, inputPartialString.size()) == inputPartialString)
        {
            size_t n;
            for(n = 0; n < i->first.size() && n < outputLongestMatch.size(); n++) {
                if(outputLongestMatch[n] != i->first[n]) {
                    // Found a point where they're different. Chop off the
                    // different part.
                    outputLongestMatch = outputLongestMatch.substr(0, n);
                    break;
                }
            }

            // Reached the end of the second one before the first one.
            // Better chop longestCommonSubstring down.
            if(n < outputLongestMatch.size()) {
                outputLongestMatch = outputLongestMatch.substr(0, n);
            }

            completionList.push_back(i->first);
            i++;
        }

        return completionList;
    }

    inline std::basic_string<uint32_t> graphicalConsoleQuoteAndEscapeIfNeeded(const std::basic_string<uint32_t> &in)
    {
        // If there's any whitespace in here, then we need quotes.
        bool needsQuotes = false;
        bool needsEscapes = false;
        for(size_t i = 0; i < in.size(); i++) {
            if(ExPop::isWhiteSpace(in[i])) {
                needsQuotes = true;
            }
            if(in[i] < ' ' || in[i] >= 127) {
                needsEscapes = true;
            }
        }

        std::basic_string<uint32_t> ret = in;

        if(needsEscapes || needsQuotes) {
            std::basic_string<uint32_t> quoteStr = ExPop::stringUTF8ToUTF32("\"");
            // FIXME: ExPop::stringEscape() is broken for UTF-32.
            ret = quoteStr + ExPop::stringUTF8ToUTF32(ExPop::stringEscape(ExPop::stringUTF32ToUTF8(ret))) + quoteStr;
        }

        return ret;
    }

    inline void GraphicalConsole::editAutocomplete()
    {
        std::basic_string<uint32_t> editLineSubPart = editLineBuffer.substr(0, editLineCursorLocation);
        std::basic_string<uint32_t> editLineRestOfStuff = editLineBuffer.substr(editLineCursorLocation);
        std::vector<std::string> splitParts =
            graphicalConsoleSplitLine(
                ExPop::stringUTF32ToUTF8(editLineSubPart));

        // If we end on a space, stick an empty slot in the end.
        if(editLineSubPart.size() && ExPop::isWhiteSpace(editLineSubPart[editLineSubPart.size() - 1])) {
            splitParts.push_back("");
        }

        // Extract the partially complete input.
        std::basic_string<uint32_t> inputPartial =
            splitParts.size() ?
            ExPop::stringUTF8ToUTF32(splitParts[splitParts.size() - 1]) :
            ExPop::stringUTF8ToUTF32("");

        std::basic_string<uint32_t> longestCommonSubstring = inputPartial;

        if(splitParts.size() <= 1) {

            // Special case for the first thing in the list. Complete
            // to a command or cvar.

            std::vector<std::basic_string<uint32_t> > completionList =
                graphicalConsoleFindCompletions(
                    commandsAndCvars,
                    inputPartial,
                    longestCommonSubstring);

            // Output the list if we have many matches.
            if(completionList.size() > 1) {

                out << "Completions for \""
                    << ExPop::stringUTF32ToUTF8(inputPartial)
                    << "\":" << std::endl;

                for(size_t i = 0; i < completionList.size(); i++) {

                    CommandOrVariableEntry_Base *entry = commandsAndCvars[completionList[i]];

                    out << "  "
                        << ExPop::stringUTF32ToUTF8(completionList[i])
                        << " "
                        << (entry->type == CommandOrVariableEntry_Base::COMMAND ? "(command)" : "(cvar)")
                        << std::endl;
                }
            }

            // Actually complete as much as we can.
            if(longestCommonSubstring.size() >= inputPartial.size()) {

                editLineBuffer = longestCommonSubstring;
                editLineCursorLocation = editLineBuffer.size();

                // If we only have a single thing matching, then just
                // add the space automatically.
                if(completionList.size() == 1 && !editLineRestOfStuff.size()) {
                    editLineBuffer.append(1, ' ');
                    editLineCursorLocation++;
                }

                // Append back whatever was in the rest of the line.
                editLineBuffer += editLineRestOfStuff;
            }

        } else {

            // Attempt to find an autocompleter for the command
            // specified at the beginning.

            auto autoCompleterItr1 = contextualAutocompleters.find(ExPop::stringUTF8ToUTF32(splitParts[0]));
            if(autoCompleterItr1 != contextualAutocompleters.end() && splitParts.size() >= 2) {

                // Find a completer for the specific parameter index.

                auto autoCompleterItr2 = autoCompleterItr1->second.find(splitParts.size() - 2);
                if(autoCompleterItr2 != autoCompleterItr1->second.end()) {

                    // Run the auto completer function and shove everything
                    // into a std::map.
                    std::map<std::basic_string<uint32_t>, bool> completionMap;
                    std::vector<std::string> completionVec =
                        autoCompleterItr2->second->getResultsAsStrings();
                    for(size_t i = 0; i < completionVec.size(); i++) {
                        completionMap[ExPop::stringUTF8ToUTF32(completionVec[i])] = true;
                    }

                    // Build completion list from that map.
                    std::vector<std::basic_string<uint32_t> > completionList =
                        graphicalConsoleFindCompletions(
                            completionMap,
                            inputPartial,
                            longestCommonSubstring);

                    // Output the list if we have many matches.
                    if(completionList.size() > 1) {

                        out << "Completions for \""
                            << ExPop::stringUTF32ToUTF8(inputPartial)
                            << "\":" << std::endl;

                        for(size_t i = 0; i < completionList.size(); i++) {

                            out << "  "
                                << ExPop::stringUTF32ToUTF8(completionList[i])
                                << std::endl;
                        }
                    }

                    // Actually complete as much as we can.
                    if(longestCommonSubstring.size() >= inputPartial.size()) {

                        std::basic_ostringstream<uint32_t> lineStr;

                        // Add the command first.
                        if(splitParts.size() > 0) {
                            lineStr << graphicalConsoleQuoteAndEscapeIfNeeded(
                                ExPop::stringUTF8ToUTF32(splitParts[0]));
                        }

                        // Add all the parameters back.
                        for(size_t i = 1; i < splitParts.size() - 1; i++) {
                            lineStr << ExPop::stringUTF8ToUTF32(" ") << graphicalConsoleQuoteAndEscapeIfNeeded(
                                ExPop::stringUTF8ToUTF32(splitParts[i]));
                        }

                        // Add our mostly-completed string.
                        editLineBuffer = lineStr.str() + ExPop::stringUTF8ToUTF32(" ") + longestCommonSubstring;

                        // Move the cursor to what is currently the
                        // end of the line (so it appears that text
                        // was inserted and the cursor moved).
                        editLineCursorLocation = editLineBuffer.size();

                        // If we only have a single thing matching,
                        // then just add the space automatically to
                        // separate it from the next parameter.
                        if(completionList.size() == 1 && !editLineRestOfStuff.size()) {
                            editLineBuffer.append(1, ' ');
                            editLineCursorLocation++;
                        }

                        // Append back whatever was in the rest of the
                        // line.
                        editLineBuffer += editLineRestOfStuff;
                    }
                }
            }
        }

        backBufferIsDirty = true;
    }

}

