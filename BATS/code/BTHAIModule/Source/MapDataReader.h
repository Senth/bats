#pragma once

#include "BaseAgent.h"
#include <vector>

struct MapData {
	BWAPI::TilePosition basePos;
	BWAPI::TilePosition pos;
	int dist;

	bool matches(const BWAPI::TilePosition& t1, const BWAPI::TilePosition& t2) const {
		return t1 == basePos && t2 == pos;
	}
};

/** This class handles load/save of additional mapdata not contained in bwapi/bwta. This
 * is used to quick calculate ground distances between important regions like start positions.
 *
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * @todo add more doxygen documentation
 * @todo add const and references
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
	int getDistance(const BWAPI::TilePosition& t1, const BWAPI::TilePosition& t2) const;
};
