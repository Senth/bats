#ifndef __MAPDATAREADER_H__
#define __MAPDATAREADER_H__

#include "BaseAgent.h"

struct MapData {
	BWAPI::TilePosition basePos;
	BWAPI::TilePosition pos;
	int dist;

	bool matches(BWAPI::TilePosition t1, BWAPI::TilePosition t2)
	{
		if (t1.x() == basePos.x() && t1.y() == basePos.y() && t2.x() == pos.x() && t2.y() == pos.y())
		{
			return true;
		}
		return false;
	}
};

/** This class handles load/save of additional mapdata not contained in bwapi/bwta. This
 * is used to quick calculate ground distances between important regions like start positions.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class MapDataReader {

private:
	std::vector<MapData> data;
	std::string mapName;

	void saveFile();
	bool loadFile();
	std::string getFilename();
	void addEntry(std::string line);
	int toInt(std::string &str);

public:
	/** Constructor */
	MapDataReader();

	/** Destructor */
	~MapDataReader();

	/** Reads the map file. If no file is found, a new is generated. */
	void readMap();

	/** Returns the ground distance between two positions. If not found in
	 * the data, bird distance is used. */
	int getDistance(BWAPI::TilePosition t1, BWAPI::TilePosition t2);
};

#endif
