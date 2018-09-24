// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2018 Kiri Jolly
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

    inline void GraphicalConsole::showHelp(const std::string &name)
    {
        if(!name.size()) {

            out << "Commands and variables available..." << std::endl;

            for(auto i = commandsAndCvars.begin(); i != commandsAndCvars.end(); i++) {
                out << "  " << ExPop::stringUTF32ToUTF8(i->first) << " ";
                out << ((i->second->type == GraphicalConsole::CommandOrVariableEntry_Base::CVAR) ? "(cvar)" : "(command)");
                out << std::endl;
            }

            out << "Use \"help <name>\" to show a specific entry." << std::endl;

        } else {

            auto i = commandsAndCvars.find(ExPop::stringUTF8ToUTF32(name));

            if(i != commandsAndCvars.end()) {

                out << ((i->second->type == GraphicalConsole::CommandOrVariableEntry_Base::CVAR) ? "Variable: " : "Command:  ");
                out << ExPop::stringUTF32ToUTF8(i->first);
                out << std::endl;

                out << "Doc:      " << i->second->docString << std::endl;

                out << "Usage:    ";
                out << ExPop::stringUTF32ToUTF8(i->first);
                out << i->second->getUsageString() << std::endl;

            } else {

                out << graphicalConsoleGetRedErrorText() << ": No command or variable named \"" << name << "\" found." << std::endl;

            }
        }
    }

    inline std::vector<std::string> GraphicalConsole::showHelpAutoCompleter()
    {
        std::vector<std::string> ret;
        for(auto i = commandsAndCvars.begin(); i != commandsAndCvars.end(); i++) {
            ret.push_back(ExPop::stringUTF32ToUTF8(i->first));
        }
        return ret;
    }

}
