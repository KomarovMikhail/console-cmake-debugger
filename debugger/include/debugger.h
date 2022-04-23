#pragma once

#include <string>
#include <memory>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/string.hpp>


namespace cmake_debugger
{

class SharedMemoryState;

class Debugger
{
public:
    Debugger(std::string&& cmakeExePath, std::string&& pathToRun);
    ~Debugger();

    void run();

private:

    void preprocessFiles();
    static void preprocessCmakeFile(const std::string& filePath);

private:

    std::string m_cmakeExePath;
    std::string m_pathToRun;

    std::unique_ptr<boost::interprocess::managed_shared_memory> m_pManagedSharedMemory;

    boost::interprocess::string* m_pState = nullptr;
    boost::interprocess::string* m_pUserInput = nullptr;
    bool* m_pNeedToWaitForInput = nullptr;
    boost::interprocess::interprocess_mutex* m_pMutex = nullptr;
};

} // namespace cmake_debugger

