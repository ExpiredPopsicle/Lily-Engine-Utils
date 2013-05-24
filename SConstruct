env = Environment();

env.VariantDir("build/", "./");

lilyUtilsAppend = env.SConscript("build/utils/SConstruct");
lilyGraphicsAppend = env.SConscript("build/graphics/SConstruct", exports="lilyUtilsAppend");

env.SConscript("build/tools/SConstruct", exports="lilyUtilsAppend");

# Make a single thing that we can append to an environment for
# graphics and utils together.
lilyLib = {};
for k,v in lilyGraphicsAppend.items():
    if not(k in lilyLib):
        lilyLib[k] = [];
    lilyLib[k] = lilyLib[k] + [v];
for k,v in lilyUtilsAppend.items():
    if not(k in lilyLib):
        lilyLib[k] = [];
    lilyLib[k] = lilyLib[k] + [v];

env.Clean(".", "build");

Return("lilyLib");

