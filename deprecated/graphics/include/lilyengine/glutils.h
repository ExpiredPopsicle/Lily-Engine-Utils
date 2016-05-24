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

namespace ExPop {

    namespace Gfx {

        class GLContext;

        /// Link shaders and return the shader program.
        GLuint linkShaders(
            GLContext *glc,
            GLuint vertexShader,
            GLuint fragmentShader,
            std::string &errorOut);

        /// Compile a shader from a string.
        GLuint compileShader(
            GLContext *glc,
            const std::string &shaderText,
            GLuint shaderType,
            std::string &errorOut);

        /// Load a shader from a file, compile it, and return the
        /// shader number. shaderType is GL_VERTEX_SHADER or
        /// GL_FRAGMENT_SHADER.
        GLuint loadShaderFile(
            GLContext *glc,
            const std::string &fileName,
            GLuint shaderType,
            std::string &errorOut);
    }
}
