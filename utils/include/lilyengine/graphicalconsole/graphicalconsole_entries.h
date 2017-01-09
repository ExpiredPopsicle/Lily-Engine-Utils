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

    inline GraphicalConsole::CommandOrVariableEntry_Base::CommandOrVariableEntry_Base()
    {
    }

    inline GraphicalConsole::CommandOrVariableEntry_Base::~CommandOrVariableEntry_Base()
    {
    }

    inline void GraphicalConsole::CommandOrVariableEntry_Base::execute(
        const std::vector<std::string> &arguments)
    {
    }

    inline std::string GraphicalConsole::CommandOrVariableEntry_Base::getUsageString() const
    {
        return "";
    }

    // ----------------------------------------------------------------------
    // The template-based CommandEntry callback stuff starts here. Be
    // warned that the template code here is somewhat fragile.

    template<typename ReturnType, typename ... ParameterTypes>
    inline GraphicalConsole::CommandEntry<ReturnType, ParameterTypes...>::CommandEntry()
    {
        useDefaultMissingArgs = false;
    }

    template<typename ReturnType, typename ... ParameterTypes>
    inline GraphicalConsole::CommandEntry<ReturnType, ParameterTypes...>::~CommandEntry()
    {
    }


    // Run something and return the result as a string.
    template<typename ReturnType>
    inline std::string graphicalConsoleRunAndStringifyResult(std::function<ReturnType()> func)
    {
        ReturnType ret = func();
        std::ostringstream ostr;
        ostr << ret;
        return ostr.str();
    }

    // void-specialized version. Just returns an empty string.
    template<>
    inline std::string graphicalConsoleRunAndStringifyResult<void>(std::function<void()> func)
    {
        func();
        return "";
    }

    enum GraphicalConsoleFunctionReturnType
    {
        GRAPHICALCONSOLE_FUNCTIONRETURN_NOERROR,
        GRAPHICALCONSOLE_FUNCTIONRETURN_BADPARAMETERCOUNT,
    };

    // End point.
    template<
        typename ReturnType,
        typename ... DecodedParams,
        typename FunctionType>
    inline std::string graphicalConsoleRunFunctionWithStringArgs(
        const std::vector<std::string> &arguments,
        bool useDefaultMissingArgs,
        FunctionType func,
        GraphicalConsoleFunctionReturnType &returnType,
        DecodedParams... doneParams)
    {
        // If we still have arguments left, then something didn't get
        // decoded.
        if(arguments.size()) {
            returnType = GRAPHICALCONSOLE_FUNCTIONRETURN_BADPARAMETERCOUNT;
            return "";
        }

        // We can't look at the void result from a void return type here,
        // so we need to pack it up into a std::bind function and send it
        // off to a template function with a specialzed version for
        // handling void return types.
        std::function<ReturnType()> boundFunc = std::bind(func, doneParams...);
        std::string retStr = graphicalConsoleRunAndStringifyResult<ReturnType>(boundFunc);

        returnType = GRAPHICALCONSOLE_FUNCTIONRETURN_NOERROR;

        return retStr;
    }

    // Recursive part.
    template<
        typename ReturnType,
        typename CurrentParam,
        typename ... EncodedParams,
        typename ... DecodedParams,
        typename FunctionType>
    inline std::string graphicalConsoleRunFunctionWithStringArgs(
        const std::vector<std::string> &arguments,
        bool useDefaultMissingArgs,
        FunctionType func,
        GraphicalConsoleFunctionReturnType &returnType,
        DecodedParams... doneParams)
    {
        // Determine actual type, in case this is a reference or
        // something. We'll need to be able to store our parsed version in
        // an actual instance.
        typedef typename std::decay<CurrentParam>::type RealParam;
        RealParam decodedValue;

        // Decode a parameter.
        if(arguments.size()) {
            decodedValue = graphicalConsoleDecodeParameter<RealParam>(arguments[0]);
        } else {
            if(useDefaultMissingArgs) {
                decodedValue = RealParam();
            } else {
                returnType = GRAPHICALCONSOLE_FUNCTIONRETURN_BADPARAMETERCOUNT;
                return "";
            }
        }

        // Shrink down the list.
        std::vector<std::string> smallerParamsList;
        if(arguments.size()) {
            smallerParamsList.insert(
                smallerParamsList.begin(), arguments.begin() + 1, arguments.end());
        }

        return graphicalConsoleRunFunctionWithStringArgs<ReturnType, EncodedParams...>(
            smallerParamsList, useDefaultMissingArgs,
            func, returnType, doneParams..., decodedValue);
    }

    template<
        typename ReturnType,
        typename ... ParamTypes,
        typename FunctionType>
    inline std::string graphicalConsoleRunFunctionWithStringArgsWrapper(
        const std::vector<std::string> &arguments,
        bool useDefaultMissingArgs,
        FunctionType func,
        GraphicalConsoleFunctionReturnType &returnType)
    {
        return graphicalConsoleRunFunctionWithStringArgs<
            ReturnType,
            ParamTypes...>(
                arguments,
                useDefaultMissingArgs,
                func,
                returnType);
    }

    template<typename ReturnType, typename ... ParameterTypes>
    inline void GraphicalConsole::CommandEntry<ReturnType, ParameterTypes...>::execute(
        const std::vector<std::string> &arguments)
    {
        GraphicalConsoleFunctionReturnType returnType = GRAPHICALCONSOLE_FUNCTIONRETURN_NOERROR;

        std::string result = graphicalConsoleRunFunctionWithStringArgsWrapper<ReturnType, ParameterTypes...>(
            arguments, useDefaultMissingArgs, func, returnType);

        if(returnType == GRAPHICALCONSOLE_FUNCTIONRETURN_BADPARAMETERCOUNT) {
            parentConsole->out << graphicalConsoleGetRedErrorText() << ": " << name << ": Bad parameter count." << std::endl;
            parentConsole->out << "Usage: " << name << getUsageString() << std::endl;
        }

        if(result.size()) {
            parentConsole->out << result << std::endl;
        }
    }

    // ----------------------------------------------------------------------
    // Help string generation.

    struct SpecialTypeJustToTerminateThisStupidVariadic
    {
        int FUCKCPLUSPLUS;
    };

    template<typename T, typename ... Types>
    inline std::string graphicalConsoleBuildUsageString()
    {
        return
            std::string(" <") +
            graphicalConsoleGetTypeDescription<typename std::decay<T>::type>() +
            std::string(">") + graphicalConsoleBuildUsageString<Types...>();
    }

    // Specialized end thingy.
    template<>
    inline std::string graphicalConsoleBuildUsageString<
        SpecialTypeJustToTerminateThisStupidVariadic,
        SpecialTypeJustToTerminateThisStupidVariadic>()
    {
        return "";
    }

    template<typename ReturnType, typename ... ParameterTypes>
    inline std::string GraphicalConsole::CommandEntry<ReturnType, ParameterTypes...>::getUsageString() const
    {
        return graphicalConsoleBuildUsageString<
            ParameterTypes...,
            SpecialTypeJustToTerminateThisStupidVariadic,
            SpecialTypeJustToTerminateThisStupidVariadic>();
    }

    // ----------------------------------------------------------------------
    // Template-based CVar stuff. More straightforward than the command
    // stuff.

    template<typename T>
    inline GraphicalConsole::VariableEntry<T>::VariableEntry()
    {
        value = nullptr;
    }

    template<typename T>
    inline GraphicalConsole::VariableEntry<T>::~VariableEntry()
    {
    }

    template<typename T>
    inline std::string GraphicalConsole::VariableEntry<T>::getUsageString() const
    {
        return " <" + graphicalConsoleGetTypeDescription<T>() + ">";
    }

    template<typename T>
    inline void GraphicalConsole::VariableEntry<T>::execute(
        const std::vector<std::string> &arguments)
    {
        if(arguments.size() == 0) {
            parentConsole->out << *value << std::endl;
        } else if(arguments.size() == 1) {
            *value = graphicalConsoleDecodeParameter<T>(arguments[0]);
        } else {
            parentConsole->out << graphicalConsoleGetRedErrorText() << ": Too many arguments given." << std::endl;
            parentConsole->out << "Usage: " << name << getUsageString() << std::endl;
        }
    }

    // ----------------------------------------------------------------------

    template<typename R, typename ... T>
    std::function<R(T...)> graphicalConsoleWrapFunction(R(*func)(T...))
    {
        return std::function<R(T...)>(func);
    }

    template<typename ReturnType, typename ... ParameterTypes>
    inline void GraphicalConsole::setCommand(
        const std::string &name,
        const std::string &doc,
        ReturnType(*callback)(ParameterTypes...),
        bool useDefaultMissingArgs)
    {
        setCommand(
            name, doc,
            graphicalConsoleWrapFunction(callback),
            useDefaultMissingArgs);
    }

    template<typename ReturnType, typename ... ParameterTypes>
    inline void GraphicalConsole::setCommand(
        const std::string &name,
        const std::string &doc,
        std::function<ReturnType(ParameterTypes...)> callback,
        bool useDefaultMissingArgs)
    {
        std::basic_string<uint32_t> utf32name = ExPop::stringUTF8ToUTF32(name);
        delete commandsAndCvars[utf32name];

        CommandEntry<ReturnType, ParameterTypes...> *newEntry =
            new CommandEntry<ReturnType, ParameterTypes...>;

        commandsAndCvars[utf32name] = newEntry;

        newEntry->name = name;
        newEntry->docString = doc;
        newEntry->type = CommandOrVariableEntry_Base::COMMAND;
        newEntry->parentConsole = this;

        newEntry->useDefaultMissingArgs = useDefaultMissingArgs;
        newEntry->func = callback;
    }

    template<typename T>
    inline void GraphicalConsole::setCVar(
        const std::string &name,
        const std::string &doc,
        T *value)
    {
        std::basic_string<uint32_t> utf32name = ExPop::stringUTF8ToUTF32(name);
        delete commandsAndCvars[utf32name];

        VariableEntry<T> *newEntry =
            new VariableEntry<T>;

        commandsAndCvars[utf32name] = newEntry;

        newEntry->name = name;
        newEntry->docString = doc;
        newEntry->type = CommandOrVariableEntry_Base::CVAR;
        newEntry->parentConsole = this;

        newEntry->value = value;
    }

    inline void GraphicalConsole::clearCommandOrCVar(const std::string &name)
    {
        std::basic_string<uint32_t> utf32name = ExPop::stringUTF8ToUTF32(name);
        delete commandsAndCvars[utf32name];
        commandsAndCvars.erase(utf32name);
    }

}
