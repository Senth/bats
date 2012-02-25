#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <BWAPI.h>

struct CTokens {
	std::string key;
	std::string value;
};

/** This class reads and parses the bthai-config.txt file, and contain methods for getting
 * parameters from the file.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Config {

private:
	std::string scriptPath;
	std::string botName;
	std::string version;

	std::string info1;
	std::string info2;

	bool init;

	void readConfigFile();

	CTokens split(std::string line, std::string delimiter);
	int toInt(std::string &str);

	static Config* instance;
	
	Config();

public:
	~Config();

	/** Returns class instance. */
	static Config* getInstance();

	/** Returns the path to the folder where the scripfiles are placed. */
	std::string getScriptPath();

	/** Returns the name of the bot as specified in the config file. */
	std::string getBotName();

	/** Returns the current bot version. */
	std::string getVersion();

	/** Displays bot name in the game window. */
	void displayBotName();
};

#endif
