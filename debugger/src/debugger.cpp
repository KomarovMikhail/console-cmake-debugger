// header
#include <debugger.h>

// std
#include <iostream>
#include <filesystem>
#include <stack>
#include <fstream>
#include <algorithm>
#include <regex>
#include <thread>

// boost
#include <boost/interprocess/sync/interprocess_mutex.hpp>


namespace
{

constexpr char CMAKE_FILE_NAME[] = "CMakeLists.txt";
constexpr char BREAKPOINT_LABEL[] = "set\\(DEBUGGER_BREAKPOINT ON\\)";
constexpr char DEBUGGER_WORKDIR[] = "workdir";

constexpr char SHARED_MEMORY_NAME[] = "cmake_debugger_shared_memory";
constexpr char SHARED_MEMORY_STATE_NAME[] = "shared_memory_state";
constexpr char SHARED_MEMORY_USER_INPUT_NAME[] = "shared_memory_user_input";
constexpr char SHARED_MEMORY_NEED_TO_WAIT_FOR_INPUT_NAME[] = "shared_memory_need_to_wait_fro_input";
constexpr char SHARED_MEMORY_MUTEX_NAME[] = "interprocess_mutex";

constexpr char CMAKE_UTILS_INCLUDE[] = "include(/home/mikhail/CLionProjects/cmake-debugger/cmake-build-debug/cmake_utils/debugger_utils.cmake)\n";
constexpr char BREAKPOINT_FUNCTION_CALL[] = "debugger_breakpoint_met()";

constexpr int SHARED_MEMORY_SIZE = 65536;

}

namespace cmake_debugger
{

Debugger::Debugger(std::string&& cmakeExePath, std::string&& pathToRun)
    : m_cmakeExePath(std::move(cmakeExePath))
    , m_pathToRun(std::move(pathToRun))
{
    std::filesystem::remove_all(DEBUGGER_WORKDIR);
    std::filesystem::create_directory(DEBUGGER_WORKDIR);

    // initialize shared memory
    boost::interprocess::shared_memory_object::remove(SHARED_MEMORY_NAME);
    m_pManagedSharedMemory = std::make_unique<boost::interprocess::managed_shared_memory>(
        boost::interprocess::open_or_create,
        SHARED_MEMORY_NAME,
        SHARED_MEMORY_SIZE
    );

    m_pState = m_pManagedSharedMemory->construct<boost::interprocess::string>(SHARED_MEMORY_STATE_NAME)();
    m_pUserInput = m_pManagedSharedMemory->construct<boost::interprocess::string>(SHARED_MEMORY_USER_INPUT_NAME)();
    m_pNeedToWaitForInput = m_pManagedSharedMemory->construct<bool>(SHARED_MEMORY_NEED_TO_WAIT_FOR_INPUT_NAME)(true);
    m_pMutex = m_pManagedSharedMemory->construct<boost::interprocess::interprocess_mutex>(SHARED_MEMORY_MUTEX_NAME)();
}

Debugger::~Debugger()
{
    std::filesystem::remove_all(DEBUGGER_WORKDIR);

    m_pManagedSharedMemory->destroy_ptr(m_pState);
    m_pManagedSharedMemory->destroy_ptr(m_pUserInput);
    m_pManagedSharedMemory->destroy_ptr(m_pNeedToWaitForInput);
    m_pManagedSharedMemory->destroy_ptr(m_pMutex);

    boost::interprocess::shared_memory_object::remove(SHARED_MEMORY_NAME);
}
void Debugger::run()
{
    std::cout << "Starting cmake debugger on " << m_pathToRun << std::endl;

    preprocessFiles();

    std::thread cmakeThread([this]()
    {
        std::system((m_cmakeExePath + " " + DEBUGGER_WORKDIR + " -B build").c_str());
        std::cout << "Debugging complete. Type 'q' to exit" << std::endl;
    });

    while (true)
    {
        std::string userInput;
        std::cin >> userInput;

        boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> scopedLock(*m_pMutex);
        m_pUserInput->assign(userInput);

        if (userInput == "q")
        {
            break;
        }
    }

    cmakeThread.join();
}

void Debugger::preprocessFiles()
{
    // copying all project files
    std::filesystem::copy(m_pathToRun, DEBUGGER_WORKDIR, std::filesystem::copy_options::recursive);

    // dfs on project files
    std::stack<std::string> unprocessedDirs;
    unprocessedDirs.push(DEBUGGER_WORKDIR);

    while (!unprocessedDirs.empty())
    {
        const auto currentDir = unprocessedDirs.top();
        unprocessedDirs.pop();

        for (const auto& iter : std::filesystem::directory_iterator(currentDir))
        {
            if (iter.is_directory())
            {
                unprocessedDirs.push(iter.path().string());
            }
            else if (iter.is_regular_file() && iter.path().filename() == CMAKE_FILE_NAME)
            {
                preprocessCmakeFile(iter.path().string());
            }
        }
    }
}

void Debugger::preprocessCmakeFile(const std::string& filePath)
{
    std::stringstream buffer;
    {
        std::ifstream istream;
        istream.open(filePath);;
        buffer << istream.rdbuf();
        istream.close();
    }

    auto fileContent = std::regex_replace(buffer.str(), std::regex(BREAKPOINT_LABEL), BREAKPOINT_FUNCTION_CALL);
    fileContent = CMAKE_UTILS_INCLUDE + fileContent;

    {
        std::ofstream ostream;
        ostream.open(filePath);
        ostream << fileContent;
        ostream.close();
    }
}

} //  namespace cmake_debugger