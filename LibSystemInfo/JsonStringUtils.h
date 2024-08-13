#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <json.hpp>

#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>  // for getcwd
#include <limits.h>  // for PATH_MAX
#endif

std::string Base64Encode(const std::string& input);
std::string Base64Decode(const std::string& input);
std::string CleanJsonString(const std::string& input);
char* Sting2Chars(const std::string& str);
nlohmann::json CleanJson(nlohmann::json jsonObj);
void WriteToEnvFile(const std::string& base64String);
void WriteJsonToExecutableDir(const nlohmann::json& jsonData);
std::map<std::string, std::string> ParseEnvFile(const std::string& envFilePath);