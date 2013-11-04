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

#pragma once

#include <string>

#include <GL/gl.h>
#include <GL/glext.h>

#ifdef WIN32
#define EXPOP_GL_API __stdcall
#else
#define EXPOP_GL_API
#endif

namespace ExPop {

    namespace Gfx {

        // FIXME: Fix GLContext name to be something accurate.

        /// This class holds all the OpenGL functions that we have to
        /// query for. Yes, it's misnamed.
        class GLContext {
        public:

            /// The default constructor makes a GLContext for the current
            /// OpenGL context.
            GLContext(void);

          #include "glcontext_generated.h"

        private:

            void *getGLProcAddress(const std::string &funcName);
            void checkGetProcResult(const std::string &funcName, void *p);
            void initFunctions(void);

        };

        /// Generic GL error checking thing.
        void assertGL(const char *fileName, const char *functionName, unsigned int lineNum);
      #define EXPOP_ASSERT_GL() ExPop::Gfx::assertGL(__FILE__, __FUNCTION__, __LINE__)

    }

}

