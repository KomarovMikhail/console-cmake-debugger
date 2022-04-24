#pragma once

#include <string>
#include <memory>

#include <boost/interprocess/managed_shared_memory.hpp>


namespace cmake_debugger
{

class Debugger
{
public:
    Debugger(std::string&& debuggerExePath, std::string&& cmakeExePath, std::string&& pathToRun);
    ~Debugger();

    bool init();
    void run();

private:

    void preprocessFiles();
    static void preprocessCmakeFile(const std::string& filePath);

private:

    std::string m_debuggerExePath;
    std::string m_cmakeExePath;
    std::string m_pathToRun;

    std::unique_ptr<boost::interprocess::managed_shared_memory> m_pManagedSharedMemory;
};

} // namespace cmake_debugger

