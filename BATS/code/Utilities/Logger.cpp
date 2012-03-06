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
int utilities::gOutputTargetError = OUTPUT_CONSOLE;
int utilities::gOutputTargetDebugMessage = OUTPUT_CONSOLE;
int utilities::gcError = 0;

const std::string ERROR_FILE_NAME = "error_log_";
const std::string DEBUG_FILE_NAME = "debug_message_log_";
const std::string ERROR_DEBUG_EXTENSION = ".txt";

std::ofstream gErrorFile;
std::ofstream gDebugMessageFile;

void utilities::setOutputDirectory(const std::string& dirPath) {
	// Get date and time
	std::string timeStamp = getTimeStamp(true);


	std::string errorPath;
	std::string debugPath;

	// If not empty: make sure the path is correct
	if (dirPath != "") {
		errorPath = dirPath;
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
			errorPath += directoryDelimiter;
			debugPath += directoryDelimiter;
		}
	}

	errorPath += ERROR_FILE_NAME;
	errorPath += timeStamp;
	errorPath += ERROR_DEBUG_EXTENSION;
	debugPath += DEBUG_FILE_NAME;
	debugPath += timeStamp;
	debugPath += ERROR_DEBUG_EXTENSION;

	gErrorFile.open(errorPath.c_str());
	gDebugMessageFile.open(debugPath.c_str());
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
			command += ERROR_FILE_NAME + ERROR_DEBUG_EXTENSION;
			system(command.c_str());
		}

#endif
	// TODO something for linux
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
