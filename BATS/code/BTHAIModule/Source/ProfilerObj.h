#pragma once

#include <string>

/** Helper class for Profiler. This class represents a profiling of one specific
 * codeblock. Profiling can be done on any number of codeblocks.
 *
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * @todo doxygen
 * @todo references and const
 * @todo names
 */
class ProfilerObj {

private:
	int startTime;
	int endTime;
	std::string id;
	int maxTime;
	int total;
	int startCalls;
	int endCalls;
	int lastShowFrame;

	int timeouts_short;
	int timeouts_medium;
	int timeouts_long;

public:
	/** Constructor */
	ProfilerObj(std::string mId);

	/** Destructor */
	~ProfilerObj();

	/** Checks if this object matches the specified id std::string. */
	bool matches(std::string mId);

	/** Starts measuring a code block. */
	void start();

	/** Stops measuring a code block. */
	void end();

	/** Returns the time elapsed between last start and end (in milliseconds). */
	int getElapsed();

	/** Print data to the in game chat window. */
	void show();

	/** Returns the html std::string for this profiling object. */
	std::string getDumpStr();
};