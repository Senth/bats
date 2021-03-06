#pragma once

#include <string>
#include <fstream>

// Namespace for the project
namespace utilities {

/** to get it as an bool, int, float, or double
 * please use bool(), int(), float(), double(). Which means that it automatically tries
 * to convert the variable. If the variable has the wrong syntax it will fail and return
 * false or 0 (depending on the type) and print out an error using ERROR_MESSAGE().\n
 * <b>Example</b> Convert the value to an int
 * \code
 * VariableInfo info;
 * info.value = "13";
 * int intValue = info; // intValue now is now 13
 * \endcode
 * <b>Example</b> Convert the value to a bool
 * \code
 * VariableInfo info;
 * info.value = "true";
 * bool boolValue = info; // boolValue now is true
 * \endcode
 */
struct VariableInfo {
	std::string file; /**< The file the variable is in (does not include .ini) */
	std::string section; /**< The section the variable belongs to */
	std::string subsection; /**< The subsection the variable belongs to, if no exists the string is empty */
	std::string name; /**< The name of the variable */
	/**
	 * The value of the variable as a string. @see VariableInfo how to get the value as, bool, int
	 * float, or double.
	 */	
	std::string value; 

	/**
	 * Converts the value to a bool. Usage: bool yourBool = variableInfo;
	 * @returns a converted bool value from the string. If it fails to convert the value it will
	 * return false and print an error using ERROR_MESSAGE().
	 */
	operator bool() const;

	/**
	 * Converts the value to an int. Usage: int yourInt = variableInfoS;
	 * @returns a converted int value from the string. If it fails to convert the value it
	 * will return 0 and print an error using ERROR_MESSAGE().
	 */
	operator int() const;

	/**
	 * Converts the value to an unsigned int (size_t). Usage: size_t yourInt = variableInfo;
	 * @returns a converted unsigned int value from the string. If it fails to convert the value it
	 * will return 0 and print an error using ERROR_MESSAGE().
	 */
	operator size_t() const;

	/**
	 * Converts the value to a float. Usage: float yourFloat = variableInfo;
	 * @returns a converted float value from the string. If it fails to convert the value it
	 * will return 0.0f and print an error using ERROR_MESSAGE().
	 */
	operator float() const;

	/**
	 * Converts the value to a double. Usage: double yourDouble = variableInfo;
	 * @returns a converted double value from the string. If it fails to convert the value it
	 * will return 0.0 and print an erorr using ERROR_MESSAGE().
	 */
	operator double() const;

	/**
	 * Converts the value to a string. Usage: string yourString = variableInfo;
	 * @returns a the value as string.
	 */
	operator const std::string&() const;
};

/**
 * Reads an ini-file, the variables are returned with readNext(). Open a file by either
 * calling the constructor or open() with a filename, check if the file is open by calling
 * isOpen(). readNext() then returns the variable,
 * including section and possible subsection, if there is no variables left or if an error
 * occurred, false is returned; good() will also return false. Close the file by either
 * calling close()�needed if you want to open another file�or when the object is deleted
 * it will close itself. Does not support escaped characters, such as \; and \#\n
 * \n
 * Ini file syntax:\n
 * [section]\n
 * variableName = variableValue # Comment\n
 * [section.subsection] ; another comment\n
 * ; line comment\n
 * # line comment\n
 * variable name for sub		=	variable value for sub\n
 * variable name for sub 2		=	variable value for sub 2\n
 * @see MassIniReader if you want to read many ini-files from a directory.
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 */
class IniReader
{
public:
	/**
	 * Default constructor
	 */
	IniReader();

	/**
	 * Constructor that takes a string with the filepath to open.
	 * Equivalent to calling the default constructor and then open().
	 * @param[in] filePath path to the ini file to open
	 * @param[in] nameToLower if all variable names <b>and sections</b> shall be converted
	 * to lowercase, defaults to false.
	 * @param[in] valueToLower if all variable values shall be converted to lowercase,
	 * default to false.
	 */
	IniReader(const std::string& filePath, bool nameToLower = false, bool valueToLower = false);

	/**
	 * Destructor
	 */
	virtual ~IniReader();

	/**
	 * Opens the specified ini file.
	 * @param[in] filePath path to the ini file to open
	 * @param[in] nameToLower if all variable names <b>and sections</b> shall be converted
	 * to lowercase, defaults to false. 
	 * @param[in] valueToLower if all variable values shall be converted to lowercase,
	 * default to false.
	 */
	virtual void open(const std::string& filePath, bool nameToLower = false, bool valueToLower = false);

	/**
	 * Closes the opened ini file. Also clears any errors
	 */
	virtual void close();

	/**
	 * Checks whether the ini file is opened or not.
	 * @return true if the ini file is opened, false otherwise.
	 * @see isGood() for checking if a next variable exists; it also checks
	 * whether the ini file is opened or not.
	 */
	virtual bool isOpen() const;

	/**
	 * Checks whether a next variable exists for reading; if the file isn't opened (
	 * isOpen()) it will return false�i.e. no next variable exists.
	 * @return true if a next variable exists for reading, otherwise false.
	 */
	virtual bool isGood() const;

	/**
	 * Reads the next variable and returns its values through variableInfo.
	 * @param[out] variableInfo the variable it will place all the information to.
	 * @return true if a variable was read, false if no next variable exists.
	 */
	virtual bool readNext(VariableInfo& variableInfo);

protected:
	/**
	 * Helper function, returns the base of the file name. I.e. it removes everything
	 * before / and \, and the extension (.ini) from the file name
	 * @param[in] filename the current file name to keep the filebase only
	 * @return the filebase of the entire filename.
	 */
	virtual std::string convertToFilebase(const std::string& filename) const;

private:
	/**
	 * Reads the next variable, for internal use only. It always needs to read the
	 * next variable before hand to know whether there exists a next variable or not.
	 */
	virtual void readNext();

	/**
	 * Remove comments from a read line.
	 * @param[in, out] line the line to remove comments from either ; or #.
	 */
	virtual void removeComment(std::string& line) const;


	std::string mFilename;
	std::string mSectionCurrent;
	std::string mSubSectionCurrent;
	std::ifstream mFile;
	bool mGood;
	bool mNameToLower;
	bool mValueToLower;

	VariableInfo mNextVariable;
};
}