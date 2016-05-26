Usage: $0 [-f] [-o] <source files> <org mode file>

ExpiredPopsicle's TODO/FIXME Scanner and Issue Tracker

This tool helps track TODO and FIXME comments in your C/C++ code by
assigning them issue numbers and consolidating them into org-mode
files.

This tool will modify your source, and generall expects everything to
be checked into version control already. It has safety features that
may be overridden to ensure that it's not modifying a file that may
not yet be checked in, or has modifications waiting to be checked in.
It is hardcoded to use Git for these purposes.

The org-mode file is both an input and an output. Notes left on issues
in the org-mode file will be preserved. Issues that are no longer
found (because their respective comment has been deleted) will be
marked as DONE. Modifications will be made to the source code to tag
issues with unique ID numbers.

Options:

  -f                Force writing files, even if they are not in any
                    version control.
  -o                Just write everything to stdout instead of any
                    files.
