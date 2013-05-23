env = Environment();

env.VariantDir("build/", "./");

lilyUtilsAppend = env.SConscript("build/utils/SConstruct");
lilyGraphicsAppend = env.SConscript("build/graphics/SConstruct", exports="lilyUtilsAppend");

# lilyLib = dict(
#     lilyUtilsAppend.items() +
#     lilyGraphicsAppend.items());

lilyLib = {};

for k,v in lilyGraphicsAppend.items():
    if not(k in lilyLib):
        lilyLib[k] = [];
    lilyLib[k] = lilyLib[k] + [v];

for k,v in lilyUtilsAppend.items():
    if not(k in lilyLib):
        lilyLib[k] = [];
    lilyLib[k] = lilyLib[k] + [v];

for k,v in lilyLib.items():
    print k + "=" + str(v);

env.Clean(".", "build");

Return("lilyLib");

