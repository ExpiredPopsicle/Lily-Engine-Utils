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
#include <string>
using namespace std;

#include <GL/gl.h>
#include <GL/glext.h>
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif

#include <lilyengine/glutils.h>
#include <lilyengine/glcontext.h>
#include <lilyengine/filesystem.h>
#include <lilyengine/preprocess.h>

namespace ExPop {

    namespace Gfx {

        GLuint linkShaders(
            GLContext *glc,
            GLuint vertexShader,
            GLuint fragmentShader,
            std::string &errorOut) {

            if(!vertexShader || !fragmentShader) {
                errorOut += "No vertex shader or no fragment shader passed into linkShaders().\n";
                return 0;
            }

            if(errorOut.size()) return 0;

            GLuint program = glc->glCreateProgram();

            glc->glAttachShader(program, vertexShader);
            glc->glAttachShader(program, fragmentShader);

            glc->glLinkProgram(program);

            GLint linkStatus = 0;
            glc->glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if(linkStatus == GL_FALSE) {

                // Pull out the error message.
                GLint logLength = 0;
                glc->glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
                char *shaderLog = new char[logLength + 1];
                glc->glGetProgramInfoLog(program, logLength, NULL, shaderLog);
                shaderLog[logLength] = 0;

                errorOut = "Program link error:\n" + string(shaderLog);
                delete[] shaderLog;

                // Clean up.
                glc->glDeleteProgram(program);
                program = 0;
            }

            return program;
        }

        GLuint compileShader(
            GLContext *glc,
            const std::string &shaderText,
            GLuint shaderType,
            std::string &errorOut) {

            if(errorOut.size()) return 0;

            GLuint shader = glc->glCreateShader(shaderType);

            const char *cShaderSourceStr = shaderText.c_str();
            glc->glShaderSource(shader, 1, &cShaderSourceStr, NULL);
            glc->glCompileShader(shader);

            // Get compile status.
            GLint compileResult = 0;
            glc->glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
            if(compileResult == GL_FALSE) {

                // Pull out the error message.
                GLint logLength = 0;
                glc->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
                char *shaderLog = new char[logLength + 1];
                glc->glGetShaderInfoLog(shader, logLength, NULL, shaderLog);
                shaderLog[logLength] = 0;

                errorOut = "Shader compile error:\n" + string(shaderLog) + "\nShader text:\n" + shaderText;
                delete[] shaderLog;

                // Clean up.
                glc->glDeleteShader(shader);
                shader = 0;
            }

            return shader;
        }

        GLuint loadShaderFile(
            GLContext *glc,
            const std::string &fileName,
            GLuint shaderType,
            std::string &errorOut) {

            std::string inFile = FileSystem::loadFileString(fileName);
            if(!inFile.size()) {
                errorOut = string("Could not load file or file empty: ") + fileName;
                return 0;
            }

            PreprocessorState ppstate;
            string preprocessedStr = preprocess(fileName, inFile, ppstate);

            if(ppstate.hadError) {
                errorOut = ppstate.errorText;
                return 0;
            }

            return compileShader(
                glc, preprocessedStr,
                shaderType, errorOut);
        }
    }
}

