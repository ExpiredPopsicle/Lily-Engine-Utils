dnl ----------------------------------------------------------------------
dnl   Lily-Engine-Utils macro file for Autotools
dnl ----------------------------------------------------------------------
dnl
dnl Include this file in your configure.ac and reference the LILY_*
dnl variables in your Makefile.am, to use Lily-Engine-Utils in a project.
dnl
dnl Example usage of this m4 file...
dnl
dnl configire.ac:
dnl
dnl   m4_include(<your_Lily-Engine-Utils_directory_here>/lilyutils.m4)
dnl
dnl Makefile.am:
dnl
dnl   projectname_CXXFLAGS=$(LILY_CXXFLAGS)
dnl   projectname_LDADD=$(LILY_LIBS)
dnl

dnl Get the directory that this m4 script is in, because the
dnl lilyutils-config script lives right next to it.
define(LILY_DIRNAME, `AS_DIRNAME(__file__)`)

# Get Lily-Engine-Utils libraries and flags.
#
# Note: CXX environment variable must be explicitly passed in for us
# to rely on the platform detection code inside lilyutils-config!
LILY_CXXFLAGS=$(CXX=${CXX} LILY_DIRNAME/lilyutils-config --enable-threads --cxxflags)
LILY_LIBS=$(CXX=${CXX} LILY_DIRNAME/lilyutils-config --enable-threads --libs)
AC_SUBST([LILY_CXXFLAGS])
AC_SUBST([LILY_LIBS])



