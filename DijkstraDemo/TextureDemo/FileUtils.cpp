#include "FileUtils.h"

std::vector<std::vector<int>> FileUtils::LoadCSVFile(const char *filename) {
	// Open file
	std::ifstream f;
	f.open(filename);
	if (f.fail()) {
		throw(std::ios_base::failure(std::string("Error opening file ") + std::string(filename)));
	}

	// Read file
	std::string line;
	std::vector<std::vector<int>> arr;
	while (std::getline(f, line)) {
		std::vector<int> vect;
		std::stringstream ss(line);

		for (int i; ss >> i;) {
			vect.push_back(i);
			if (ss.peek() == ',')
				ss.ignore();
		}
		arr.push_back(vect);
	}

	// Close file
	f.close();

	return arr;
}


std::string FileUtils::LoadTextFile(const char* filename) {
	// Open file
	std::ifstream f;
	f.open(filename);
	if (f.fail()) {
		throw(std::ios_base::failure(std::string("Error opening file ") + std::string(filename)));
	}

	// Read file
	std::string content;
	std::string line;
	while (std::getline(f, line)) {
		content += line + "\n";
	}

	// Close file
	f.close();

	return content;
}

