import buildcommon;

env = Environment();

env.VariantDir("build/", "./");

def addLibReturns(libRet):
    global lilyLib;
    lilyLib = buildcommon.merge(lilyLib, libRet);

lilyUtilsAppend = env.SConscript("build/utils/SConstruct");
lilyToolsAppend = env.SConscript("build/tools/SConstruct", exports="lilyUtilsAppend");
lilyGraphicsAppend = env.SConscript("build/graphics/SConstruct", exports=[
        "lilyUtilsAppend",
        "lilyToolsAppend"]);

# Make a single thing that we can append to an environment for
# graphics and utils together.
lilyLib = {};

addLibReturns(lilyGraphicsAppend);
addLibReturns(lilyUtilsAppend);
addLibReturns(lilyToolsAppend);

env.Clean(".", "build");

Return("lilyLib");

