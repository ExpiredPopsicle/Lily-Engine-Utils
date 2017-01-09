# Run this Makefile to regenerate graphicalconsole_fontimage.h after
# modifying the source TGA file.

CXXFLAGS := \
	-Wall \
	-g \
	--std=c++11

LIBS := \
	-g \
	-lz

HEADERS := \
	graphicalconsole.h \
	textrendering.h \
	graphicalconsole_fontimage.h

all : ../graphicalconsole_fontimage.h

fontembedtool : fontembedtool.cpp
	$(CXX) $(CXXFLAGS) $(filter %.cpp,$^) $(LIBS) -o $@

../graphicalconsole_fontimage.h : fontembedtool bitmapfont2.tga graphicalconsole_fontimage_template.h
	./fontembedtool

.INTERMEDIATE: fontembedtool


