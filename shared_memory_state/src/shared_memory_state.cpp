// header
#include <shared_memory_state.h>


namespace cmake_debugger
{

void SharedMemoryState::setState(const std::string& state)
{
    if (m_state != state)
    {
        m_state = state;
    }
}

std::string SharedMemoryState::getState() const
{
    return m_state;
}

void SharedMemoryState::setUserInput(const std::string& userInput)
{
    if (m_userInput != userInput)
    {
        m_userInput = userInput;
    }
}

[[nodiscard]] std::string SharedMemoryState::getUserInput() const
{
    return m_userInput;
}

void SharedMemoryState::setNeedForWaitInput(bool needWaitForInput)
{
    if (m_needWaitForInput != needWaitForInput)
    {
        m_needWaitForInput = needWaitForInput;
    }
}

[[nodiscard]] bool SharedMemoryState::getNeedForWaitInput() const
{
    return m_needWaitForInput;
}


} // namespace cmake_debugger

