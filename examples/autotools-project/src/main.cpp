#include <lilyengine/utils.h>

#include <iostream>

#ifndef PACKAGE_TARNAME
#define PACKAGE_TARNAME backup_butts
#endif

int main(int argc, char *argv[])
{
    std::cout << "Holy shit it worked." << std::endl;

    std::cout << "My path is: " << ExPop::FileSystem::getExecutablePath(argv[0]) << std::endl;

    std::cout << "My data is in: "
              << ExPop::FileSystem::makeFullPath(
                  ExPop::FileSystem::getExecutablePath(argv[0]) + "/../share")
              << std::endl;

    std::cout << "My data might also be in: "
              << ExPop::FileSystem::makeFullPath(
                  ExPop::FileSystem::getExecutablePath(argv[0]) + "/../share/" PACKAGE_TARNAME)
              << std::endl;

    return 0;
}
