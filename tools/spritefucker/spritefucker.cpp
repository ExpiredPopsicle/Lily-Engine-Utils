#include <lilyengine/utils.h>

#include <sstream>
#include <string>
#include <iostream>
#include <functional>

#include "usagetext.h"

class CommandlineParser
{
public:

    CommandlineParser(const std::string &inArgv0);
    ~CommandlineParser();

    /// Create a function handler for some command line parameter that
    /// takes a single additional parameter.
    template<typename T>
    void addHandler(
        const std::string &paramName,
        std::function<void(T)> handler);

    /// Create a function handler for some command line parameter that
    /// takes no additional parameters.
    void addHandler(
        const std::string &paramName,
        std::function<void()> handler);

    /// Add a handler that just sets a value in a variable. Creates a
    /// command line parameter that requires an additional parameter,
    /// which will be the value to be saved in the variable.
    template<typename T>
    void addVariableHandler(const std::string &paramName, T *value);

    /// Add a handler for a (boolean or integer) value, which will be
    /// set to true of the flag shows up on the command line. The
    /// resulting command line parameter will not require any
    /// additional parameters.
    template<typename T>
    void addFlagHandler(const std::string &paramName, T *value);

    void setParameterAlias(
        const std::string &parameterName,
        const std::string &alias);

    /// Handle a command line using the configured callbacks. This
    /// will return false if an error occurs, OR if
    /// raiseEarlyOutFlag() has been called by any of the handlers (to
    /// handle --help, --version, etc).
    bool handleCommandline(int argc, char *argv[]);

    /// Set the documentation for a given command line parameter. The
    /// parameter must already have a handler. valueName must be empty
    /// if the parameter does not expect additional parameters.
    /// valueName must NOT be empty if the parameter does expect
    /// additional parameters.
    void setParameterDoc(
        const std::string &paramName,
        const std::string &doc,
        const std::string &valueName = "");

    /// Set the documentation parts for the non-parameter parts.
    /// inVersionString should be a version string, and what you
    /// expect to print out when the user uses the --version command
    /// line parameter. inUsageString should be a short usage string,
    /// not including the name of the executable. header is the
    /// documentation before the options list. footer is the
    /// documentation after the options list (where you can put a
    /// copyright notice or similar).
    void setDoc(
        const std::string &inVersionString,
        const std::string &inUsageString,
        const std::string &header,
        const std::string &footer);

    /// Get the complete help text, formatted to a given column
    /// length.
    std::string getHelpText(size_t columns = 80);

    /// Get just the version string.
    std::string getVersionText();

    /// Get just the usage text.
    std::string getUsageText(size_t columns = 80);

    /// Use this from inside a handler to indicate that you wish to
    /// quit and stop processing parameters. handleCommandline() will
    /// return back to the host application immediately with a false
    /// result. The host application should then check for errors, or
    /// just quit.
    void raiseEarlyOutFlag();

    /// Raise the error flag explicitly. This is usually not necessary
    /// if you just return false from an error handler.
    void raiseErrorFlag();

    /// Get the flag indicating that we've done something that needs
    /// us to exit early. It might not be an error. Something like
    /// --help or --version will cause this without an error.
    bool getEarlyOutFlag();

    /// Get the flag indicating that we've had an error.
    bool getErrorFlag();

    /// Throw an error from one of the handlers. This will raise the
    /// error flag, the early out flag, and write the error message to
    /// the error stream output.
    void throwError(const std::string &errorText);

private:

    // FIXME: Make this modifiable.
    std::ostream *errorStream;

    class HandlerWrapper_Base
    {
    public:
        HandlerWrapper_Base(CommandlineParser *inParent);
        virtual ~HandlerWrapper_Base();
        virtual void handle(const std::string &value);
        virtual void handle();
        virtual bool isZeroParam();
    private:
        CommandlineParser *parent;
    };

    template<typename T>
    class HandlerWrapper : public HandlerWrapper_Base
    {
    public:
        HandlerWrapper(CommandlineParser *inParent, std::function<void(T)> &inHandlerFunction);
        virtual void handle(const std::string &value) override;
        virtual bool isZeroParam() override;
    private:
        std::function<void(T)> handlerFunction;
    };

    class HandlerWrapper_NoParam : public HandlerWrapper_Base
    {
    public:
        HandlerWrapper_NoParam(CommandlineParser *inParent, std::function<void()> &inHandlerFunction);
        virtual void handle() override;
    private:
        std::function<void()> handlerFunction;
    };

    std::map<std::string, std::vector<HandlerWrapper_Base*> > handlerWrappers;
    std::map<std::string, std::string> parameterAliases;

    struct ParameterDoc
    {
        std::string doc;
        std::string valueName;
    };
    std::map<std::string, ParameterDoc> parameterDocumentation;
    std::string documentationHeader;
    std::string documentationFooter;

    std::string usageString;
    std::string versionString;

    std::string argv0;

    // This is the only real "state" this thing has that is meant to
    // change after setup. If we want to use it multiple times in a
    // single instance, we'll have to clear this.
    bool earlyOutFlag;
    bool errorFlag;
};

// ----------------------------------------------------------------------

template<typename T>
inline void CommandlineParser::addHandler(
    const std::string &paramName,
    std::function<void(T)> handler)
{
    HandlerWrapper<T> *wrapper = new HandlerWrapper<T>(this, handler);

    std::vector<HandlerWrapper_Base*> &wrappers = handlerWrappers[paramName];

    // Check that no zero-param wrappers are here already, because
    // we're making this parameter one that does require parameters.
    for(size_t i = 0; i < wrappers.size(); i++) {
        assert(!wrappers[i]->isZeroParam());
    }

    wrappers.push_back(wrapper);
}

inline void CommandlineParser::addHandler(
    const std::string &paramName,
    std::function<void()> handler)
{
    HandlerWrapper_NoParam *wrapper = new HandlerWrapper_NoParam(this, handler);

    std::vector<HandlerWrapper_Base*> &wrappers = handlerWrappers[paramName];

    // Check that no non-zero-param wrappers are here already, because
    // we're making this parameter one that does NOT require
    // parameters.
    for(size_t i = 0; i < wrappers.size(); i++) {
        assert(wrappers[i]->isZeroParam());
    }

    wrappers.push_back(wrapper);
}

template<typename T>
inline void CommandlineParser::addVariableHandler(
    const std::string &paramName,
    T *value)
{
    addHandler<T>(
        paramName,
        [value](T v) {
            *value = v;
        });
}

template<typename T>
inline void CommandlineParser::addFlagHandler(
    const std::string &paramName,
    T *value)
{
    addHandler(
        paramName,
        [value]() {
            *value = true;
        });
}

void CommandlineParser::setParameterAlias(
    const std::string &parameterName,
    const std::string &alias)
{
    // Make sure we aren't adding an alias to something that doesn't
    // exist.
    assert(handlerWrappers.find(parameterName) != handlerWrappers.end());

    parameterAliases[alias] = parameterName;
}

inline void CommandlineParser::raiseEarlyOutFlag()
{
    earlyOutFlag = true;
}

inline bool CommandlineParser::getEarlyOutFlag()
{
    return earlyOutFlag;
}

inline void CommandlineParser::raiseErrorFlag()
{
    raiseEarlyOutFlag();
    errorFlag = true;
}

inline bool CommandlineParser::getErrorFlag()
{
    return errorFlag;
}

inline void CommandlineParser::throwError(const std::string &errorText)
{
    (*errorStream) << errorText << std::endl;
    raiseErrorFlag();
}

inline CommandlineParser::CommandlineParser(const std::string &inArgv0)
{
    errorStream = &std::cerr;
    argv0 = inArgv0;

    earlyOutFlag = false;
    errorFlag = false;

    addHandler(
        "help",
        [this]() {
            raiseEarlyOutFlag();
            std::cout << getHelpText() << std::endl;
        });

    addHandler(
        "version",
        [this]() {
            raiseEarlyOutFlag();
            std::cout << getVersionText() << std::endl;
        });

    setParameterDoc("help",    "Print help and quit.");
    setParameterDoc("version", "Print version information and quit.");
}

inline CommandlineParser::~CommandlineParser()
{
    for(auto k = handlerWrappers.begin(); k != handlerWrappers.end(); k++) {
        for(size_t i = 0; i < k->second.size(); i++) {
            delete k->second[i];
        }
    }
    handlerWrappers.clear();
}

inline bool CommandlineParser::handleCommandline(int argc, char *argv[])
{
    std::vector<std::string> paramsWithParameters;
    std::vector<ExPop::ParsedParameter> outputParameters;

    // Add extra-parameter-taking parameters.
    for(auto i = handlerWrappers.begin(); i != handlerWrappers.end(); i++) {
        for(size_t k = 0; k < i->second.size(); k++) {
            if(!i->second[k]->isZeroParam()) {
                paramsWithParameters.push_back(i->first);
                break;
            }
        }
    }

    // Add extra-parameter-taking aliases of parameters.
    for(auto k = parameterAliases.begin(); k != parameterAliases.end(); k++) {
        if(handlerWrappers[k->second].size()) {
            if(!handlerWrappers[k->second][0]->isZeroParam()) {
                paramsWithParameters.push_back(k->first);
            }
        }
    }

    bool success = parseCommandLine(
        argc, argv,
        paramsWithParameters,
        outputParameters,
        *errorStream);

    if(!success) {
        raiseErrorFlag();
    }

    if(success) {

        for(size_t i = 0; i < outputParameters.size(); i++) {

            std::string finalParamName = outputParameters[i].name;

            // Check to see if this is an alias.
            auto aliasItr = parameterAliases.find(finalParamName);
            if(aliasItr != parameterAliases.end()) {
                finalParamName = aliasItr->second;
            }

            auto handlerItr = handlerWrappers.find(finalParamName);
            if(handlerItr == handlerWrappers.end()) {
                (*errorStream) << "Unknown parameter: " << outputParameters[i].name << std::endl;
                raiseErrorFlag();
                break;
            }

            for(size_t k = 0; k < handlerItr->second.size(); k++) {
                if(handlerItr->second[k]->isZeroParam()) {
                    handlerItr->second[k]->handle();
                } else {
                    handlerItr->second[k]->handle(outputParameters[i].value);
                }
            }

            if(getErrorFlag()) {
                raiseEarlyOutFlag();
            }

            if(getEarlyOutFlag()) {
                break;
            }
        }
    }

    if(!success || getErrorFlag()) {
        *errorStream << "Try \"" << argv[0] << " --help\" for information." << std::endl;
    }

    return !getEarlyOutFlag();
}

inline CommandlineParser::HandlerWrapper_Base::HandlerWrapper_Base(CommandlineParser *inParent)
{
    parent = inParent;
}

inline CommandlineParser::HandlerWrapper_Base::~HandlerWrapper_Base()
{
}

inline void CommandlineParser::HandlerWrapper_Base::handle(const std::string &value)
{
    (*parent->errorStream) << "Got a parameter when one was not expected." << std::endl;
    parent->raiseErrorFlag();
}

inline void CommandlineParser::HandlerWrapper_Base::handle()
{
    (*parent->errorStream) << "Got no parameters when one was expected." << std::endl;
    parent->raiseErrorFlag();
}

inline bool CommandlineParser::HandlerWrapper_Base::isZeroParam()
{
    return true;
}

template<typename T>
inline CommandlineParser::HandlerWrapper<T>::HandlerWrapper(
    CommandlineParser *inParent,
    std::function<void(T)> &inHandlerFunction) :
    HandlerWrapper_Base(inParent)
{
    handlerFunction = inHandlerFunction;
}

template<typename T>
inline void CommandlineParser::HandlerWrapper<T>::handle(const std::string &value)
{
    T convertedValue;
    std::istringstream istr(value);
    istr >> convertedValue;
    handlerFunction(convertedValue);
}

template<typename T>
inline bool CommandlineParser::HandlerWrapper<T>::isZeroParam()
{
    return false;
}

inline CommandlineParser::HandlerWrapper_NoParam::HandlerWrapper_NoParam(
    CommandlineParser *inParent,
    std::function<void()> &inHandlerFunction) :
    HandlerWrapper_Base(inParent)
{
    handlerFunction = inHandlerFunction;
}

inline void CommandlineParser::HandlerWrapper_NoParam::handle()
{
    handlerFunction();
}

// ----------------------------------------------------------------------

inline std::string wrapParagraphs(const std::string &block, size_t columns)
{
    std::vector<std::string> paragraphs;

    std::string tmp = block;
    while(tmp.size()) {
        std::string paragraph;
        std::string tmp2;
        ExPop::stringSplit(tmp, "\n\n", paragraph, tmp2);
        tmp = tmp2;
        paragraphs.push_back(ExPop::stringWordWrap(paragraph, columns, columns));
    }

    std::ostringstream ostr;

    for(size_t i = 0; i < paragraphs.size(); i++) {
        ostr << paragraphs[i] << std::endl;
        if(i + 1 != paragraphs.size()) {
            ostr << std::endl;
        }
    }

    return ostr.str();
}

// Ugly, slow way to do this.
inline std::string makeSpaces(size_t numSpaces)
{
    std::string ret;
    ret.append(numSpaces, ' ');
    return ret;
}

inline std::string formatBlock(
    const std::string &paramName,
    const std::string &valueName,
    const std::string &block,
    size_t columns = 80,
    size_t paramSpace = 20)
{
    // FIXME: Remove these.
    const size_t spacing = paramSpace;
    const size_t totalWidth = columns;

    std::string paragraphs = wrapParagraphs(block, totalWidth - spacing);
    std::vector<std::string> paragraphLines;
    ExPop::stringTokenize(paragraphs, "\n", paragraphLines, true);

    std::string usagePart;
    if(valueName.size()) {
        if(paramName.size() == 1) {
            usagePart = "-" + paramName + " <" + valueName + ">";
        } else {
            usagePart = "--" + paramName + "=<" + valueName + ">";
        }
    } else {
        if(paramName.size() == 1) {
            usagePart = "-" + paramName;
        } else {
            usagePart = "--" + paramName;
        }
    }

    usagePart = "  " + usagePart;
    std::string spacesPart = makeSpaces(spacing);

    std::ostringstream out;

    if(usagePart.size() > spacing - 2) {

        // The param name + value is longer than the halfway point, so
        // we need to just start the description on the next line.
        out << usagePart << std::endl;
        for(size_t i = 0; i < paragraphLines.size(); i++) {
            out << spacesPart << paragraphLines[i] << std::endl;
        }

    } else {
        std::string usageWithSpaces = usagePart + makeSpaces(spacing - usagePart.size());

        if(paragraphLines.size() > 0) {
            // Description and usage in two columns.
            for(size_t i = 0; i < paragraphLines.size(); i++) {
                out << (i == 0 ? usageWithSpaces : spacesPart) << paragraphLines[i] << std::endl;
            }
        } else {
            // No description. Just dump the usage part.
            out << usagePart << std::endl;
        }
    }


    return out.str();
}

inline std::string CommandlineParser::getHelpText(size_t columns)
{
    std::ostringstream out;

    if(usageString.size()) {
        out << getUsageText() << std::endl << std::endl;
    }

    if(versionString.size()) {
        out << getVersionText() << std::endl << std::endl;
    }

    if(documentationHeader.size()) {
        out << wrapParagraphs(documentationHeader, columns) << std::endl;
    }

    if(handlerWrappers.size()) {

        out << "Options:" << std::endl << std::endl;

        for(auto i = handlerWrappers.begin(); i != handlerWrappers.end(); i++) {

            if(i->first.size()) {
                ParameterDoc &docblock = parameterDocumentation[i->first];
                out << formatBlock(
                    i->first,
                    docblock.valueName,
                    docblock.doc,
                    columns);
            }

        }
    }

    if(documentationFooter.size()) {
        out << std::endl << wrapParagraphs(documentationFooter, columns);
    }

    return out.str();
}

inline std::string CommandlineParser::getVersionText()
{
    return versionString;
}

inline std::string CommandlineParser::getUsageText(size_t columns)
{
    std::ostringstream usageOstr;
    usageOstr << "Usage: " << argv0 << " " << usageString;
    return ExPop::stringIndent(
        ExPop::stringWordWrap(usageOstr.str(), columns, columns - 4),
        0, 4);

}

inline void CommandlineParser::setParameterDoc(
    const std::string &paramName,
    const std::string &doc,
    const std::string &valueName)
{
    ParameterDoc &docblock = parameterDocumentation[paramName];

    docblock.doc = doc;
    docblock.valueName = valueName;

    // Catch adding documentation for something that doesn't exist.
    assert(handlerWrappers[paramName].size());

    // Catch adding a value name for something that does not take a
    // value, or NOT adding a value name for something that does.
    assert(handlerWrappers[paramName][0]->isZeroParam() == !valueName.size());
}

inline void CommandlineParser::setDoc(
    const std::string &inVersionString,
    const std::string &inUsageString,
    const std::string &header,
    const std::string &footer)
{
    documentationHeader = header;
    documentationFooter = footer;
    versionString = inVersionString;
    usageString = inUsageString;
}



int main(int argc, char *argv[])
{
    CommandlineParser cmdParser(argv[0]);

    cmdParser.addHandler<std::string>(
        "cheese",
        [](const std::string &foo) {
            std::cout << foo << std::endl;
        });

    cmdParser.setParameterAlias("cheese", "butter");

    cmdParser.addHandler(
        "butts", []()
        {
            std::cout << "DICKBUTT" << std::endl;
        });

    cmdParser.addHandler<std::string>(
        "", [](const std::string &foo)
        {
            std::cout << "Empty shit: " << foo << std::endl;
        });

    cmdParser.addHandler<int>(
        "", [](int foo)
        {
            std::cout << "Empty shit2: " << foo << std::endl;
        });

    std::string theShit1 = "";
    bool theShit2 = false;

    cmdParser.addVariableHandler("s", &theShit1);
    cmdParser.addFlagHandler("shit2", &theShit2);

    cmdParser.setDoc(
        "Herp derp v1.0",
        "[options]",
        R"( This is just a boring test nobody cares about. This is just a boring
test nobody cares about. This is just a boring test nobody cares
about. This is just a boring test nobody cares about. This is just a
boring test nobody cares about. This is just a boring test nobody cares
about. This is just a boring test nobody cares about. )",
        R"(Copyright me or whatever, I guess.)");

    cmdParser.setParameterDoc("butts",  "Thing you stick in your butt.");
    cmdParser.setParameterDoc("s",      "Hurf blurf.", "stuff");
    cmdParser.setParameterDoc("shit2",  "Beep boop");
    cmdParser.setParameterDoc("cheese", "Say something on the console.", "stuff");


    if(!cmdParser.handleCommandline(argc, argv)) {
        return cmdParser.getErrorFlag();
    }

    return 0;
}



