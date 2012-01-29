#ifndef GAME_H_
#define GAME_H_

#import "Interfaces.h"
#import "Mesh.h"
#import "OrderedPairMap.h"

#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <list>
#include <set>

class TileCoord {
public:
    struct Before {
        bool operator()(const TileCoord &a, const TileCoord &b) {
            return a.row != b.row ? a.row < b.row : a.col < b.col;
        }
    };
    
    struct RowGetter {
        inline const int &operator()(const TileCoord &loc) const { return loc.row; }
        inline int &operator()(TileCoord &loc) const { return loc.row; }
    };
    
    struct ColGetter {
        inline const int &operator()(const TileCoord &loc) const { return loc.col; }
        inline int &operator()(TileCoord &loc) const { return loc.col; }
    };
    
    int row, col;
    
    TileCoord() : row(0), col(0) { }
    
    TileCoord(int row_, int col_) : row(row_), col(col_) { }
    
    inline friend std::ostream &operator<<(std::ostream &out, const TileCoord &tile) {
        return out << "(row " << tile.row << ", col " << tile.col << ")";
    }
    
    inline bool operator==(const TileCoord &that) const { return row == that.row && col == that.col; }
    inline bool operator!=(const TileCoord &that) const { return !operator==(that); }
};

class Area {
public:
    TileCoord origin;
    TileCoord size;
    
    Area(const TileCoord &origin_, const TileCoord &size_) :
    origin(origin_),
    size(size_) { }
    
    inline friend std::ostream &operator<<(std::ostream &out, const Area &area) {
        return out << "(origin " << area.origin << " size " << area.size << ")";
    }
};

class Tile {
public:
    bool walkable;
    char character;
    
    Tile() :
    walkable(false),
    character('?') { }
    
    Tile(bool walkable_, char character_) :
    walkable(walkable_),
    character(character_) { }
    
    inline bool operator==(const Tile &that) const {
        return character == that.character;
    }
    
    inline bool operator!=(const Tile &that) const { return !operator==(that); }
};

class MapTiles : public OrderedPairMap<TileCoord, TileCoord::RowGetter, TileCoord::ColGetter, Tile> {
public:
    MapTiles(const TileCoord &size, const Tile &defaultTile) :
    OrderedPairMap(size, defaultTile) { }
    
    void fillAreaWithTile(const Area &area, const Tile &tile) {
        for (int row = area.origin.row; row < area.origin.row + area.size.row; row++)
            for (int col = area.origin.col; col < area.origin.col + area.size.col; col++)
                (*this)[TileCoord(row, col)] = tile;
    }
};

struct GenerateOptions {
    int numRows, numCols;
    int randomSeed;
    int minRoomRows;
    int minRoomCols;
    int maxRoomRows;
    int maxRoomCols;
    int padding;
    int numRooms;
    Tile wall, floor;
};

MapTiles *generateMap(const GenerateOptions &options);

class Position {
public:
    struct Before {
        bool operator()(const Position &a, const Position &b) const {
            if (a.x != b.x)
                return a.x < b.x;
            return a.y < b.y;
        }
    };
    
    struct XGetter {
        inline const int &operator()(const Position &pos) const {
            return pos.x;
        }
        inline int &operator()(Position &pos) const {
            return pos.x;
        }
    };
    
    struct YGetter {
        inline const int &operator()(const Position &pos) const {
            return pos.y;
        }
        inline int &operator()(Position &pos) const {
            return pos.y;
        }
    };
    
    int x, y;
    
    Position() :
    x(0),
    y(0) { }
    
    Position(int x_, int y_) :
    x(x_),
    y(y_) { }
    
    inline friend std::ostream &operator<<(std::ostream &out, const Position &pos) {
        return out << "(" << pos.x << ", " << pos.y << ")";
    }
    
    inline bool operator==(const Position &that) const {
        return x == that.x && y == that.y;
    }
    
    inline bool operator!=(const Position &that) const { return !operator==(that); }
    
    inline Position operator+(const Position &that) const {
        return Position(x + that.x, y + that.y);
    }
};

class Creature;

class GridLocation {
    const Tile *tile;
    
public:
    int maximumRadiusOfInhabitingCreature;
    Creature *inhabitingCreature;
    
    GridLocation() :
    tile(NULL),
    maximumRadiusOfInhabitingCreature(-1),
    inhabitingCreature(NULL) { }
    
    GridLocation(const Tile *tile_) :
    tile(tile_),
    maximumRadiusOfInhabitingCreature(-1),
    inhabitingCreature(NULL) { }
    
    bool isWalkable() const { return tile->walkable && inhabitingCreature == NULL; }
};

class Creature : public IObject {
public:
    const char character;
    int radius; // if radius is 1, the creature inhabits a 1x1 square. If its 2, the creature inhabits a 3x3 square, if its 3, inhabits 5x5 square, etc.
    Position center;
    
    static Position areaForRadius(int radius) {
        int length = radius * 2 - 1;
        return Position(length, length);
    }
    
    static Position topLeftOffsetForRadius(int radius) {
        return Position(-(radius - 1), -(radius - 1));
    }
    
    Creature(char character_, int radius_, const Position &center_) :
    character(character_),
    radius(radius_),
    center(center_) {
        switch (character) {
            case '@': {
                IMesh *mesh = new Mesh("atsym.obj", "atsym.png");
                mesh->meshMtx = mat4::Translate(center.x, center.y, 0);
                m_meshList.push_back(mesh);
            } break;
                
            default:
                assert(false);
                break;
        }
    }
    
    ~Creature() {
        list<IMesh *>::iterator mesh;
        for (mesh = m_meshList.begin(); mesh != m_meshList.end(); ++mesh) {
            delete *mesh;
        }
    }
    
    void setLoc(vec3 loc) {
        for (list<IMesh *>::iterator i = m_meshList.begin(), iEnd = m_meshList.end(); i != iEnd; i++) {
            IMesh *mesh = *i;
            mesh->meshMtx = mat4::Translate(loc.x, loc.y, loc.z);
        }
    }
    
    list<IMesh *> m_meshList;
    
    virtual list<IMesh *>* getMeshes() {
        return &m_meshList;
    }
};


inline float distance(const Position &a, const Position &b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}



class Map : public IObject {
public:
    const int tileLengthInMapUnits;
    const MapTiles *const tiles;
    const Position size;
    
    typedef OrderedPairMap<Position, Position::XGetter, Position::YGetter, GridLocation> GridType;
    GridType grid;
    
    std::vector<std::set<Position, Position::Before> > walkablePositionsByRadius;
    
    Map(int tileLengthInMapUnits_, const MapTiles *tiles_);
    
    inline TileCoord tileCoordAtPosition(const Position &pos) {
        return TileCoord(pos.y / tileLengthInMapUnits, pos.x / tileLengthInMapUnits);
    }
    
    inline const Tile &getTileAtPosition(const Position &pos) {
        return (*tiles)[tileCoordAtPosition(pos)];
    }
    
    Position findCenterOfRandomWalkableAreaOfRadius(int radius);
    
    void placeCreature(Creature *creature);
    
    list<IMesh *> m_meshList;
    virtual list<IMesh *>* getMeshes() {
        return &m_meshList;
    }

private:
    bool nextToAnyCalculatedPositions(const Position &currentPosition);
    
    bool notNextToAnyCalculatedPositions(const Position &currentPosition);
    
    void calculateAllGridLocationsMaximumRadiiOfInhabitingCreatures();
    
public:
    bool findPath(const Creature *creature, const Position &destination, std::list<Position> *finalPath) const;
};

#endif