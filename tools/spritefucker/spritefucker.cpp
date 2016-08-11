#include <lilyengine/utils.h>

#include <string>

int main(int argc, char *argv[])
{
    // How to instantiate the object.
    ExPop::CommandlineParser cmdParser(argv[0]);

    // Simple handler that just writes the value to stdout.
    cmdParser.addHandler<std::string>(
        "cheese",
        [](const std::string &foo) { std::cout << foo << std::endl; });

    // Alias example, so --cheese and -f mean the same thing.
    cmdParser.setParameterAlias("cheese", "f");

    // Zero-parameter handler.
    cmdParser.addHandler(
        "butts",
        []() { std::cout << "Haha. You said butts." << std::endl; });

    // Zero-parameter handler that throws an error. Parsing of
    // parameters will stop after it returns from parsing this.
    cmdParser.addHandler(
        "giveerror",
        [&cmdParser]() { cmdParser.throwError("You asked for an error."); });

    // Handling of bare parameters with no -- or -. For example: "./a
    // thing1 thing2 etc".
    cmdParser.addHandler<std::string>(
        "",
        [](const std::string &foo){ std::cout << "Bare parameter: " << foo << std::endl; });

    // Multiple handlers can be attached to the same parameter. In
    // this case, the function to add it has a template parameter for
    // a type to pass to the handler. Conversion to this type is
    // handled automatically through overloaded ostream operators.
    cmdParser.addHandler<int>(
        "",
        [](int foo){ std::cout << "Bare parameter int: " << foo << std::endl; });

    // This is an example that will intentionally remain undocumented
    // as an example of how undocumented parameters show up in the
    // generated --help text.
    cmdParser.addHandler(
        "mystery",
        [](){ std::cout << "Mystery command discovered." << std::endl; });

    // For most cases, we might just want to feed the parameters
    // straight into variables. In this case, '-s "foo"' will fill
    // someValueForS with "foo".
    std::string someValueForS = "";
    cmdParser.addVariableHandler("s", &someValueForS);

    // An even simpler case can be used for booleans or integers,
    // where the mere presence of the parameter will turn on a boolean
    // flag. No extra parameter or value is required.
    bool someValueForDFlag = false;
    cmdParser.addFlagHandler("dflag", &someValueForDFlag);

    // Main documentation setup.
    cmdParser.setDoc(

        // Name and version. This is the output of --version, and also
        // part of the output of --help.
        "Example Program v1.0: Herp Derp Edition",

        // The usage text, without the executable. This will turn into
        // something like "./a.out [options]" in the usage text,
        // depending on argv[0]. This will be word-wrapped.
        "[options]",

        // The main documentation, before the descriptions of the
        // options. This will be word-wrapped.
        R"( This is just a boring test nobody cares about. This is just a boring
test nobody cares about. This is just a boring test nobody cares
about. This is just a boring test nobody cares about. This is just a
boring test nobody cares about. This is just a boring test nobody cares
about. This is just a boring test nobody cares about. )",

        // A footer to appear below the options. This will be
        // word-wrapped.
        R"(Copyright me or whatever, I guess.)");

    // This demonstrates how to document individual command line
    // parameters. The first parameter is the name, and must match the
    // name of the parameter. The second parameter is the
    // documentation, which will be automatically word-wrapped for the
    // --help text display. The last parameter is a nice name for the
    // value being set. The value name must not be empty if the
    // command takes parameters, and must be empty if it does not take
    // additional parameters.
    cmdParser.setParameterDoc("butts",     "Laughs at you.");
    cmdParser.setParameterDoc("giveerror", "Throws an error and quits.");
    cmdParser.setParameterDoc("s",         "Sets the variable someValueForS to stuff.",     "stuff");
    cmdParser.setParameterDoc("dflag",     "Sets the variable someValueForDFlag to true.");
    cmdParser.setParameterDoc("cheese",    "Say something on the console.",                 "stuff");

    // "--mystery" documentation intentionally omitted.

    // This is an easy example of firing off the parsing and reacting
    // to an error. Note that exiting early does not necessarily imply
    // an error, because --help and --version will ask for an early
    // exit too.
    if(!cmdParser.handleCommandline(argc, argv)) {
        return cmdParser.getErrorFlag();
    }

    return 0;
}



