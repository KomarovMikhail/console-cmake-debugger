// header
#include <initialization_file_parser.h>

// std
#include <fstream>
#include <iostream>


namespace cmake_debugger
{

InitializationFileParser::InitializationFileParser(std::string&& filePath, std::string&& sectionName)
    : m_filePath(std::move(filePath))
    , m_sectionName(std::move(sectionName))
{
}

bool InitializationFileParser::parse()
{
    // TODO file check and section check

    std::ifstream stream(m_filePath);
    std::string line;
    std::string sectionWithBraces = "[" + m_sectionName + "]";
    std::string delimiter = "=";

    bool startSection = false;

    while (std::getline(stream, line))
    {
        if (sectionWithBraces == line)
        {
            startSection = true;
            continue;
        }

        if (startSection)
        {
            if (line.empty())
            {
                break;
            }

            auto delimiterPos = line.find(delimiter);
            if (delimiterPos == std::string::npos)
            {
                break;
            }

            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1, line.size());

            m_map.emplace(std::move(key), std::move(value));
        }
    }

    stream.close();

    return true;
}

std::string InitializationFileParser::getProperty(const std::string& propertyName)
{
    auto iter = m_map.find(propertyName);
    if (iter == m_map.end())
    {
        return std::string();
    }

    return iter->second;
}

} // namespace cmake_debugger

