#include <iostream>

// boost
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/containers/string.hpp>

// cmake debugger
#include <shared_memory_state.h>


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


int main(int argc, char *argv[])
{
    std::cout << "Breakpoint met" << std::endl;
    if (argc < 2)
    {
        std::cout << "Need to specify at least 1 argument" << std::endl;
        return 1;
    }
    std::string state(argv[1]);

    std::unique_ptr<boost::interprocess::managed_shared_memory> pManagedSharedMemory;
    try
    {
        pManagedSharedMemory = std::make_unique<boost::interprocess::managed_shared_memory>(
                boost::interprocess::open_only,
                SHARED_MEMORY_NAME
        );
    }
    catch (const boost::interprocess::interprocess_exception& exception)
    {
        std::cout << "Exception on accessing shared memory: " << exception.what() << std::endl;
        return 1;
    }

    auto pState = pManagedSharedMemory->find<boost::interprocess::string>(SHARED_MEMORY_STATE_NAME).first;
    auto pUserInput = pManagedSharedMemory->find<boost::interprocess::string>(SHARED_MEMORY_USER_INPUT_NAME).first;
    auto pNeedToWaitForInput = pManagedSharedMemory->find<bool>(SHARED_MEMORY_NEED_TO_WAIT_FOR_INPUT_NAME).first;
    auto pMutex = pManagedSharedMemory->find<boost::interprocess::interprocess_mutex>(SHARED_MEMORY_MUTEX_NAME).first;
    bool needToWaitForInput = false;

    if (pState == nullptr || pUserInput == nullptr || pNeedToWaitForInput == nullptr || pMutex == nullptr)
    {
        std::cout << "Couldn't find state in shared memory" << std::endl;
        return 1;
    }

    // set state
    {
        boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> scopedLock(*pMutex);
        pState->assign(state);
        needToWaitForInput = *pNeedToWaitForInput;
    }

    std::cout << "Current state:\n" << state << std::endl<< "Type 'c' to continue" << std::endl
        << "Type 's' to make step" << std::endl;

    while (needToWaitForInput)
    {
        boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> scopedLock(*pMutex);
        if (!pUserInput->empty())
        {
            pUserInput->assign(std::string());
            if (*pUserInput == USER_INPUT_CONTINUE)
            {
                *pNeedToWaitForInput = false;
            }
            else if (*pUserInput == USER_INPUT_STEP)
            {
                *pNeedToWaitForInput = true;
            }

            break;
        }
    }

    return 0;
}


