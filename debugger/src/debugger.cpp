// header
#include <debugger.h>

// std
#include <iostream>
#include <filesystem>
#include <stack>
#include <fstream>
#include <algorithm>
#include <regex>
#include <future>

// boost
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/containers/string.hpp>


namespace
{

constexpr char CMAKE_FILE_NAME[] = "CMakeLists.txt";
constexpr char BREAKPOINT_LABEL[] = "set\\(DEBUGGER_BREAKPOINT ON\\)";
constexpr char DEBUGGER_WORKDIR[] = "/workdir";

constexpr char SHARED_MEMORY_NAME[] = "cmake_debugger_shared_memory";
constexpr char SHARED_MEMORY_STATE_NAME[] = "shared_memory_state";
constexpr char SHARED_MEMORY_USER_INPUT_NAME[] = "shared_memory_user_input";
constexpr char SHARED_MEMORY_NEED_TO_WAIT_FOR_INPUT_NAME[] = "shared_memory_need_to_wait_fro_input";
constexpr char SHARED_MEMORY_MUTEX_NAME[] = "interprocess_mutex";

constexpr char CMAKE_UTILS_INCLUDE[] = "include(${CMAKE_SOURCE_DIR}/debugger_utils.cmake)\n";
constexpr char RUN_DEBUGGER_SUBPROCESS_CALL[] = ")\nrun_debugger_subprocess(\"\")";
constexpr char RUN_DEBUGGER_SUBPROCESS_BREAKPOINT_CALL[] = "run_debugger_subprocess(\"breakpoint\")";

constexpr char DEBUGGER_CMAKE_UTILS_PATH[] = "cmake_utils/debugger_utils.cmake";
constexpr char DEBUGGER_SUBPROCESS_PATH[] = "debugger_subprocess/debugger_subprocess";

constexpr int SHARED_MEMORY_SIZE = 1048576; // 1Mb;

constexpr char USER_INPUT_QUIT[] = "q";

}

using SharedMemoryObject = boost::interprocess::shared_memory_object;
using ManagedSharedMemory = boost::interprocess::managed_shared_memory;
template <typename T>
using SharedAllocator = boost::interprocess::allocator<T, ManagedSharedMemory::segment_manager>;
using SharedString = boost::interprocess::basic_string<char, std::char_traits<char>, SharedAllocator<char> >;
using SharedMutex = boost::interprocess::interprocess_mutex;
using SharedScopedLock = boost::interprocess::scoped_lock<SharedMutex>;

namespace cmake_debugger
{

Debugger::Debugger(std::string&& debuggerExePath, std::string&& cmakeExePath, std::string&& pathToRun)
    : m_debuggerExePath(std::move(debuggerExePath))
    , m_cmakeExePath(std::move(cmakeExePath))
    , m_pathToRun(std::move(pathToRun))
{
}

bool Debugger::init()
{
    bool success = true;

    std::filesystem::remove_all(m_pathToRun + DEBUGGER_WORKDIR);
    std::filesystem::create_directory(m_pathToRun + DEBUGGER_WORKDIR);

    // initialize shared memory
    SharedMemoryObject::remove(SHARED_MEMORY_NAME);

    try {
        m_pManagedSharedMemory = std::make_unique<ManagedSharedMemory>(
                boost::interprocess::open_or_create,
                SHARED_MEMORY_NAME,
                SHARED_MEMORY_SIZE
        );

        m_pManagedSharedMemory->construct<SharedString>(SHARED_MEMORY_STATE_NAME)(
            m_pManagedSharedMemory->get_segment_manager()
        );
        m_pManagedSharedMemory->construct<SharedString>(SHARED_MEMORY_USER_INPUT_NAME)(
            m_pManagedSharedMemory->get_segment_manager()
        );
        m_pManagedSharedMemory->construct<bool>(SHARED_MEMORY_NEED_TO_WAIT_FOR_INPUT_NAME)(false);
        m_pManagedSharedMemory->construct<SharedMutex>(SHARED_MEMORY_MUTEX_NAME)();
    }
    catch (const boost::interprocess::interprocess_exception& exception)
    {
        std::cout << "Failed to construct shared memory" << std::endl;
        success = false;
    }

    return success;
}

Debugger::~Debugger()
{
    std::filesystem::remove_all(m_pathToRun + DEBUGGER_WORKDIR);

    m_pManagedSharedMemory->destroy<SharedString>(SHARED_MEMORY_STATE_NAME);
    m_pManagedSharedMemory->destroy<SharedString>(SHARED_MEMORY_USER_INPUT_NAME);
    m_pManagedSharedMemory->destroy<bool>(SHARED_MEMORY_NEED_TO_WAIT_FOR_INPUT_NAME);
    m_pManagedSharedMemory->destroy<SharedMutex>(SHARED_MEMORY_MUTEX_NAME);

    SharedMemoryObject::remove(SHARED_MEMORY_NAME);
}
void Debugger::run()
{
    std::cout << "Starting cmake debugger on " << m_pathToRun << std::endl;

    preprocessFiles();

    std::cout << "Start debugging" << std::endl;

    auto pMutex = m_pManagedSharedMemory->find<SharedMutex>(SHARED_MEMORY_MUTEX_NAME).first;
    if (pMutex == nullptr)
    {
        std::cout << "Couldn't find mutex in shared memory" << std::endl;
        return;
    }

    auto future = std::async(std::launch::async, [this, pMutex]() -> int
    {
        auto command = m_cmakeExePath + " " + m_pathToRun + DEBUGGER_WORKDIR + " -B build -DDEBUGGER_SUBPROCESS_PATH=" +
            m_debuggerExePath + DEBUGGER_SUBPROCESS_PATH;
        const int code = std::system(command.c_str());
        if (code != 0)
        {
            std::cout << "Cmake finished with status code " << code << std::endl << "Last state:" << std::endl;

            SharedScopedLock scopedLock(*pMutex);
            auto pState = m_pManagedSharedMemory->find<SharedString>(SHARED_MEMORY_STATE_NAME).first;
            if (pState == nullptr)
            {
                std::cout << "Couldn't find state in shared memory" << std::endl;
            }

            std::cout << *pState << std::endl;
        }
        std::cout << "Debugging complete. Type 'q' to exit" << std::endl;
        return code;
    });

    while (true)
    {
        std::string userInput;
        std::cin >> userInput;

        SharedScopedLock scopedLock(*pMutex);
        auto pUserInput = m_pManagedSharedMemory->find<SharedString>(SHARED_MEMORY_USER_INPUT_NAME).first;
        if (pUserInput == nullptr)
        {
            std::cout << "Couldn't find user input in shared memory" << std::endl;
            break;
        }
        pUserInput->assign(userInput);

        auto futureStatus = future.wait_for(std::chrono::milliseconds(0));
        if (futureStatus == std::future_status::ready && userInput == USER_INPUT_QUIT)
        {
            break;
        }
    }

    future.wait();
}

void Debugger::preprocessFiles()
{
    std::cout << "Preparing to start debugging" << std::endl;
    // copying all project files
    std::filesystem::copy(m_pathToRun, m_pathToRun + DEBUGGER_WORKDIR, std::filesystem::copy_options::recursive);

    // copy cmake utils
    std::filesystem::copy(m_debuggerExePath + DEBUGGER_CMAKE_UTILS_PATH, m_pathToRun + DEBUGGER_WORKDIR);

    // dfs on project files
    std::stack<std::string> unprocessedDirs;
    unprocessedDirs.push(m_pathToRun + DEBUGGER_WORKDIR);

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

    auto fileContent = std::regex_replace(buffer.str(), std::regex(BREAKPOINT_LABEL), RUN_DEBUGGER_SUBPROCESS_BREAKPOINT_CALL);
    fileContent = std::regex_replace(fileContent, std::regex("\\)\\n"), RUN_DEBUGGER_SUBPROCESS_CALL);
    fileContent = CMAKE_UTILS_INCLUDE + fileContent;

    {
        std::ofstream ostream;
        ostream.open(filePath);
        ostream << fileContent;
        ostream.close();
    }
}

} //  namespace cmake_debugger