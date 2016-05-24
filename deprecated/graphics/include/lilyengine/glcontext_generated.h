// Auto-generated GL function pointer declarations for GLContext.
// (Just #include this inside the body of the GLContext class.)
// Buffer related stuff.
void (EXPOP_GL_API *glGenBuffers)(GLsizei, GLuint*);
void (EXPOP_GL_API *glBindBuffer)(GLenum, GLuint);
void (EXPOP_GL_API *glBufferData)(GLenum, ptrdiff_t, const GLvoid*, GLenum);
void (EXPOP_GL_API *glEnableVertexAttribArray)(GLuint);
void (EXPOP_GL_API *glDisableVertexAttribArray)(GLuint);
void (EXPOP_GL_API *glDrawElements)(GLenum, GLsizei, GLenum, GLvoid*);

// Shader related stuff.
void (EXPOP_GL_API *glAttachShader)(GLuint, GLuint);
void (EXPOP_GL_API *glLinkProgram)(GLuint);
void (EXPOP_GL_API *glUseProgram)(GLuint);
GLuint (EXPOP_GL_API *glCreateShader)(GLenum);
GLuint (EXPOP_GL_API *glCreateProgram)(void);
void (EXPOP_GL_API *glCompileShader)(GLuint);
void (EXPOP_GL_API *glShaderSource)(GLuint, GLsizei, const char**, const GLint*);
void (EXPOP_GL_API *glGetShaderiv)(GLuint, GLenum, GLint*);
void (EXPOP_GL_API *glGetProgramiv)(GLuint, GLenum, GLint*);
GLint (EXPOP_GL_API *glGetUniformLocation)(GLuint, const char*);
GLint (EXPOP_GL_API *glGetAttribLocation)(GLuint, const char*);
void (EXPOP_GL_API *glBindAttribLocation)(GLuint, GLuint, const char*);
void (EXPOP_GL_API *glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, char*);
void (EXPOP_GL_API *glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, char*);
void (EXPOP_GL_API *glDeleteProgram)(GLuint);
void (EXPOP_GL_API *glDeleteShader)(GLuint);

// Shader inputs.
void (EXPOP_GL_API *glUniform1f)(GLint, GLfloat);
void (EXPOP_GL_API *glUniform2f)(GLint, GLfloat, GLfloat);
void (EXPOP_GL_API *glUniform3f)(GLint, GLfloat, GLfloat, GLfloat);
void (EXPOP_GL_API *glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);

void (EXPOP_GL_API *glUniform1i)(GLint, GLint);
void (EXPOP_GL_API *glUniform2i)(GLint, GLint, GLint);
void (EXPOP_GL_API *glUniform3i)(GLint, GLint, GLint, GLint);
void (EXPOP_GL_API *glUniform4i)(GLint, GLint, GLint, GLint, GLint);

void (EXPOP_GL_API *glUniform1fv)(GLint, GLsizei, const GLfloat*);
void (EXPOP_GL_API *glUniform2fv)(GLint, GLsizei, const GLfloat*);
void (EXPOP_GL_API *glUniform3fv)(GLint, GLsizei, const GLfloat*);
void (EXPOP_GL_API *glUniform4fv)(GLint, GLsizei, const GLfloat*);

void (EXPOP_GL_API *glUniform1iv)(GLint, GLsizei, const GLint*);
void (EXPOP_GL_API *glUniform2iv)(GLint, GLsizei, const GLint*);
void (EXPOP_GL_API *glUniform3iv)(GLint, GLsizei, const GLint*);
void (EXPOP_GL_API *glUniform4iv)(GLint, GLsizei, const GLint*);

void (EXPOP_GL_API *glUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat*);
void (EXPOP_GL_API *glUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat*);
void (EXPOP_GL_API *glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*);
void (EXPOP_GL_API *glUniformMatrix2x3fv)(GLint, GLsizei, GLboolean, const GLfloat*);
void (EXPOP_GL_API *glUniformMatrix3x2fv)(GLint, GLsizei, GLboolean, const GLfloat*);
void (EXPOP_GL_API *glUniformMatrix2x4fv)(GLint, GLsizei, GLboolean, const GLfloat*);
void (EXPOP_GL_API *glUniformMatrix4x2fv)(GLint, GLsizei, GLboolean, const GLfloat*);
void (EXPOP_GL_API *glUniformMatrix3x4fv)(GLint, GLsizei, GLboolean, const GLfloat*);
void (EXPOP_GL_API *glUniformMatrix4x3fv)(GLint, GLsizei, GLboolean, const GLfloat*);

void (EXPOP_GL_API *glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*);

// Normal OpenGL stuff that's probably already present.
void (EXPOP_GL_API *glEnable)(GLenum);
void (EXPOP_GL_API *glDisable)(GLenum);
void (EXPOP_GL_API *glBlendFunc)(GLenum, GLenum);
void (EXPOP_GL_API *glBlendEquation)(GLenum);
void (EXPOP_GL_API *glBindTexture)(GLenum, GLuint);
void (EXPOP_GL_API *glActiveTexture)(GLenum);
void (EXPOP_GL_API *glDeleteTextures)(GLsizei, const GLuint*);
void (EXPOP_GL_API *glDeleteBuffers)(GLsizei, const GLuint*);

// FBO stuff
void (EXPOP_GL_API *glGenFramebuffers)(GLsizei, GLuint*);
void (EXPOP_GL_API *glDeleteFramebuffers)(GLsizei, const GLuint*);
void (EXPOP_GL_API *glBindFramebuffer)(GLenum, GLuint);

void (EXPOP_GL_API *glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint);


// Renderbuffers
void (EXPOP_GL_API *glGenRenderbuffers)(GLsizei, GLuint*);
void (EXPOP_GL_API *glDeleteRenderbuffers)(GLsizei, const GLuint*);
void (EXPOP_GL_API *glBindRenderbuffer)(GLenum, GLuint);
void (EXPOP_GL_API *glRenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei);
void (EXPOP_GL_API *glGetRenderbufferParameteriv)(GLenum, GLenum, GLint*);

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif
