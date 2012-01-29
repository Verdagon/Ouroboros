#import "Game.h"

#include <set>
#include <vector>


bool findOnMapRandomTileOfType(const MapTiles &map, const Tile &tile, int numTries, TileCoord *resultingTileLocation) {
    for (int i = 0; i < numTries; i++) {
        int row = rand() % (map.getSize().row - 2);
        int col = rand() % (map.getSize().col - 2);
        
        if (map[TileCoord(row, col)] == tile) {
            *resultingTileLocation = TileCoord(row, col);
            return true;
        }
    }
    
    return false;
}

bool areaOnMapIsFilledWithTile(const MapTiles &map, const Tile &tile, const Area &area) {
    if (area.origin.row < 0 || area.origin.row + area.size.row >= map.getSize().row)
        return false;
    if (area.origin.col < 0 || area.origin.col + area.size.col >= map.getSize().col)
        return false;
    for (int row = area.origin.row; row < area.origin.row + area.size.row; row++)
        for (int col = area.origin.col; col < area.origin.col + area.size.col; col++)
            if (map[TileCoord(row, col)] != tile)
                return false;
    return true;
}

//void displayMap(const Map &map) {
//    std::cout << "Map:" << std::endl;
//    for (int row = 0; row < map.getSize().row; row++) {
//        for (int col = 0; col < map.getSize().col; col++) {
//            std::cout << map[TileLocation(row, col)].character;
//        }
//        std::cout << std::endl;
//    }
//    std::cout << "/Map" << std::endl;
//}

void findContiguous(const MapTiles &map, TileCoord startTileLocation, std::set<TileCoord, TileCoord::Before> *regionTiles) {
    regionTiles->clear();
    
    std::set<TileCoord, TileCoord::Before> tilesToExpandFrom;
    tilesToExpandFrom.insert(startTileLocation);
    
    regionTiles->insert(startTileLocation);
    
    while (!tilesToExpandFrom.empty()) {
        TileCoord tileToExpandFrom = *tilesToExpandFrom.begin();
        tilesToExpandFrom.erase(tilesToExpandFrom.begin());
        
        for (int rowDelta = -1; rowDelta <= 1; rowDelta++) {
            for (int colDelta = -1; colDelta <= 1; colDelta++) {
                if (rowDelta == 0 && colDelta == 0)
                    continue;
                
                TileCoord tile(tileToExpandFrom.row + rowDelta, tileToExpandFrom.col + colDelta);
                
                if (map[tile] == map[tileToExpandFrom]) {
                    if (regionTiles->find(tile) != regionTiles->end())
                        continue;
                    
                    regionTiles->insert(tile);
                    tilesToExpandFrom.insert(tile);
                }
            }
        }
    }
    
//    std::cout << "found contiguous block of size " << regionTiles->size() << std::endl;
}

typedef std::set<TileCoord, TileCoord::Before> Region;
//
//std::map<Region *, char> charsByRegion;

TileCoord goTowards(const TileCoord &from, TileCoord towards) {
    if (from.row < towards.row)
        return TileCoord(from.row + 1, from.col);
    if (from.row > towards.row)
        return TileCoord(from.row - 1, from.col);
    if (from.col < towards.col)
        return TileCoord(from.row, from.col + 1);
    if (from.col > towards.col)
        return TileCoord(from.row, from.col - 1);
    assert(false);
}

int distanceSquared(const TileCoord &a, const TileCoord &b) {
    return (a.row - b.row) * (a.row - b.row) + (a.col - b.col) * (a.col - b.col);
}

TileCoord findTileLocationInRegionClosestToTileLocation(Region *region, TileCoord destination) {
    int closestDistanceSquared = distanceSquared(*region->begin(), destination);
    TileCoord closestTileLocation = *region->begin();
    
    for (Region::iterator i = region->begin(), iEnd = region->end(); i != iEnd; i++) {
        TileCoord testTileLocation = *i;
        int testTileLocationDistanceSquared = distanceSquared(testTileLocation, destination);
        if (testTileLocationDistanceSquared < closestDistanceSquared) {
            closestTileLocation = testTileLocation;
            closestDistanceSquared = testTileLocationDistanceSquared;
        }
    }
    
    return closestTileLocation;
}

void digTowardsButStopWhenHitAnotherRegion(MapTiles *map, Region ***regionsByRowCol, Region *region, TileCoord digTo) {
    TileCoord closestFromRegion1 = findTileLocationInRegionClosestToTileLocation(region, digTo);
    
    TileCoord current = closestFromRegion1;
    while (true) {
        current = goTowards(current, digTo);
        if (current == digTo)
            break;
        
        Region *&currentTileRegion = regionsByRowCol[current.row][current.col];
        
        if (currentTileRegion == NULL) {
            currentTileRegion = region;
            (*map)[current] = (*map)[closestFromRegion1];
        }
        
        bool hitAnotherRegion = false;
        for (int rowDelta = -1; rowDelta <= 1; rowDelta++) {
            for (int colDelta = -1; colDelta <= 1; colDelta++) {
                if (rowDelta * colDelta == 0 && (rowDelta || colDelta)) { // gives (-1, 0), (0, -1), (1, 0), (0, 1), which is all four directions
                    Region *adjacentTileRegion = regionsByRowCol[current.row + rowDelta][current.col + colDelta];
                    if (adjacentTileRegion != NULL && adjacentTileRegion != currentTileRegion) {
                        hitAnotherRegion = true;
                    }
                }
            }
        }
        
        if (hitAnotherRegion) {
//            std::cout << "Dug from region " << region << " at " << closestFromRegion1 << " to " << digTo << " and encountered other region " << regionsByRowCol[current.row][current.col] << " at " << current << std::endl;
            break;
        }
    }
}

//void displayRegions(const Map &map, Region ***regionsByRowCol) {
//    for (int row = 0; row < map.getSize().row; row++) {
//        for (int col = 0; col < map.getSize().col; col++) {
//            if (regionsByRowCol[row][col] == NULL) {
//                std::cout << " ";
//            }
//            else {
//                std::cout << charsByRegion[regionsByRowCol[row][col]];
//            }
//        }
//        std::cout << std::endl;
//    }
//}

void connectAllRegions(MapTiles *map, const Tile &WALL, const Tile &FLOOR) {
    Region ***regionsByRowCol = new Region**[map->getSize().row];
    for (int row = 0; row < map->getSize().row; row++) {
        regionsByRowCol[row] = new Region*[map->getSize().col];
        for (int col = 0; col < map->getSize().col; col++) {
            regionsByRowCol[row][col] = NULL;
        }
    }
    
    std::vector<Region *> regions;
    
//    std::cout << "Regions:" << std::endl;
    
    for (int row = 0; row < map->getSize().row; row++) {
        for (int col = 0; col < map->getSize().col; col++) {
            if ((*map)[TileCoord(row, col)] == FLOOR && regionsByRowCol[row][col] == NULL) {
                std::set<TileCoord, TileCoord::Before> *newRegion = new std::set<TileCoord, TileCoord::Before>();
//                charsByRegion[newRegion] = charsByRegion.size() + '0';
                findContiguous(*map, TileCoord(row, col), newRegion);
                
                for (Region::iterator i = newRegion->begin(), iEnd = newRegion->end(); i != iEnd; i++)
                    regionsByRowCol[i->row][i->col] = newRegion;
                
                regions.push_back(newRegion);
            }
            
//            std::cout << charsByRegion[regionsByRowCol[row][col]];
        }
//        std::cout << std::endl;
    }
//    std::cout << "/Regions" << std::endl;
    
//    std::cout << "Begin!" << std::endl;
//    for (int i = 0; i < regions.size(); i++)
//        std::cout << "region " << regions[i] << " size " << regions[i]->size() << std::endl;
    
    while (regions.size() > 1) {
        int randomRegionsIndex = rand() % (regions.size() - 1);
        Region *region1 = regions[randomRegionsIndex];
        Region *region2 = regions[randomRegionsIndex + 1];
        assert(!region1->empty());
        assert(!region2->empty());
        
        std::set<TileCoord, TileCoord::Before>::iterator iter = region2->begin();
        std::advance(iter, rand() % region2->size());
        TileCoord destination = *iter;
        
//        std::cout << "digging!" << std::endl;
        
        digTowardsButStopWhenHitAnotherRegion(map, regionsByRowCol, region1, destination);
        
//        std::cout << "After dig, before flood: " << std::endl;
//        displayRegions(*map, regionsByRowCol);
//        for (int i = 0; i < regions.size(); i++)
//            std::cout << "region " << regions[i] << " size " << regions[i]->size() << std::endl;
        
//        std::cout << "flooding!" << std::endl;
        
        findContiguous(*map, *region1->begin(), region1);
        for (Region::iterator i = region1->begin(), iEnd = region1->end(); i != iEnd; i++) {
            if (regionsByRowCol[i->row][i->col] != NULL && regionsByRowCol[i->row][i->col] != region1)
                regionsByRowCol[i->row][i->col]->erase(*i);
            regionsByRowCol[i->row][i->col] = region1;
        }
        
//        std::cout << "After flood:" << std::endl;
        
//        for (int i = 0; i < regions.size(); i++)
//            std::cout << "region " << regions[i] << " size " << regions[i]->size() << std::endl;
        
        int numRegionsNowEmpty = 0;
        for (int i = 0; i < regions.size(); ) {
//            if (regions[i]->size() == 1)
//                std::cout << *regions[i]->begin() << std::endl;
            if (regions[i]->empty()) {
                regions.erase(regions.begin() + i);
                numRegionsNowEmpty++;
            }
            else
                i++;
        }
        
        assert(numRegionsNowEmpty);
        
//        displayRegions(*map, regionsByRowCol);
    }
}

MapTiles *generateMap(const GenerateOptions &options) {
    srand(options.randomSeed);
    
    MapTiles *map = new MapTiles(TileCoord(options.numRows, options.numCols), options.wall);
    int numRoomsGenerated = 0;
    
    for (int numTries = 0; numTries < 100 && numRoomsGenerated < options.numRooms; numTries++) {
        TileCoord roomCenterTileLocation;
        if (!findOnMapRandomTileOfType(*map, options.wall, 100, &roomCenterTileLocation))
            break;
        
        Area room(roomCenterTileLocation, TileCoord(1, 1));
        assert(areaOnMapIsFilledWithTile(*map, options.wall, room));
        
        // Expand down until we have max room rows + 2 (the +2 is so we can retract later to have nice padding)
        while (room.size.row < options.maxRoomRows + 2 * options.padding) {
            Area proposedRoom = room;
            proposedRoom.size.row++;
            if (areaOnMapIsFilledWithTile(*map, options.wall, proposedRoom))
                room = proposedRoom;
            else
                break;
        }
        
        // Expand up until we have max room rows + 2 (we may have already gotten that many in the last while loop, which is fine
        while (room.size.row < options.maxRoomRows + 2 * options.padding) {
            Area proposedRoom = room;
            proposedRoom.origin.row--;
            proposedRoom.size.row++;
            if (areaOnMapIsFilledWithTile(*map, options.wall, proposedRoom))
                room = proposedRoom;
            else
                break;
        }
        
        // Expand down until we have max room cols + 2 (the +2 is so we can retract later to have nice padding)
        while (room.size.col < options.maxRoomCols + 2 * options.padding) {
            Area proposedRoom = room;
            proposedRoom.size.col++;
            if (areaOnMapIsFilledWithTile(*map, options.wall, proposedRoom))
                room = proposedRoom;
            else
                break;
        }
        
        // Expand up until we have max room cols + 2 (we may have already gotten that many in the last while loop, which is fine
        while (room.size.col < options.maxRoomCols + 2 * options.padding) {
            Area proposedRoom = room;
            proposedRoom.origin.col--;
            proposedRoom.size.col++;
            if (areaOnMapIsFilledWithTile(*map, options.wall, proposedRoom))
                room = proposedRoom;
            else
                break;
        }
        
        // Retract, so we have a padding of wall around the outside of the room (so no rooms touch)
        room.origin.row += options.padding;
        room.origin.col += options.padding;
        room.size.row -= 2 * options.padding;
        room.size.col -= 2 * options.padding;
        
        if (room.size.row < options.minRoomRows)
            continue;
        if (room.size.col < options.minRoomCols)
            continue;
        
        map->fillAreaWithTile(room, options.floor);
        numRoomsGenerated++;
        
    }
    
    connectAllRegions(map, options.wall, options.floor);
    
    assert(numRoomsGenerated > 0);
    
        std::cout << "Map:" << std::endl;
        for (int row = 0; row < map->getSize().row; row++) {
            for (int col = 0; col < map->getSize().col; col++) {
                std::cout << (*map)[TileCoord(row, col)].character;
            }
            std::cout << std::endl;
        }
        std::cout << "/Map" << std::endl;
    
    return map;
}