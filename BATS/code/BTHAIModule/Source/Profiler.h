#include <string>
#include <vector>

class ProfilerObj;

/** This class is used to measure the time (in milliseconds) it takes to execute a codeblock.
 * It also counts timeouts according to the rules from the AIIDE 2011 bot competition. If one of
 * the following three conditions are true, a bot is disqualified due to timeout:
 * - 1 frame over 1 minute execution time.
 * - 10 frames over 1 second execution time.
 * - 320 frames over 55 ms execution time.
 * 
 * The profiler can measure multiple parallell codeblocks. It uses an identifier std::string to differ
 * between different blocks.
 *
 * After a game is ended all profiling data is stored in a html file in the BTHAI-data folder.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Profiler {

	private:
		std::vector<ProfilerObj*> obj;
		ProfilerObj* getObj(const std::string& mId);

		Profiler();
		static Profiler* instance;
		static bool instanceFlag;

	public:
		/** Destructor */
		~Profiler();

		/** Returns the instance of the class. */
		static Profiler* getInstance();

		/** Starts measuring. Put at beginning of a codeblock. 
		 * Make sure the startiId is the same as the end id. */
		void start(const std::string& mId);

		/** Stops measuring. Put at the end of a codeblock.
		 * Make sure the startiId is the same as the end id. */
		void end(const std::string& mId);

		/** Returns the time elapsed between start and end. */
		int getElapsed(const std::string& mId);

		/** Outputs result to the in-game chat window. */
		void show(const std::string& mId);
		
		/** Outputs result from all profiles to the in-game chat window. */
		void showAll();

		/** Stores all profiling data to file. */
		void dumpToFile();
	
};
