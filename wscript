APPNAME="LilyEngineUtils"
VERSION="1.0"
outLibName = "liblilyutils.a";

top = ".";
out = "build";

import os;
import string;

sourceFileList = [];
obFileList = [];

# Ugly search thing to just find all the source files.
def makeSourceList(ctx):

    def buildSourceList(arg, dirname, names):

        global sourceFileList;

        for n in names:

            if(os.path.isfile(dirname + "/" + n)):

                skipThisFile = False;

                # Screen out testing junk.
                if(len(n)):
                    if(n[0] == '_'):
                        skipThisFile = True;

                # Screen out things that aren't source files.
                if(not (n.endswith(".cpp") or
                        n.endswith(".h") or
                        n.endswith(".c"))):

                    skipThisFile = True;

                if(not skipThisFile):
                    sourceFileList.append(dirname + "/" + n);
                    print("Adding source file: " + n);


    # I'm going to be lazy and just scan the directory for source
    # files. But these have to be relative to the base for the utils
    # library. Unfortunately, the script runs out of whatever base
    # directory the entire project is in.
    oldDir = os.getcwd();
    os.chdir(ctx.path.relpath());
    os.path.walk("src", buildSourceList, "");
    os.chdir(oldDir);

def configure(ctx):
    pass;

def addBuildFile(ctx, srcFileName):
    global obFileNames;
    outFileName = string.replace(srcFileName, ".cpp", ".o");
    ctx(rule="g++ -c ${SRC} -o ${TGT}", target=outFileName, source=srcFileName);
    obFileList.append(outFileName);

def addBuildLibTarget(ctx, outFileName):
    global obFileList;
    ctx(rule="ar -cvq ${TGT} ${SRC}", target=outFileName, source=string.join(obFileList, " "));

def build(ctx):

    global sourceFileList;
    global obFileList;
    global outLibName;

    makeSourceList(ctx);

    outLibNode = ctx.path.find_or_declare(outLibName);

    # Path to lib file is in outLibNode.abspath()

    for fname in sourceFileList:
        if(fname.endswith(".cpp") or fname.endswith(".c")):
            addBuildFile(ctx, fname);

    addBuildLibTarget(ctx, outLibName);

