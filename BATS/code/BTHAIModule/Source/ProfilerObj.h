#ifndef __PROFILEROBJ_H__
#define __PROFILERBJ_H__

#include "BaseAgent.h"

/** Helper class for Profiler. This class represents a profiling of one specific
 * codeblock. Profiling can be done on any number of codeblocks.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
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

	/** Starts measuring a codeblock. */
	void start();

	/** Stops measuring a codeblock. */
	void end();

	/** Returns the time elapsed between last start and end (in milliseconds). */
	int getElapsed();

	/** Print data to the ingame chat window. */
	void show();

	/** Returns the html std::string for this profiling object. */
	std::string getDumpStr();
};

#endif
