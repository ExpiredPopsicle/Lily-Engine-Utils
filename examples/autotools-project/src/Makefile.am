# Simple test program.
bin_PROGRAMS = test
test_SOURCES = main.cpp
test_LDFLAGS=-static

# ----------------------------------------------------------------------
# Add Lily-Engine-Utils libs and flags in.
test_CXXFLAGS=$(LILY_CXXFLAGS)
test_LDADD=$(LILY_LIBS)
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
# Everything after here is just for testing and experimenting with
# autotools.

# Test data directory stuff.
data_DATA=main.cpp
pkgdata_DATA=main.cpp

# Testing out an idea to see if autotools really does use directory
# name syntax like this. Spoiler: It does.
testwhateverdir=$(prefix)/foo/bar
testwhatever_DATA=main.cpp

# Testing more directory stuff. It looks like we can install something
# right into the root of a prefix (or where ever we want, really).
# Should have interesting applications for game data locations?
othertestdir=$(prefix)
othertest_PROGRAMS=test2
test2_SOURCES=main.cpp
test2_LDFLAGS=-static
test2_CXXFLAGS=$(LILY_CXXFLAGS)
test2_LDADD=$(LILY_LIBS)

# One more test, to see if we can install something under the tree
# we're building in. It looks like we can, pretty easily.
onemoretestdir=$(abs_top_builddir)/prefix/junk
onemoretest_PROGRAMS=test3
test3_SOURCES=main.cpp
test3_LDFLAGS=-static
test3_CXXFLAGS=$(LILY_CXXFLAGS)
test3_LDADD=$(LILY_LIBS)



