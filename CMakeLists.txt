cmake_minimum_required(VERSION 2.8)

project(Lily-Engine-Utils)

include_directories("${PROJECT_SOURCE_DIR}/include/lilyengine")

set(CMAKE_CXX_FLAGS "-O3 -ggdb -g -Wall")
add_library(Lily-Engine-Utils
  src/thread/thread.cpp
  src/assetloader/assetloader.cpp
  src/math/matrix.cpp
  src/assert/malassert.cpp
  src/sockets/sockets.cpp
  src/string/string.cpp
  src/string/pooledstring.cpp
  src/image/imagedxt.cpp
  src/image/image.cpp
  src/preprocess/preprocess.cpp
  src/console/console.cpp
  src/parser/parserxml.cpp
  src/parser/parser.cpp
  src/derpscript/derpvm.cpp
  src/derpscript/derpparser.cpp
  src/derpscript/derpparser_value.cpp
  src/derpscript/derpparser_function.cpp
  src/derpscript/derpcontext.cpp
  src/derpscript/derpvm_builtinfunctions.cpp
  src/derpscript/derplexer.cpp
  src/derpscript/derpexecnode.cpp
  src/derpscript/derpparser_block.cpp
  src/derpscript/derpparser_ifelse.cpp
  src/derpscript/derpobject.cpp
  src/derpscript/derpparser_variabledec.cpp
  src/derpscript/derpexecnode_function.cpp
  src/derpscript/derpexecnode_unarymath.cpp
  src/derpscript/derpexecnode_loop.cpp
  src/derpscript/derpparser_expression.cpp
  src/derpscript/derpparser_debug.cpp
  src/derpscript/derpexecnode_binarymath.cpp
  src/derpscript/derpparser_statement.cpp
  src/derpscript/derperror.cpp
  src/derpscript/derpparser_loops.cpp
  src/derpscript/derpparser_functioncall.cpp
  src/derpscript/derpparser_index.cpp
  src/refsystem/refsystem.cpp
  src/compress/compress.cpp
  src/filesystem/filesystem.cpp
  src/filesystem/archive.cpp
)
