#pragma once

#include <string>


namespace cmake_debugger
{

class SharedMemoryState
{
public:

    void setState(const std::string& state);
    [[nodiscard]] std::string getState() const;

    void setUserInput(const std::string& userInput);
    [[nodiscard]] std::string getUserInput() const;

    void setNeedForWaitInput(bool needWaitForInput);
    [[nodiscard]] bool getNeedForWaitInput() const;

private:

    std::string m_state = "initial";
    std::string m_userInput;
    bool m_needWaitForInput = true;
};

} // namespace cmake_debugger
