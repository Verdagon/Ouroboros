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
    mutable Mesh *mesh;
    
    Tile() :
    walkable(false),
    character('?'),
    mesh(NULL) { }
    
    Tile(bool walkable_, char character_) :
    walkable(walkable_),
    character(character_),
    mesh(NULL) { }
    
    inline bool operator==(const Tile &that) const {
        return character == that.character;
    }
    
    inline bool operator!=(const Tile &that) const { return !operator==(that); }
};

class MapTiles : public OrderedPairMap<TileCoord, TileCoord::RowGetter, TileCoord::ColGetter, Tile>, public IObject {
public:
    MapTiles(const TileCoord &size, const Tile &defaultTile) :
    OrderedPairMap(size, defaultTile) { }
    
    void fillAreaWithTile(const Area &area, const Tile &tile) {
        for (int row = area.origin.row; row < area.origin.row + area.size.row; row++)
            for (int col = area.origin.col; col < area.origin.col + area.size.col; col++)
                (*this)[TileCoord(row, col)] = tile;
    }
    
    void setLightPosition(const TileCoord &lightCoord) {
        for (int row = 0; row < size.row; row++) {
            for (int col = 0; col < size.col; col++) {
                TileCoord coord(row, col);
                (*this)[coord].mesh->display = (sqrt((lightCoord.row - coord.row) * (lightCoord.row - coord.row) + (lightCoord.col - coord.col) * (lightCoord.col - coord.col)) < 10);
            }
        }
    }
    
    list<IMesh *> visibleMeshes;
    virtual list<IMesh *>* getMeshes() {
        return &visibleMeshes;
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

class GridNode {
    const Tile *tile;
    
public:
    int maximumRadiusOfInhabitingCreature;
    Creature *inhabitingCreature;
    
    GridNode() :
    tile(NULL),
    maximumRadiusOfInhabitingCreature(-1),
    inhabitingCreature(NULL) { }
    
    GridNode(const Tile *tile_) :
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
    vec3 orientation;
    
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
    center(center_),
    orientation(0, 1, 0) {
        switch (character) {
            case '@': {
                IMesh *mesh = new Mesh("atsym.obj", "atsym.png");
                mesh->size = 2;
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
    
    void setCenter(const Position &pos) {
        for (list<IMesh *>::iterator i = m_meshList.begin(), iEnd = m_meshList.end(); i != iEnd; i++) {
            IMesh *mesh = *i;
            mesh->meshMtx = mat4::Identity();
            mesh->meshMtx *= mat4::Rotate(-90, vec3(1, 0, 0));
            mesh->meshMtx *= mat4::Translate(center.x - (radius), -(center.y - (radius)), 0);
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



class Map {
public:
    const int tileLengthInMapUnits;
    MapTiles *const tiles;
    const Position size;
    
    typedef OrderedPairMap<Position, Position::XGetter, Position::YGetter, GridNode> GridType;
    GridType grid;
    
    std::vector<std::set<Position, Position::Before> > walkablePositionsByRadius;
    
    Map(int tileLengthInMapUnits_, MapTiles *tiles_);
    
    inline TileCoord tileCoordAtPosition(const Position &pos) {
        return TileCoord(pos.y / tileLengthInMapUnits, pos.x / tileLengthInMapUnits);
    }
    
    inline const Tile &getTileAtPosition(const Position &pos) {
        return (*tiles)[tileCoordAtPosition(pos)];
    }
    
    Position findCenterOfRandomWalkableAreaOfRadius(int radius);
    
    void placeCreature(Creature *creature);
    void removeCreature(Creature *creature);
    bool areaIsWalkable(const Position &pos, int radius);

private:
    bool nextToAnyCalculatedPositions(const Position &currentPosition);
    
    bool notNextToAnyCalculatedPositions(const Position &currentPosition);
    
    void calculateAllGridNodesMaximumRadiiOfInhabitingCreatures();
    
public:
    bool findPath(const Creature *creature, const Position &destination, std::list<Position> *finalPath) const;
};

#endif