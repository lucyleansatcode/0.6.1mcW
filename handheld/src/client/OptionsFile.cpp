#include "OptionsFile.h"
#include <stdio.h>
#include <string.h>

std::string OptionsFile::_defaultSettingsPath;

OptionsFile::OptionsFile() {
    settingsPath = _defaultSettingsPath.empty() ? "options.txt" : _defaultSettingsPath;
}

void OptionsFile::setDefaultSettingsPath(const std::string& path) {
    _defaultSettingsPath = path;
}

void OptionsFile::save(const StringVector& settings) {
	FILE* pFile = fopen(settingsPath.c_str(), "w");
	if(pFile != NULL) {
		for(StringVector::const_iterator it = settings.begin(); it != settings.end(); ++it) {
			fprintf(pFile, "%s\n", it->c_str());
		}
		fclose(pFile);
	}
}

StringVector OptionsFile::getOptionStrings() {
	StringVector returnVector;
	FILE* pFile = fopen(settingsPath.c_str(), "r");
	if(pFile != NULL) {
		char lineBuff[128];
		while(fgets(lineBuff, sizeof lineBuff, pFile)) {
			if(strlen(lineBuff) > 2)
				returnVector.push_back(std::string(lineBuff));
		}
		fclose(pFile);
	}
	return returnVector;
}
