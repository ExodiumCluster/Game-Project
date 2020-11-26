#pragma once

#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

class FileUtils {
public:
	static std::vector<std::vector<int>> LoadCSVFile(const char *filename);
	static std::string LoadTextFile(const char *filename);
};