#include <iostream>
#include <debugger.h>
#include <initialization_file_parser.h>
#include <string>
#include <filesystem>


int main(int argc, char *argv[])
{
    // get path to run
    if (argc != 2)
    {
        std::cout << "Need to specify exactly 1 argument" << std::endl;
        return 1;
    }

    std::string debuggerExe = std::filesystem::absolute((argv[0]));
    std::string debuggerExePath = debuggerExe.substr(0, debuggerExe.find_last_of('/') + 1);
    std::string pathToRun(argv[1]);

    // get cmake exe
    cmake_debugger::InitializationFileParser parser(debuggerExePath + "config/config.ini", "cmake_debugger");
    const bool parseSuccess = parser.parse();
    if (!parseSuccess)
    {
        std::cout << "Couldn't find config.ini file" << std::endl;
        return 1;
    }

    std::string cmakeExePath = parser.getProperty("cmake_exe");
    if (cmakeExePath.empty())
    {
        std::cout << "Need to specify 'cmake_exe' in config.ini file" << std::endl;
        return 1;
    }

    cmake_debugger::Debugger debugger(std::move(debuggerExePath), std::move(cmakeExePath), std::move(pathToRun));
    debugger.run();

    return 0;
}
