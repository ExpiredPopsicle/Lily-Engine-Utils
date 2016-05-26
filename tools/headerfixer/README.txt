Usage: $0 <source files>

ExpiredPopsicle's Header Fixer Tool 1.1

Automate fixing copyright notices in lots of C/C++ source and header
files.

Use "--" as an input filename if you want the built-in, default
header.

Options:

  --help            You're sitting in it.
  --dumpheader      Write the built-in, default header to standard
                    output and quit.
  --header <file>   Use a file as the header text.
  --prefix <text>   Prefix each line with text instead of the default
                    "//" used for C++ files. Note that this alters the
                    search for the old header. Use --strip to remove
                    an old one before attempting to alter a prefix.
  --strip           Strip all headers.

Report bugs to expiredpopsicle@gmail.com
