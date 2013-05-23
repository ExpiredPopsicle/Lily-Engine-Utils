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

#include <string>
#include <cassert>
#include <cstdlib>
#include <iostream>
using namespace std;

#include <lilyengine/utils.h>
using namespace ExPop::Console;

#include <GL/gl.h>

#if defined(_WIN32)
#include <windows.h>
#include <GL/glu.h>

#elif defined(__APPLE__)
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>

#else
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glu.h>
#endif

#include <lilyengine/glcontext.h>

namespace ExPop {

    namespace Gfx {

        static void *checkGlobalGL(const std::string &funcName) {
            if(funcName == "glEnable") return (void*)glEnable;
            if(funcName == "glDisable") return (void*)glDisable;
            if(funcName == "glBlendFunc") return (void*)glBlendFunc;
            return NULL;
        }

        GLContext::GLContext(void) {

            // out("info") << "Loading GL functions..." << endl;

            // checkGetProcResult(
            //     "glCreateProgram",
            //     (void*)(
            //         glCreateProgram =
            //         (GLuint(*)(void))
            //         getGLProcAddress("glCreateProgram")));

            initFunctions();

            // out("info") << "Done loading GL functions." << endl;

        }

        void *GLContext::getGLProcAddress(const std::string &funcName) {

            void *globalFunc = checkGlobalGL(funcName);
            if(globalFunc) return globalFunc;

          #if defined(_WIN32)

          // #error "Untested! Remove this line to try it anyway."
            return (void*)wglGetProcAddress(funcName.c_str());

          #elif defined(__APPLE__)

          #error "Untested! Remove this line to try it anyway."
            return (void*)glXGetProcAddressARB((const GLubyte*)funcName.c_str());

          #else

            return (void*)glXGetProcAddress((const GLubyte*)funcName.c_str());

          #endif

        }

        void GLContext::checkGetProcResult(const std::string &funcName, void *p) {

            string consoleId = "info";
            if(!p) consoleId = "error";

            // out(consoleId) << p << ": " << funcName << endl;

            // FIXME: Should this be fatal?
            assert(p);
        }

        void assertGL(const char *fileName, const char *functionName, unsigned int lineNum) {
            unsigned int error = glGetError();
            if(error != GL_NO_ERROR) {
                std::cout << "GL error (" << fileName << ":" << functionName << ":" << lineNum << "): " << gluErrorString(error) << std::endl;
                // std::assert(0);
                std::exit(1);
            }
        }
    }
}

