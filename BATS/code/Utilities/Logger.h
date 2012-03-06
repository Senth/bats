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

#ifndef LOGGER_H_
#define LOGGER_H_

#ifndef WINDOWS
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_WINDOWS)
#define WINDOWS
#define WINDOWS_LEAN_AND_MEAN
#endif
#endif

#include <string>
#include <sstream>
#include <cstdlib>

namespace utilities {

/**
 * Enumeration for logging levels
 */
enum LogLevels {
	LogLevel_Finest, /**< Finest messages, lowest level of logging. Only seen if logging is set to this level. */
	LogLevel_Finer, /** Finer messages */
	LogLevel_Fine, /**< Fine messages to display, finer than info */
	LogLevel_Info, /**< Displays info messages */
	LogLevel_Warning, /**< Displays warning messages */
	LogLevel_Severe, /**< Displays severe messages, highest level of logging */
	LogLevel_None, /**< Only for disabling messages, never specify this for logging messages. */
};

#ifdef _DEBUG
/**
* Prints a debug message. Only active if _DEBUG is defined.
* Does not process the message if verbosity level is too low.
* Examples:
* DEBUG_MESSAGE(LogLevel_Info, "This is a message"); // A regular
* DEBUG_MESSAGE(LogLevel_Debug, "Value of " << strMessage << " is: " << intValue);
* @param message the message to display, using streams are possible
* @param verbosity the verbosity level to use
*/
#define DEBUG_MESSAGE(verbosity, message) { \
	if (verbosity >= gVerbosityLevelConsole || verbosity >= gVerbosityLevelFile) { \
			std::stringstream ss; ss << message; \
			printDebugMessage(verbosity, ss.str(), false); \
	} \
}

/**
* Prints a debug message and waits for input after the message has been displayed.
* Only active if _DEBUG is defined, and only stops for console verbosity.
* @param message the message to display, using streams are possible
* @param verbosity the verbosity level to use
*/
#define DEBUG_MESSAGE_STOP(verbosity, message) {  \
	if (verbosity >= gVerbosityLevelConsole || verbosity >= gVerbosityLevelFile) { \
			std::stringstream ss; ss << message << "  Press enter to continue..."; \
			printDebugMessage(verbosity, ss.str(), true); \
	} \
}
#else
#define DEBUG_MESSAGE(verbosity, message)
#define DEBUG_MESSAGE_STOP(verbosity, message)
#endif

// The types of output targets that are available
#define OUTPUT_CONSOLE		0x0001	/**< Output messages to the console window */
#define OUTPUT_FILE			0x0002	/**< Output messages to a file */

extern int gVerbosityLevelConsole;
extern int gVerbosityLevelFile;
extern int gOutputTargetError;
extern int gOutputTargetDebugMessage;
extern int gcError;

/**
* Set the output target for the error messages. The parameter values can be combined
* with the binary | operator.
* @param targets the specified output targets, can be OUTPUT_CONSOLE, OUTPUT_FILE.
* @pre if OUTPUT_FILE is specified a directory for the file output must be set with
* @setFileError(const std::string) first.
*/
inline void setOutputTargetError(int targets) {gOutputTargetError = targets;}

/**
* Set the output target for the error messages. The parameter values can be combined
* with the binary | operator.
* @param targets the specified output targets, can be OUTPUT_CONSOLE, OUTPUT_FILE.
* @pre if OUTPUT_FILE is specified a directory for the file output must be set with
* setFileError() first.
*/
inline void setOutputTargetDebugMessage(int targets) {gOutputTargetDebugMessage = targets;}

/**
 * Set the directory where to put the output files, both for the error
 * messages and debug messages. Creates a new file with the current date
 * and time at the specified location.
 */
void setOutputDirectory(const std::string& dirPath);

/**
* Sets the verbosity level.
* @param verbosity the level of verbosity we want to set, between LEVEL_HIGHEST and LEVEL_LOWEST.
* @param the target to set the verbosity level in, can be OUTPUT_CONSOLE or OUTPUT_FILE
* @note LEVEL_HIGHEST only displays messages that are of highest priority and LEVEL_LOWEST
* displays all messages
*/
void setVerbosityLevel(LogLevels verbosity, int target);

/**
* Error messages, uses streams so you can use it in the same manner as DEBUG_MESSAGE
* @param forceQuit if the program should forcequit.
* @param message is the error message to display
*/
#define ERROR_MESSAGE(forceQuit, message) \
{ \
	std::stringstream ss; \
	ss << message; \
	printErrorMessage(ss.str(), __FILE__, __LINE__); \
	__pragma(warning(push)); \
	__pragma(warning(disable:4127)); \
	if (forceQuit) { \
		exit(EXIT_FAILURE); \
	} \
	__pragma(warning(pop));\
}

/**
* Checks for error. Should be called 'last' in the program. Checks if there
* were any errors, prints how many there were and gives the option to open
* the error_log file.
*/
void checkForErrors();

/**
* Actually prints the error message
* @param errorMessage the error message
* @param file the file which the error is in
* @param line the line on which the error message appeared on
* @param time the timestamp of the error
*/
void printErrorMessage(const std::string& errorMessage, const char* file, long line);

/**
* Actually prints the debug message
* @param verbosity the verbosity level to use
* @param debugMessage the debug message
* @param stop if we should stop at the message, default is false
*/
void printDebugMessage(LogLevels verbosity, const std::string& debugMessage, bool stop = false);

/**
* Returns a string with the current timestamp in hours, minutes, seconds and milliseconds.
* @param filenameProof if the format should be filename proof, i.e. no colons ':', default false
* @return current timestamp in hours, minutes, seconds and milliseconds.
*/
std::string getTimeStamp(bool filenameProof = false);

}

#endif /* LOGGER_H_ */
