/**
 * @file
 * @version 0.1
 * @section COPYRIGHT Copyright © UFO Escape!
 * @author Matteus Magnusson <senth.wallace@gmail.com>
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 */

#include "Logger.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <string>

#if defined(__linux__) || defined(__APPLE__)
#include <cstdio>
#include <sys/time.h>
#include <unistd.h>
#elif defined(WINDOWS)
#include <Windows.h>
#endif

using namespace utilities;

int utilities::gVerbosityLevelConsole = LogLevel_None;
int utilities::gVerbosityLevelFile = LogLevel_None;
int utilities::gVerbosityLevelStarCraft = LogLevel_None;
int utilities::gOutputTargetError = OUTPUT_CONSOLE;
int utilities::gOutputTargetDebugMessage = OUTPUT_CONSOLE;
int utilities::gcError = 0;

const std::string ERROR_FILE_NAME = "error_log_";
const std::string DEBUG_FILE_NAME = "debug_message_log_";
const std::string ERROR_DEBUG_EXTENSION = ".txt";

std::string gErrorPath;
std::ofstream gErrorFile;
std::ofstream gDebugMessageFile;

#include <BWAPI/Game.h>
namespace BWAPI{
	extern Game* Broodwar;
}

void utilities::setOutputDirectory(const std::string& dirPath) {
	// Get date and time
	std::string timeStamp = getTimeStamp(true);


	gErrorPath;
	std::string debugPath;

	// If not empty: make sure the path is correct
	if (dirPath != "") {
		gErrorPath = dirPath;
		debugPath = dirPath;

		// Should we add an ending / or not?
		char lastChar = dirPath[dirPath.size()-1];
		char directoryDelimiter;
#if defined(WINDOWS)
		directoryDelimiter = '\\';
#elif defined(__linux__) || defined(__APPLE__)
		directoryDelimiter = '/';
#endif
		if (lastChar != directoryDelimiter) {
			gErrorPath += directoryDelimiter;
			debugPath += directoryDelimiter;
		}
	}

	gErrorPath += ERROR_FILE_NAME;
	gErrorPath += timeStamp;
	gErrorPath += ERROR_DEBUG_EXTENSION;
	debugPath += DEBUG_FILE_NAME;
	debugPath += timeStamp;
	debugPath += ERROR_DEBUG_EXTENSION;

	// Create the directory if it doesn't exist
#ifdef WINDOWS
	CreateDirectory(dirPath.c_str(), NULL);
#endif
	///@todo create directory for linux

	gErrorFile.open(gErrorPath.c_str());
	gDebugMessageFile.open(debugPath.c_str());
}

void utilities::loadSettings(const std::string& settingsFile) {
	std::ifstream settings(settingsFile.c_str());
	
	if (settings.is_open()) {
		// Read through all tokens
		std::list<std::string> tokens;
		while (settings.good()) {
			std::string token;
			settings >> token;

			std::transform(token.begin(), token.end(), token.begin(), ::toupper);

			// Complete the assignment
			// Note, the token can be as the last char in the current token
			if (token == ";" || token[token.size()-1] == ';') {
				// Remove ; from the token (if it was at the end) and insert
				// it as a token
				if (token[token.size()-1] == ';') {
					token = token.substr(0, token.size()-1);
					tokens.push_back(token);
				}

				if (!tokens.empty()) {
					std::string assigning = tokens.front();
					tokens.pop_front();

					// Next token should be assignment operator
					// but for simplicity anything will do
					tokens.pop_front();

					if (assigning == "DEBUG") {
						int targets = 0;
						while (!tokens.empty()) {
							if (tokens.front() == "FILE") {
								targets |= OUTPUT_FILE;
							} else if (tokens.front() == "CONSOLE") {
								targets |= OUTPUT_CONSOLE;
							} else if (tokens.front() == "STARCRAFT") {
								targets |= OUTPUT_STARCRAFT;
							}
							tokens.pop_front();
						}
						utilities::setOutputTargetDebugMessage(targets);

					} else if (assigning == "ERROR") {
						int targets = 0;
						while (!tokens.empty()) {
							if (tokens.front() == "FILE") {
								targets |= OUTPUT_FILE;
							} else if (tokens.front() == "CONSOLE") {
								targets |= OUTPUT_CONSOLE;
							} else if (tokens.front() == "STARCRAFT") {
								targets |= OUTPUT_STARCRAFT;
							}
							tokens.pop_front();
						}
						utilities::setOutputTargetError(targets);

					} else if (assigning == "FILE") {
						int target = OUTPUT_FILE;

						// Should only be one verbosity level, so skip the rest
						if (!tokens.empty()) {
							if (tokens.front() == "FINEST") {
								utilities::setVerbosityLevel(LogLevel_Finest, target);
							} else if (tokens.front() == "FINER") {
								utilities::setVerbosityLevel(LogLevel_Finer, target);
							} else if (tokens.front() == "FINE") {
								utilities::setVerbosityLevel(LogLevel_Fine, target);
							} else if (tokens.front() == "INFO") {
								utilities::setVerbosityLevel(LogLevel_Info, target);
							} else if (tokens.front() == "WARNING") {
								utilities::setVerbosityLevel(LogLevel_Warning, target);
							} else if (tokens.front() == "SEVERE") {
								utilities::setVerbosityLevel(LogLevel_Severe, target);
							} else if (tokens.front() == "NONE") {
								utilities::setVerbosityLevel(LogLevel_None, target);
							}
						}
						tokens.clear();

					} else if (assigning == "CONSOLE") {
						int target = OUTPUT_CONSOLE;

						// Should only be one verbosity level, so skip the rest
						if (!tokens.empty()) {
							if (tokens.front() == "FINEST") {
								utilities::setVerbosityLevel(LogLevel_Finest, target);
							} else if (tokens.front() == "FINER") {
								utilities::setVerbosityLevel(LogLevel_Finer, target);
							} else if (tokens.front() == "FINE") {
								utilities::setVerbosityLevel(LogLevel_Fine, target);
							} else if (tokens.front() == "INFO") {
								utilities::setVerbosityLevel(LogLevel_Info, target);
							} else if (tokens.front() == "WARNING") {
								utilities::setVerbosityLevel(LogLevel_Warning, target);
							} else if (tokens.front() == "SEVERE") {
								utilities::setVerbosityLevel(LogLevel_Severe, target);
							} else if (tokens.front() == "NONE") {
								utilities::setVerbosityLevel(LogLevel_None, target);
							}
						}
						tokens.clear();

					} else if (assigning == "STARCRAFT") {
						int target = OUTPUT_STARCRAFT;

						// Should only be one verbosity level, so skip the rest
						if (!tokens.empty()) {
							if (tokens.front() == "FINEST") {
								utilities::setVerbosityLevel(LogLevel_Finest, target);
							} else if (tokens.front() == "FINER") {
								utilities::setVerbosityLevel(LogLevel_Finer, target);
							} else if (tokens.front() == "FINE") {
								utilities::setVerbosityLevel(LogLevel_Fine, target);
							} else if (tokens.front() == "INFO") {
								utilities::setVerbosityLevel(LogLevel_Info, target);
							} else if (tokens.front() == "WARNING") {
								utilities::setVerbosityLevel(LogLevel_Warning, target);
							} else if (tokens.front() == "SEVERE") {
								utilities::setVerbosityLevel(LogLevel_Severe, target);
							} else if (tokens.front() == "NONE") {
								utilities::setVerbosityLevel(LogLevel_None, target);
							}
						}
						tokens.clear();

					}
				}
			} else {
				tokens.push_back(token);
			}
		}
	}
}

void utilities::setVerbosityLevel(LogLevels verbosity, int target)
{
	if (target & OUTPUT_CONSOLE)
	{
		gVerbosityLevelConsole = verbosity;
	}

	if (target & OUTPUT_FILE)
	{
		gVerbosityLevelFile = verbosity;
	}

	if (target & OUTPUT_STARCRAFT)
	{
		gVerbosityLevelStarCraft = verbosity;
	}
}

void utilities::checkForErrors()
{
	if (gcError > 0)
	{
#ifdef WINDOWS
		int choice = MessageBoxA(NULL, "There were some errors during the run-time. Do you want to open the error_log file?", "Errors occured!", MB_YESNO | MB_ICONSTOP);

		if (choice == IDYES)
		{
			std::string command = "notepad ";
			command += gErrorPath;
			system(command.c_str());
		}

#endif
	///@TODO something for linux
	}
}

void utilities::printErrorMessage(const std::string& errorMessage, const char* file, long line) {
	std::string time = getTimeStamp();

	gcError++;
	std::stringstream outMessage;
	outMessage << std::left << "****************************************************\n" << std::setw(10) <<
		"MESSAGE: " << errorMessage << "\n" << std::setw(10) <<
		"TIME: " << time << "\n" << std::setw(10) <<
		"FILE: " << file << "\n" << std::setw(10) <<
		"LINE: " << line << "\n****************************************************" << std::endl;

	if (gOutputTargetError & OUTPUT_FILE)
	{
		gErrorFile << outMessage.str() << std::endl;
	}

	if (gOutputTargetError & OUTPUT_CONSOLE)
	{
		std::cerr << outMessage.str() << std::endl;
	}

	if (gOutputTargetError & OUTPUT_STARCRAFT)
	{
		BWAPI::Broodwar->printf("%s", outMessage.str().c_str());
	}
}

void utilities::printDebugMessage(LogLevels verbosity, const std::string& debugMessage, bool stop)
{
	std::string timeStamp = getTimeStamp();

	// File output
	if (verbosity >= gVerbosityLevelFile && gOutputTargetDebugMessage & OUTPUT_FILE)
	{
		gDebugMessageFile << std::left << std::setw(14) << timeStamp << debugMessage << std::endl;
	}

	// console output
	if (verbosity >= gVerbosityLevelConsole)
	{
		if (gOutputTargetDebugMessage & OUTPUT_CONSOLE)
		{
			std::cout << std::left << std::setw(15) << timeStamp << debugMessage << std::endl;
		}

		if (stop)
		{
			std::cin.ignore(100, '\n');
		}
	}
	// StarCraft
	if (verbosity >= gVerbosityLevelStarCraft)
	{
		std::stringstream ss;
		ss << std::left << std::setw(15) << timeStamp << debugMessage;
		BWAPI::Broodwar->printf("%s", ss.str().c_str());
	}
}

std::string utilities::getTimeStamp(bool filenameProof) {

	std::stringstream timeStamp;

	// Set delimiter depending on if fileproof or not
	std::string delimiter = filenameProof ? "-" : ":";


#if defined(WINDOWS)
	SYSTEMTIME curTime;
	GetSystemTime(&curTime);
	timeStamp << std::setfill('0') << std::setw(2) << curTime.wHour+1 << delimiter <<
		std::setw(2) << curTime.wMinute << delimiter <<
		std::setw(2) << curTime.wSecond << delimiter <<
		std::setw(3) << curTime.wMilliseconds << std::setfill(' ') << " ";
#elif defined(__linux__) || defined(__APPLE__)
	char timeString[256];
	time_t timeSeconds = time(0);
	tm* timeInfo = localtime(&timeSeconds);
	if (filenameProof) {
		strftime(timeString,256,"%H-%M-%S",timeInfo);
	} else {
		strftime(timeString,256,"%H:%M:%S",timeInfo);
	}

	timeval dayTime;
	gettimeofday(&dayTime, NULL);
	int milliSeconds = static_cast<int>(static_cast<double>(dayTime.tv_usec) / 1000.0) + 0.5;

	timeStamp << std::setfill('0') << timeString << delimiter << std::setw(3)
				<< milliSeconds << std::setfill(' ');
#endif

	return timeStamp.str();
}
