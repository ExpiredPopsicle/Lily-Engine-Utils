Usage: $0 [options] <filename> [variable]

ExpiredPopsicle's Static Buffer Header Generator 1.2

Useful for when you really want to embed a whole damn file into your
source code.

Output goes to standard output. Redirect to a file as needed.

Options:

  --help            You're sitting in it.
  --nonull          Default behavior is to add a null terminator
                    character to the end of the buffer in case it's
                    going to be used as a C-style string. This skips
                    that. Useful if you want to use sizeof() to get an
                    accurate size of the original buffer.

Report bugs to expiredpopsicle@gmail.com
