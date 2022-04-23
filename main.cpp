#include <iostream>
#include <debugger.h>
#include <initialization_file_parser.h>
#include <string>


int main(int argc, char *argv[])
{
    // get path to run
    if (argc != 2)
    {
        std::cout << "Need to specify exactly 1 argument" << std::endl;
        return 1;
    }

    std::string pathToRun(argv[1]);

    // get cmake exe
    cmake_debugger::InitializationFileParser parser("/home/mikhail/CLionProjects/cmake-debugger/cmake-build-debug/config/config.ini", "cmake_debugger");
    const bool parseSuccess = parser.parse();
    if (!parseSuccess)
    {
        std::cout << "Need to specify cmake exe in config.ini file" << std::endl;
        return 1;
    }

    std::string cmakeExe = parser.getProperty("cmake_exe");
    if (cmakeExe.empty())
    {
        std::cout << "Need to specify cmake exe in config.ini file" << std::endl;
        return 1;
    }


    cmake_debugger::Debugger debugger(std::move(cmakeExe), std::move(pathToRun));
    debugger.run();

    return 0;
}
