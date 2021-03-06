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

#pragma once

#include <string>
#include <sstream>
#include <cstdlib>
#include "Helper.h"

namespace utilities {

/**
 * Enumeration for logging levels. Guidelines have been given to help the user
 * choose what LogLevel s/he can use.
 */
enum LogLevels {
	/**
	 * Messages that are called often and several times in a function.
	 * For example loop messages.
	 */
	LogLevel_Finest,
	/**
	 * Messages inside functions, but not inside loops. More of information
	 * what is happening inside the function.
	 */
	LogLevel_Finer,
	/**
	 * The first and last message (and statement) in a function to see that a function
	 * has been called, and a function has ended. Could be in the middle of the function if 
	 * you return from the middle.
	 */
	LogLevel_Fine,
	/**
	 * General messages that could be good to know. These include initialization successes for big
	 * functions, e.g. window created.
	 */
	LogLevel_Info,
	/**
	 * Warning messages that, maybe an item wasn't found that should be found, but doesn't cause
	 * any major error. Or that we could not create a window for the specific type so that we use
	 * another one instead.
	 */
	LogLevel_Warning,
	/**
	 * Severe messages are in essential error messages. Although the error should not be that
	 * great to crash the program itself. Consider using the macro ERROR_MESSAG(...) instead of
	 * DEBUG_MESSAGE
	 */
	LogLevel_Severe,
	/**
	 * Disables all messages, never use this for logging messages.
	 */
	LogLevel_Off
};

#if defined(_DEBUG) || defined(USE_DEBUG_MESSAGE)
/**
* Prints a debug message. Does not process the message if verbosity level is too low.
* @param verbosity tho verbosity level to use
* @param message the message to display, using streams are possible
* @note Only active in debug if _DEBUG is defined, or can be set active when USE_DEBUG_MESSAGE
* is defined. When inactive the macro is replaced by empty space, meaning no extra processing
* is done.
* @see utilities::LogLevels for guidelines what levels to use when
* @see ERROR_MESSAGE() if you want to print an error message.
* 
*  @section Examples
* \code
* // Prints a regular message
* DEBUG_MESSAGE(LogLevel_Info, "This is a message");
* 
* // You can also use "cout" syntax to create more complex messages.
* DEBUG_MESSAGE(LogLevel_Warning, "Value of " << strMessage << " is: " << intValue);
* \endcode
*/
#define DEBUG_MESSAGE(verbosity, message) { \
	if (verbosity >= utilities::gVerbosityLevelConsole || \
		verbosity >= utilities::gVerbosityLevelFile || \
		verbosity >= utilities::gVerbosityLevelStarCraft) \
	{ \
		std::stringstream ss; \
		ss << message; \
		utilities::printDebugMessage(verbosity, ss.str(), false); \
	} \
}

/**
* Prints a debug message and waits for input after the message has been displayed.
* Only active if _DEBUG is defined, and only stops for console verbosity.
* @param verbosity the verbosity level to use
* @param message the message to display, using streams are possible
* @note Only active in debug if _DEBUG is defined, or can be set active when USE_DEBUG_MESSAGE
* is defined. When inactive the macro is replaced by empty space, meaning no extra processing
* is done.
* @see utilities::LogLevels for guidelines what levels to use when
* @see DEBUG_MESSAGE() for examples how to this function.
* @see DEBUG_MESSAGE_CONDITION() which only prints a debug message when a condition is true.
* @see ERROR_MESSAGE() if you want to print an error message.
*/
#define DEBUG_MESSAGE_STOP(verbosity, message) {  \
	if (verbosity >= utilities::gVerbosityLevelConsole || \
	verbosity >= utilities::gVerbosityLevelFile || \
	verbosity >= utilities::gVerbosityLevelStarCraft) \
	{ \
		std::stringstream ss; \
		ss << message; \
		utilities::printDebugMessage(verbosity, ss.str(), true); \
	} \
}

/**
 * Prints a debug message if the conditional is true.
 * @param condition the condition that needs to be true for the message to appear.
 * @param verbosity the verbosity level to use
 * @param message the message to display, using streams are possible
 * @note Only active in debug if _DEBUG is defined, or can be set active when USE_DEBUG_MESSAGE
 * is defined. When inactive the macro is replaced by empty space, meaning no extra processing
 * is done.
 * @see utilities::LogLevels for guidelines what levels to use when
 * @see ERROR_MESSAGE() if you want to print an error message.
 * 
 * @section Examples
 * Only prints the debug message when the result wasn't equal to SUCCESS.
 * \code
 * DEBUG_MESSAGE_CONDITION(result != SUCCESS, LogLevel_Warning, "Your message :D");
 * \endcode
 */
#define DEBUG_MESSAGE_CONDITION(condition, verbosity, message) { \
	if (condition) { \
		if (verbosity >= utilities::gVerbosityLevelConsole || \
			verbosity >= utilities::gVerbosityLevelFile || \
			verbosity >= utilities::gVerbosityLevelStarCraft) \
		{ \
			std::stringstream ss; \
			ss << message; \
			utilities::printDebugMessage(verbosity, ss.str(), false); \
		} \
	} \
}
#else
#define DEBUG_MESSAGE(verbosity, message)
#define DEBUG_MESSAGE_STOP(verbosity, message)
#define DEBUG_MESSAGE_CONDITION(condition, verbosity, message)
#endif

// The types of output targets that are available
#define OUTPUT_CONSOLE		0x0001	/**< Output messages to the console window */
#define OUTPUT_FILE			0x0002	/**< Output messages to a file */
#define OUTPUT_STARCRAFT	0x0004  /**< Output messages to StarCraft */

extern int gVerbosityLevelConsole;
extern int gVerbosityLevelFile;
extern int gVerbosityLevelStarCraft;
extern int gOutputTargetError;
extern int gOutputTargetDebugMessage;
extern int gcError;

/**
* Set the output target for the error messages. The parameter values can be combined
* with the binary | operator.
* @param targets the specified output targets, can be OUTPUT_CONSOLE, OUTPUT_FILE.
* @pre if OUTPUT_FILE is specified a directory for the file output must be set with
* setFileError(const std::string) first.
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
 * Reads the settings from the specified file. This file has the syntax:
 * \code
 * DEBUG = TARGET | ANOTHER_TARGET; e.g. DEBUG = FILE | CONSOLE;
 * ERROR = TARGET | ANOTHER_TARGET;
 * TARGET = VERBOSITY; e.g. FILE = FINEST;
 * \endcode
 * @section Example
 * \code
 * // config.ini - Note comments are not allowed in the ini-file.
 * DEBUG = CONSOLE | FILE;	// Print debug messages to console and file.
 * ERROR = FILE;			// Print error messages only to file
 * 
 * // Set the debug message verbosity to info for console. Finer debug messages than info will
 * // not be displayed in the console.
 * CONSOLE = INFO;
 * // Set the debug message verbosity to finest for file. All debug messages will
 * // be added to the file.
 * FILE = FINEST;
 * \endcode
 * 
 * @param settingsFile path to the settings file.
 * @note the variables are not case sensitive, but the variables have been shortened
 * for easier reading.
 */
void loadLogSettings(const std::string& settingsFile);

/**
* Sets the verbosity level.
* @param verbosity the level of verbosity we want to set, between LEVEL_HIGHEST and LEVEL_LOWEST.
* @param target the target to set the verbosity level in, can be OUTPUT_CONSOLE or OUTPUT_FILE
* @note setting the verbosity level to LogLevel_Severe only displays messages that are of highest
* priority (i.e. LogLevel_Sever) and LogLevel_Finest displays all messages.
*/
void setVerbosityLevel(LogLevels verbosity, int target);

/**
* Error messages, uses streams so you can use it in the same manner as DEBUG_MESSAGE()
* @param forceQuit if the program should force a quit.
* @param message is the error message to display
*/
#define ERROR_MESSAGE(forceQuit, message) \
{ \
	std::stringstream ss; \
	ss << message; \
	utilities::printErrorMessage(ss.str(), __FILE__, __LINE__); \
	__pragma(warning(push)); \
	__pragma(warning(disable:4127)); \
	if (forceQuit) { \
		exit(EXIT_FAILURE); \
	} \
	__pragma(warning(pop));\
}

/**
 * Prints an error message if the condition is true. Can use streams in the same manner
 * as DEBUG_MESSAGE() does.
 * @param condition the condition to test. Example: NULL == pointer.
 * @param forceQuit if the program shall force a quit if the condition is true.
 * @param message the error message to display, can use streams.
 */
#define ERROR_MESSAGE_CONDITION(condition, forceQuit, message) \
{ \
	if (condition) { \
		std::stringstream ss; \
		ss << message; \
		utilities::printErrorMessage(ss.str(), __FILE__, __LINE__); \
		__pragma(warning(push)); \
		__pragma(warning(disable:4127)); \
		if (forceQuit) { \
			exit(EXIT_FAILURE); \
		} \
		__pragma(warning(pop)); \
	} \
} 

/**
* Checks for errors and prints them, if there were any. Should be called 'last' in the program.
* Prints the number of errors and gives the option to open the error_log file.
*/
void checkForErrors();

/**
* Actually prints the error message
* @param errorMessage the error message
* @param file the file which the error is in
* @param line the line on which the error message appeared on
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

/**
 * Returns the string with the current date in the format: YYYY-MM-DD
 * @return current date as string
 */
std::string getDateStamp();

}

