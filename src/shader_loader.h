#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

std::string loadShaderSource(const std::string& filePath)
{
    std::ifstream shaderFile;
    shaderFile.open(filePath);
    
    if (!shaderFile.is_open())
    {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }
    
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    
    return shaderStream.str();
}

