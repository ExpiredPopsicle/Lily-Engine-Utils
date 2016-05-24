// ---------------------------------------------------------------------------
//
//   Lily Engine alpha
//
//   Copyright (c) 2012 Clifford Jolly
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
#include <vector>
#include <cstring>
#include <sstream>
using namespace std;

#include <lilyengine/utils.h>
using namespace ExPop;

int main(int argc, char *argv[]) {

    if(argc < 4) {
        cout << "Usage: " << argv[0] << " <functions list file> <generated cpp file> <generated header file>" << endl;
        return 1;
    }

    const char *inFilename = argv[1];
    const char *outCppFilename = argv[2];
    const char *outHeaderFilename = argv[3];

    int len = 0;
    char *data = FileSystem::loadFile(inFilename, &len, true);

    if(!data) {
        cerr << "Couldn't load the list of functions." << endl;
        return 1;
    }

    vector<string> lines;
    stringTokenize(data, "\n", lines, true);

    delete[] data;

    ostringstream pointerDeclarations;
    pointerDeclarations << "// Auto-generated GL function pointer declarations for GLContext." << endl;
    pointerDeclarations << "// (Just #include this inside the body of the GLContext class.)" << endl;

    ostringstream setupCode;
    setupCode << "// Auto-generated GL function pointer setup for GLContext." << endl;
    setupCode << "// (Compile this on its own and link it with the project.)" << endl;
    setupCode << "#include <GL/gl.h>" << endl;
    setupCode << "#include <GL/glext.h>" << endl;
    setupCode << "#include <lilyengine/glcontext.h>" << endl;
    setupCode << "namespace ExPop {" << endl;
    setupCode << "    namespace Gfx {" << endl;
    setupCode << "        void GLContext::initFunctions(void) {" << endl;

    for(unsigned int i = 0; i < lines.size(); i++) {

        if(strStartsWith("//", lines[i])) {

            // Copy comments right over.
            pointerDeclarations << lines[i] << endl;
            setupCode << "            " << lines[i] << endl;

        } else{

            vector<string> tokens;
            stringTokenize(lines[i], ",", tokens);

            if(tokens.size() == 3 && tokens[0] == "DEF") {

                string prefix = "";
                pointerDeclarations << "#ifndef " << prefix << tokens[1] << endl;
                pointerDeclarations << "#define " << prefix << tokens[1] << " " << tokens[2] << endl;
                pointerDeclarations << "#endif" << endl;

            } else if(tokens.size() >= 2) {

                string returnType = tokens[0];
                string funcName = tokens[1];

                vector<string> paramTypes;
                paramTypes.insert(
                    paramTypes.end(),
                    tokens.begin() + 2,
                    tokens.end());

                ostringstream paramList;

                if(paramTypes.size()) {
                    for(unsigned int j = 0; j < paramTypes.size(); j++) {
                        paramList << paramTypes[j];
                        if(j + 1 < paramTypes.size()) {
                            paramList << ", ";
                        }
                    }
                } else {
                    paramList << "void";
                }

                // Pointer declaration for the header.

                pointerDeclarations << returnType << " (EXPOP_GL_API *" << funcName << ")(";
                pointerDeclarations << paramList.str() << ");" << endl;

                // Initialization code to get it from the library.

                setupCode << "            checkGetProcResult(\"" << funcName << "\", ";
                setupCode << "(void*)(" << funcName << " = (" <<
                    returnType << "(EXPOP_GL_API *)(" << paramList.str() << "))";
                setupCode << "getGLProcAddress(\"" << funcName << "\")));" << endl;

            } else {

                if(lines[i].size() == 0) {
                    setupCode << endl;
                    pointerDeclarations << endl;
                } else {
                    cerr << "Bad line " << i << ": " << lines[i] << endl;
                    return 1;
                }
            }
        }
    }

    setupCode << "        }" << endl;
    setupCode << "    }" << endl;
    setupCode << "}" << endl;

    // cout << pointerDeclarations.str() << endl;
    // cout << setupCode.str() << endl;

    string pointerDeclarationsStr = pointerDeclarations.str();
    string setupCodeStr = setupCode.str();

    FileSystem::saveFile(outHeaderFilename, pointerDeclarationsStr.c_str(), pointerDeclarationsStr.size());
    FileSystem::saveFile(outCppFilename,    setupCodeStr.c_str(), setupCodeStr.size());

    return 0;
}
