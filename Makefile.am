
noinst_LIBRARIES = liblilyengine_utils.a

noinst_HEADERS =
	src/thread/thread.h \
	src/assetloader/assetloader.h \
	src/math/matrix.h \
	src/math/mathdefs.h \
	src/assert/malassert.h \
	src/hashtable/hashtable.h \
	src/string/malstring.h \
	src/utils.h \
	src/preprocess/preprocess.h \
	src/console/console.h \
	src/parser/lilyparser.h \
	src/common/winhacks.h \
	src/refsystem/refsystem.h \
	src/compress/compress.h \
	src/filesystem/filesystem.h \
	src/filesystem/archive.h

liblilyengine_utils_a_SOURCES = \
	src/thread/thread.cpp \
	src/assetloader/assetloader.cpp \
	src/math/matrix.cpp \
	src/assert/malassert.cpp \
	src/string/string.cpp \
	src/preprocess/preprocess.cpp \
	src/console/console.cpp \
	src/parser/parserxml.cpp \
	src/parser/parser.cpp \
	src/refsystem/refsystem.cpp \
	src/compress/compress.cpp \
	src/filesystem/filesystem.cpp \
	src/filesystem/archive.cpp

noinst_PROGRAMS = \
	test_filesystem \
	test_compress \
	test_parser \
	test_thread

test_filesystem_SOURCES = src/filesystem/_test_filesystem.cpp
test_filesystem_LDADD = liblilyengine_utils.a

test_compress_SOURCES = src/compress/_test_compress.cpp
test_compress_LDADD = liblilyengine_utils.a

test_parser_SOURCES = src/parser/_test_parser.cpp
test_parser_LDADD = liblilyengine_utils.a

test_thread_SOURCES = src/thread/_test_thread.cpp
test_thread_LDADD = liblilyengine_utils.a


