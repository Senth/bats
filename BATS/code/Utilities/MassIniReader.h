#pragma once

#include "Helper.h"
#include "IniReader.h"
#include <list>

#ifdef WINDOWS
#include <Windows.h>
#endif

// Namespace for the project
namespace utilities {

/**
 * An ini reader that can read a whole directory with ini-files. Can open
 * a directory and read all ini files, you can specify if it shall open all
 * ini-files recursively in sub directories, and an optional filter to only
 * open files with the specific name. \n
 * \n
 * Use openDirectory() to open a directory. You then read the next variable with
 * readNext(). isOpen() can be used to test if the directory is open, i.e. exists.
 * 
 * To see whether a next variable exists for reading; this will automatically close
 * and open files to read the configuration from. I.e. when it's done with file A.ini it
 * will close file A.ini, open file B.ini and read the next variable from B.ini  
 * 
 * @todo Implement filter functionality with regex.
 * @todo Implentation for linux
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 */
class MassIniReader : protected IniReader
{
public:
	/**
	 * Default constructor
	 */
	MassIniReader();

	/**
	 * Constructor that takes a string with the directory path to open.
	 * Equivalent to calling the default constructor and then open().
	 * @param[in] directory path to the directory to open.
	 * @param[in] recursive if it shall continue reading ini-files from sub directories,
	 * defaults to false.
	 * @param[in] regex a filter to only read the ini files matching the regex filter. Not
	 * implemented for the moment, defaults to all ini files.	 
	 */
	MassIniReader(
		const std::string& directory,
		bool recursive = false,
		const std::string& regex = ""
	);

	/**
	 * Destructor
	 */
	virtual ~MassIniReader();

	/**
	 * Opens the specified directory with ini-files.
	 * @param[in] directory path to the directory to open.
	 * @param[in] recursive if it shall continue reading ini-files from sub directories,
	 * defaults to false.
	 * @param[in] regex a filter to only read the ini files matching the regex filter. Not
	 * implemented for the moment, defaults to all ini files.
	 */
	virtual void open(
		const std::string& directory,
		bool recursive = false,
		const std::string& regex = ""
	);

	/**
	 * Closes the directory and the file it is currently reading from. No reads are
	 * now possible from this instance until open has been called again.
	 */
	virtual void close();

	/**
	 * Checks whether the directory is opened.
	 * @return true if the the directory exists, false otherwise.
	 * @see isGood() for checking if a next variable exists; it also checks whether
	 * the directory is open.
	 */
	virtual bool isOpen() const;

	/**
	 * Checks whether a next variable exists for reading; if the directory isn't opened (
	 * isOpen()) it will return false—i.e. no next variable exists.
	 * @return true if a next variable exists for reading, otherwise false.
	 */
	virtual bool isGood() const;

	/**
	 * Reads the next variable and returns its values through variableInfo.
	 * @param[out] variableInfo the variable it will place all information into.
	 * @return true if a variable was read, false i no next variable exists.
	 */
	virtual bool readNext(VariableInfo& variableInfo);

	/**
	 * Skips directly to the next file. The next call to readNext will thus be
	 * in another file than the current.
	 */
	virtual void nextFile();
private:
	/**
	 * Find all ini files to read from and put them into the file queue.
	 * @param[in] directory the directory to search the files for
	 */
	void findAllFiles(const std::string& directory);

	std::string mDirectory;
	std::string mRegex;
	bool mRecursive;
	bool mOpened;

	std::list<std::string> mFileQueue;
};
}