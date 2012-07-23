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

# for fname in sourceFileList:
#     print fname;

# def recurseSubModules(ctx):
#     for sm in submoduleList:
#         ctx.recurse(sm);

def configure(ctx):
    print("Configuring main thing.");

    # makeSourceList(ctx);

    # for p in ctx.env.buildPlatforms:
    #     print("  Platform: " + p);

    if(not ("expopLibs") in ctx.env):
        ctx.env.expopLibs = [];

    ctx.env.expopLibs.append(ctx.path.relpath() + "/" + outLibName);

    # print("-----" + ctx.path.relpath());

    # print("  Blah: " + ctx.env['CC'][0]);
    # print("  Blah: " + ctx.env['DEST_PLATFORM']);

    if(not ctx.env["CC"]):
        ctx.env["CC"] = "gcc";
    if(not ctx.env["AR"]):
        ctx.env["AR"] = "ar";
    if(not ctx.env["CXX"]):
        ctx.env["CXX"] = "g++";
    if(not ctx.env["LD"]):
        ctx.env["LD"] = "ld";

    if(not ctx.env["NATIVE_CC"]):
        ctx.env["NATIVE_CC"] = "gcc";
    if(not ctx.env["NATIVE_AR"]):
        ctx.env["NATIVE_AR"] = "ar";
    if(not ctx.env["NATIVE_CXX"]):
        ctx.env["NATIVE_CXX"] = "g++";
    if(not ctx.env["NATIVE_LD"]):
        ctx.env["NATIVE_LD"] = "ld";

    # TODO: Make this specific to Linux, Mac, and Android builds.
    ctx.env['LIBS'] = "-lpthread";
    print("ASDGFASDFASDFASDF");

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
    print("*** " + outLibNode.abspath());

    for fname in sourceFileList:
        if(fname.endswith(".cpp") or fname.endswith(".c")):
            addBuildFile(ctx, fname);

    addBuildLibTarget(ctx, outLibName);

    # ctx(rule="g++ -c ${SRC} -o ${TGT}", target="out.o", source="src/everything.cpp");

