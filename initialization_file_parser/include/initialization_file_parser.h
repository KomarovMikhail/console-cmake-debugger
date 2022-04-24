#pragma once

#include <string>
#include <unordered_map>

namespace cmake_debugger
{

class InitializationFileParser
{
public:
    InitializationFileParser(std::string&& filePath, std::string&& sectionName);

    bool parse();
    std::string getProperty(const std::string& propertyName);

private:

    std::string m_filePath;
    std::string m_sectionName;
    std::unordered_map<std::string, std::string> m_map;
};

} // namespace cmake_debugger
