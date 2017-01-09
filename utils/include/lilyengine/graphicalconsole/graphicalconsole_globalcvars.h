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
    template<typename T>
    class GraphicalConsoleCVarSetter
    {
    public:

        inline GraphicalConsoleCVarSetter(
            const std::string &name,
            const std::string &doc,
            T *value)
        {
            getMainConsole()->setCVar<T>(name, doc, value);
            this->name = name;
        }

        // FIXME: Because we can't guarantee de-init order of globals,
        // we can't guarantee that the main console isn't torn down
        // already, so we're going to have to skip this. Don't use
        // GraphicalConsoleCVarSetter to hook up non-global variables,
        // or be prepared to tear them down yourself.

        // inline ~GraphicalConsoleCVarSetter()
        // {
        //     getMainConsole()->clearCommandOrCVar(name);
        // }

    private:
        std::string name;
    };

    template<typename T>
    class GraphicalConsoleCommandSetter
    {
    public:

        inline GraphicalConsoleCommandSetter(
            const std::string &name,
            const std::string &doc,
            T func)
        {
            getMainConsole()->setCommand(name, doc, func);
            this->name = name;
        }

        // FIXME: See GraphicalConsoleCVarSetter to see why we have no
        // destructor.

    private:
        std::string name;
    };

}

