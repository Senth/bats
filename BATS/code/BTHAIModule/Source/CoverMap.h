#pragma once

#include "UnitAgent.h"
#include "MapDataReader.h"
#include <map>
#include <BWAPI/TilePosition.h>

// Forward declaration
namespace BWTA {
	class Region;
	class Chokepoint;
}

namespace bats {
	class ExplorationManager;
}

struct Corners {
	int x1;
	int y1;
	int x2;
	int y2;

	/**
	 * Adds a position to these corners.
	 * @param position the position to add to these corners
	 * @return a reference to this object
	 */
	Corners& operator+=(const BWAPI::TilePosition& position) {
		return *this = *this + position;
	}

	/**
	 * Adds all corners with the position, but does not change these corners instead
	 * it returns a new corner.
	 * @param position the position to add to the new corner
	 * @return new Corners object with the position added.
	 */
	inline Corners operator+(const BWAPI::TilePosition& position) const {
		Corners newCorners = *this;
		newCorners.x1 += position.x();
		newCorners.x2 += position.x();
		newCorners.y1 += position.y();
		newCorners.y2 += position.y();
		return newCorners;
	}
};

/**
 * Adds all corners with the position.
 * @param position the position to add to the corners
 * @param corners the corners to add
 * @return new Corners object with the position added
 */
inline Corners operator+(const BWAPI::TilePosition& position, const Corners& corners) {
	return corners + position;
}

/** The CoverMap class is used to keep track of the own base and which Tiles that are occupied by buildings,
 * and which Tiles are free and possibly can be used to construct new buildings on. 
 *
 * Internally a matrix of the same size as the map is used. If a Tile is occupied or cant be reached by ground
 * units, the value if the tile is 0. If the Tile can be built on, the value is 1. 
 * Buildings typically use up more space in the matrix than their actual size since we want some free space
 * around each building. Different types of buildings have different space requirements.
 *
 * The CoverMap is implemented as a singleton class. Each class that needs to access CoverMap can request an instance,
 * and all classes shares the same CoverMap instance.
 *
 * @author Johan Hagelback (johan.hagelback@gmail.com)
 * @author Matteus Magnusson (matteus.magnusson@gmail.com)
 * 
 * @todo add more doxygen documentation (param and private)
 */
class CoverMap {
public:
	/** Destructor */
	~CoverMap();

	/** Finds a good choke point to build at or place units at */
	BWAPI::TilePosition findChokepoint() const;

	/** Returns the instance of the class. */
	static CoverMap* getInstance();

	/** Adds a newly constructed building to the cover map. */
	void addConstructedBuilding(const BWAPI::Unit* unit);

	/** Used by WorkerAgent when constructing builds. */
	void fillTemp(const BWAPI::UnitType& toBuild, const BWAPI::TilePosition& buildSpot);

	/** Used by WorkerAgent when constructing builds. */
	void clearTemp(const BWAPI::UnitType& toBuild, const BWAPI::TilePosition& buildSpot);

	/** Called when a building is destroyed, to free up the space. */
	void buildingDestroyed(const BWAPI::Unit* unit);

	/** Checks if the specified building type can be built at the buildSpot. True if it can,
	 * false otherwise. */
	bool canBuild(const BWAPI::UnitType& toBuild, const BWAPI::TilePosition& buildSpot) const;

	/** Checks if a position is free. */
	bool isPositionFree(const BWAPI::TilePosition& pos) const;

	/** Blocks a position from being used as a valid buildSpot. Used when a worker is timedout when
	 * moving towards the buildSpot. */
	void blockPosition(const BWAPI::TilePosition& buildSpot);

	/** Finds and returns a buildSpot for the specified building type.
	 * If no buildspot is found, a BWAPI::TilePosition(-1,-1) is returned. */
	BWAPI::TilePosition findBuildSpot(const BWAPI::UnitType& toBuild) const;

	/** Searches for the closest vespene gas that is not in use. If no gas is sighted,
	 * the ExplorationManager is queried. */
	BWAPI::TilePosition findRefineryBuildSpot(const BWAPI::UnitType& toBuild, const BWAPI::TilePosition& start) const;

	/** Finds and returns the position of the closest free vespene gas around the specified start position.
	 * If no gas vein is found, a BWAPI::TilePositions::Invalid is returned. */
	BWAPI::TilePosition findClosestGasWithoutRefinery(const BWAPI::UnitType& toBuild, const BWAPI::TilePosition& start) const;

	/** Searches for a spot to build a refinery at. Returns BWAPI::TilePositions::Invalid if no spot was found.*/
	BWAPI::TilePosition searchRefinerySpot() const;

	/** Returns a position of a suitable site for expansion, i.e. new bases. */
	BWAPI::TilePosition findExpansionSite() const;

	/** Finds a mineral to gather from. */
	BWAPI::Unit* findClosestMineral(const BWAPI::TilePosition& workerPos) const;

	/** Shows debug info on screen. */
	void printGraphicDebugInfo();

	/**
	 * Checks if a region is occupied by either our structures or our allied structures.
	 * Can be overridden to only check allied or our structures.
	 * @param region the region to check if it's occupied by our team
	 * @param includeOurStructures if our structures shall be included in the search,
	 * defaults to true
	 * @param includeAlliedStructures if allied structures shall be included in the search,
	 * defaults to true.
	 * @return true if the region is occupied by either any team structure.
	 */
	static bool isRegionOccupiedByOurTeam(
		const BWTA::Region* region,
		bool includeOurStructures = true,
		bool includeAlliedStructures = true
	);


	/**
	 * Different states a tile can be in
	 */
	enum TileStates {
		TileState_Blocked,
		TileState_Buildable,
		TileState_TempBlocked,
		TileState_Mineral,
		TileState_Gas,
		TileState_ExpansionReserved
	};

private:
	CoverMap();
	Corners getCorners(const BWAPI::Unit* unit) const;
	Corners getCorners(const BWAPI::UnitType& type, const BWAPI::TilePosition& center = BWAPI::TilePosition(0,0)) const;
	BWAPI::TilePosition findSpotAtSide(
		const BWAPI::UnitType& toBuild,
		const BWAPI::TilePosition& start,
		const BWAPI::TilePosition& end
	) const;
	bool canBuildAt(const BWAPI::UnitType& toBuild, const BWAPI::TilePosition& pos) const;
	/**
	 * Fills the areas defined by the corners to the specified tile state. Default TileState is
	 * TileState_Blocked
	 * @param corners an area to fill, defined by these corners
	 * @param tileState which state to fill the area with, defaults to TileState_Blocked
	 */
	void fill(const Corners& corners, TileStates tileState = TileState_Blocked);
	void clear(const Corners& corners);
	bool suitableForDetector(const BWAPI::TilePosition& pos) const;
	BWAPI::TilePosition findBuildSpot(const BWAPI::UnitType& toBuild, const BWAPI::TilePosition& start) const;
	BWAPI::Unit* hasMineralNear(const BWAPI::TilePosition& pos) const;
	BWAPI::TilePosition findDefensePos(const BWTA::Chokepoint* choke) const;
	bool isOccupied(const BWTA::Region* region) const;
	bool isEdgeChokepoint(const BWTA::Chokepoint* choke) const;
	double getChokepointPrio(const BWAPI::TilePosition& center) const;

	/**
	 * Checks if an area defined by the corners is free
	 * @param corners the area to check if it's free to build on
	 */
	bool isAreaFree(const Corners& corners) const;
	
	/**
	 * Initializes the default cover map. Adding default agents, minerals
	 * gas, choke points, and static map locations that are inpassible
	 */
	void initCoverMap();

	/**
	 * Finds all TilePosition for all regions and inserts them into the multimap.
	 * This is used later for faster calculation finding all regions in a map
	 */
	void initTilePositionRegions();

	/**
	 * Tries to find a build spot in the specified region
	 * @param unitType the structure to find a build spot for
	 * @param region the region to search the build spot in
	 * @return a valid build position for the structure, if no build spot was
	 * found TilePositions::Invalid will be returned.
	 */
	BWAPI::TilePosition findBuildSpotInRegion(const BWAPI::UnitType unitType, const BWTA::Region* region) const;

	MapDataReader mapData;
	bats::ExplorationManager* mExplorationManager;
	int mRange;
	int mMapWidth;
	int mMapHeight;
	int** mCoverMap;
	typedef std::multimap<const BWTA::Region*, const BWAPI::TilePosition> RegionTileMap;
	RegionTileMap mRegionTiles;

	static CoverMap* msInstance;
};