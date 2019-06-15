#include <lilyengine/utils.h>

#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "My path is: "
              << ExPop::FileSystem::getExecutablePath(argv[0])
              << std::endl;
    return 0;
}
