#include <iostream>

// boost
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/containers/string.hpp>

#include <regex>


namespace
{

constexpr char SHARED_MEMORY_NAME[] = "cmake_debugger_shared_memory";
constexpr char SHARED_MEMORY_STATE_NAME[] = "shared_memory_state";
constexpr char SHARED_MEMORY_USER_INPUT_NAME[] = "shared_memory_user_input";
constexpr char SHARED_MEMORY_NEED_TO_WAIT_FOR_INPUT_NAME[] = "shared_memory_need_to_wait_fro_input";
constexpr char SHARED_MEMORY_MUTEX_NAME[] = "interprocess_mutex";

constexpr char USER_INPUT_CONTINUE[] = "c";
constexpr char USER_INPUT_STEP[] = "s";

}

using ManagedSharedMemory = boost::interprocess::managed_shared_memory;
template <typename T>
using SharedAllocator = boost::interprocess::allocator<T, ManagedSharedMemory::segment_manager>;
using SharedString = boost::interprocess::basic_string<char, std::char_traits<char>, SharedAllocator<char> >;
using SharedMutex = boost::interprocess::interprocess_mutex;
using SharedScopedLock = boost::interprocess::scoped_lock<SharedMutex>;


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Need to specify at least 1 argument" << std::endl;
        return 1;
    }
    std::string state(argv[1]);
    state = std::regex_replace(state, std::regex("\\|"), "\n");

    std::cout << state.size() << std::endl;

    bool isBreakpoint = false;
    if (argc == 3 && std::string(argv[2]) == "breakpoint")
    {
        std::cout << "Breakpoint met" << std::endl;
        isBreakpoint = true;

        std::cout << "Current state:\n" << state << std::endl<< "Type 'c' to continue" << std::endl
                  << "Type 's' to make step" << std::endl;
    }

    std::unique_ptr<ManagedSharedMemory> pManagedSharedMemory;
    try
    {
        pManagedSharedMemory = std::make_unique<ManagedSharedMemory>(
                boost::interprocess::open_only,
                SHARED_MEMORY_NAME
        );
    }
    catch (const boost::interprocess::interprocess_exception& exception)
    {
        std::cout << "Exception on accessing shared memory: " << exception.what() << std::endl;
        return 1;
    }

    auto pState = pManagedSharedMemory->find<SharedString>(SHARED_MEMORY_STATE_NAME).first;
    auto pUserInput = pManagedSharedMemory->find<SharedString>(SHARED_MEMORY_USER_INPUT_NAME).first;
    auto pNeedToWaitForInput = pManagedSharedMemory->find<bool>(SHARED_MEMORY_NEED_TO_WAIT_FOR_INPUT_NAME).first;
    auto pMutex = pManagedSharedMemory->find<SharedMutex>(SHARED_MEMORY_MUTEX_NAME).first;
    bool needToWaitForInput = false;

    if (pState == nullptr || pUserInput == nullptr || pNeedToWaitForInput == nullptr || pMutex == nullptr)
    {
        std::cout << "Couldn't find state in shared memory" << std::endl;
        return 1;
    }

    // set state
    {
        SharedScopedLock scopedLock(*pMutex);
        pState->assign(state);
        needToWaitForInput = isBreakpoint ? true : *pNeedToWaitForInput;
    }

    while (needToWaitForInput)
    {
        SharedScopedLock scopedLock(*pMutex);
        if (!pUserInput->empty())
        {
            auto userInput = *pUserInput;
            pUserInput->assign(std::string());
            if (userInput == USER_INPUT_CONTINUE)
            {
                *pNeedToWaitForInput = false;
                break;
            }
            else if (userInput == USER_INPUT_STEP)
            {
                *pNeedToWaitForInput = true;
                break;
            }

            std::cout << "Unknown command" << std::endl;
        }
    }

    return 0;
}


