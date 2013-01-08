
# noinst_LIBRARIES = liblilyengine_utils.a
lib_LIBRARIES = liblilyengine_utils.a

liblilyengine_utils_a_includedir = $(includedir)/lilyengine

liblilyengine_utils_a_include_HEADERS = \
	include/thread.h \
	include/assetloader.h \
	include/matrix.h \
	include/mathdefs.h \
	include/malassert.h \
	include/hashtable.h \
	include/malstring.h \
	include/preprocess.h \
	include/console.h \
	include/lilyparser.h \
	include/winhacks.h \
	include/refsystem.h \
	include/compress.h \
	include/filesystem.h \
	include/archive.h \
	include/derpconfig.h \
	include/derperror.h \
	include/derpvm.h \
	include/derpobject.h \
	include/derpcontext.h \
	include/expopsockets.h \
	include/utils.h \
	include/pooledstring.h

liblilyengine_utils_a_SOURCES = \
	src/thread/thread.cpp \
	src/assetloader/assetloader.cpp \
	src/math/matrix.cpp \
	src/assert/malassert.cpp \
	src/string/string.cpp \
	src/string/pooledstring.cpp \
	src/preprocess/preprocess.cpp \
	src/console/console.cpp \
	src/parser/parserxml.cpp \
	src/parser/parser.cpp \
	src/refsystem/refsystem.cpp \
	src/compress/compress.cpp \
	src/filesystem/filesystem.cpp \
	src/filesystem/archive.cpp \
	src/derpscript/derpcontext.cpp \
	src/derpscript/derperror.cpp \
	src/derpscript/derpexecnode_binarymath.cpp \
	src/derpscript/derpexecnode.cpp \
	src/derpscript/derpexecnode_function.cpp \
	src/derpscript/derpexecnode_loop.cpp \
	src/derpscript/derpexecnode_unarymath.cpp \
	src/derpscript/derplexer.cpp \
	src/derpscript/derpobject.cpp \
	src/derpscript/derpparser_block.cpp \
	src/derpscript/derpparser.cpp \
	src/derpscript/derpparser_debug.cpp \
	src/derpscript/derpparser_expression.cpp \
	src/derpscript/derpparser_functioncall.cpp \
	src/derpscript/derpparser_function.cpp \
	src/derpscript/derpparser_ifelse.cpp \
	src/derpscript/derpparser_index.cpp \
	src/derpscript/derpparser_loops.cpp \
	src/derpscript/derpparser_statement.cpp \
	src/derpscript/derpparser_value.cpp \
	src/derpscript/derpparser_variabledec.cpp \
	src/derpscript/derpvm_builtinfunctions.cpp \
	src/derpscript/derpvm.cpp \
	src/sockets/sockets.cpp

noinst_HEADERS = \
	src/derpscript/derpexecnode.h \
	src/derpscript/derpexecnode_internal.h \
	src/derpscript/derperror_internal.h \
	src/derpscript/derplexer.h \
	src/derpscript/derpparser.h \
	src/derpscript/derpparser_internal.h \
	src/derpscript/derpvm_builtinfunctions.h

noinst_PROGRAMS = \
	test_filesystem \
	test_compress \
	test_parser \
	test_thread \
	test_derpscript \
	test_sockets

test_filesystem_SOURCES = src/filesystem/_test_filesystem.cpp
test_filesystem_LDADD = liblilyengine_utils.a $(EXTRA_LDADD)

test_compress_SOURCES = src/compress/_test_compress.cpp
test_compress_LDADD = liblilyengine_utils.a $(EXTRA_LDADD)

test_parser_SOURCES = src/parser/_test_parser.cpp
test_parser_LDADD = liblilyengine_utils.a $(EXTRA_LDADD)

test_thread_SOURCES = src/thread/_test_thread.cpp
test_thread_LDADD = liblilyengine_utils.a $(EXTRA_LDADD)

test_derpscript_SOURCES = src/derpscript/_test_derpscript.cpp
test_derpscript_LDADD = liblilyengine_utils.a $(EXTRA_LDADD)

test_sockets_SOURCES = src/sockets/_test_sockets.cpp
test_sockets_LDADD = liblilyengine_utils.a $(EXTRA_LDADD)

AM_CPPFLAGS = \
	-I$(srcdir)/src/string \
	-I$(srcdir)/include

EXTRA_DIST = Doxyfile

