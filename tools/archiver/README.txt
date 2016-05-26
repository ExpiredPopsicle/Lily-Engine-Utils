Usage: $0 [command] <archive filename> [command parameters]

ExpiredPopsicle's Archive Generator 1.0

Generates a file for use with the archive subsystem of
LilyEngineUtils. This is mostly for building archive files all at
once. As a result, adding or removing files from existing archives is
unsupported. Because the archives are intended to be consumed as data
for games using LilyEngineUtils, they aren't trivial to extract and
shouldn't be used for general file transfer or distribution.

No compression is applied to the files.

Attempting to use archives created with one version of LilyEngineUtils
with archives created with a different version of LilyEngineUtils may
result in scary behavior.

Options:

  --help            You're sitting in it.

Commands:

  -c                Create an archive. This will overwrite any
                    existing archive file. Command parameters are the
                    list of files to add to the new archive.
  -e                Extract files, from an archive. Command parameters
                    are the list of files to extract.
  -l                List files existing inside an archive.
  -d                Dump specified files to standard output. Command
                    parameters are the list of files to dump.

Report bugs to expiredpopsicle@gmail.com
