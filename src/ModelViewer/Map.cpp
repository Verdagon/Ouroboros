#include "Game.h"
#import "Object.h"

Map::Map(int tileLengthInMapUnits_, const MapTiles *tiles_) :
tileLengthInMapUnits(tileLengthInMapUnits_),
tiles(tiles_),
size(tiles->getSize().col * tileLengthInMapUnits, tiles->getSize().row * tileLengthInMapUnits),
grid(size) {
    for (int row = 0; row < tiles->getSize().row; row++) {
        for (int col = 0; col < tiles->getSize().col; col++) {
            Mesh *mesh = NULL;
            
            switch ((*tiles)[TileCoord(row, col)].character) {
                case '.':
                    mesh = new Mesh("blank.obj", "blank.png");
                    break;
                    
                case '#':
                    mesh = new Mesh("pound.obj", "pound.png");
                    break;
                    
                default:
                    assert(false);
                    break;
            }
            
            
            mesh->size = 2;
            mesh->meshMtx = mat4::Identity();
//            mesh->meshMtx *= mat4::Scale(2);
            mesh->meshMtx *= mat4::Translate(col * tileLengthInMapUnits, row * tileLengthInMapUnits, 0);
//            mesh->meshMtx *= mat4::Scale(tileLengthInMapUnits);
//            mesh->meshMtx *= mat4::Scale(.5);
//            mesh->meshMtx *= mat4::Translate(1, 1, 1);
            std::cout << col * tileLengthInMapUnits << " " << row * tileLengthInMapUnits << std::endl;
            
            m_meshList.push_back(mesh);
        }
    }
    
    for (int x = 0; x < size.x; x++) {
        for (int y = 0; y < size.y; y++) {
            Position pos(x, y);
            grid[pos] = GridLocation(&getTileAtPosition(pos));
        }
    }
    
    calculateAllGridLocationsMaximumRadiiOfInhabitingCreatures();
}

Position Map::findCenterOfRandomWalkableAreaOfRadius(int radius) {
    std::set<Position, Position::Before>::iterator randomTileIter = walkablePositionsByRadius[radius].begin();
    std::advance(randomTileIter, rand() % walkablePositionsByRadius[radius].size());
    return *randomTileIter;
}

void Map::placeCreature(Creature *creature) {
    Position creatureOrigin = creature->center + Creature::topLeftOffsetForRadius(creature->radius);
    Position creatureSize = Creature::areaForRadius(creature->radius);
    
    for (int x = creatureOrigin.x; x < creatureOrigin.x + creatureSize.x; x++) {
        for (int y = creatureOrigin.y; y < creatureOrigin.y + creatureSize.y; y++) {
            assert(grid[Position(x, y)].inhabitingCreature == NULL);
            grid[Position(x, y)].inhabitingCreature = creature;
        }
    }
}

bool Map::nextToAnyCalculatedPositions(const Position &currentPosition) {
    for (int deltaX = -1; deltaX <= 1; deltaX++) {
        for (int deltaY = -1; deltaY <= 1; deltaY++) {
            if (deltaX == 0 && deltaY == 0)
                continue;
            
            Position adjacentPosition(currentPosition.x + deltaX, currentPosition.y + deltaY);
            if (adjacentPosition.x < 0 || adjacentPosition.x >= size.x || adjacentPosition.y < 0 || adjacentPosition.y >= size.y)
                continue;
            
            if (grid[adjacentPosition].maximumRadiusOfInhabitingCreature != -1)
                return true;
        }
    }
    
    return false;
}

bool Map::notNextToAnyCalculatedPositions(const Position &currentPosition) {
    return !nextToAnyCalculatedPositions(currentPosition);
}

int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a > b ? a : b;
}

void Map::calculateAllGridLocationsMaximumRadiiOfInhabitingCreatures() {
    int sufficientDistanceFromUnwalkableLocations = 1;
    std::set<Position, Position::Before> uncalculatedPositions;
    
    for (int currentX = 0; currentX < size.x; currentX++) {
        for (int currentY = 0; currentY < size.y; currentY++) {
            Position currentPosition(currentX, currentY);
            if (grid[currentPosition].isWalkable()) {
                uncalculatedPositions.insert(currentPosition);
            }
            else {
                grid[currentPosition].maximumRadiusOfInhabitingCreature = 0;
            }
        }
    }
    
    int biggestRadiusThatCanFitSomewhereOnTheMap = 0;
    
    while (!uncalculatedPositions.empty()) {
        std::set<Position, Position::Before> uncalculatedPositionsNextToCalculatedPositions;
        
        for (std::set<Position, Position::Before>::iterator i = uncalculatedPositions.begin(), iEnd = uncalculatedPositions.end(); i != iEnd; i++)
            if (nextToAnyCalculatedPositions(*i))
                uncalculatedPositionsNextToCalculatedPositions.insert(*i);
        
        for (std::set<Position, Position::Before>::iterator i = uncalculatedPositionsNextToCalculatedPositions.begin(), iEnd = uncalculatedPositionsNextToCalculatedPositions.end(); i != iEnd; i++) {
            Position currentPosition = *i;
            
            int minimumOfAdjacentsMaximumRadiusOfInhabitingCreature = std::numeric_limits<int>::max();
            for (int deltaX = -1; deltaX <= 1; deltaX++) {
                for (int deltaY = -1; deltaY <= 1; deltaY++) {
                    if (deltaX == 0 && deltaY == 0)
                        continue;
                    
                    Position adjacentPosition(currentPosition.x + deltaX, currentPosition.y + deltaY);
                    if (adjacentPosition.x < 0 || adjacentPosition.x >= size.x || adjacentPosition.y < 0 || adjacentPosition.y >= size.y)
                        continue;
                    
                    int adjacentPositionMaximumRadiusOfInhabitingCreature = grid[adjacentPosition].maximumRadiusOfInhabitingCreature;
                    if (adjacentPositionMaximumRadiusOfInhabitingCreature != -1)
                        minimumOfAdjacentsMaximumRadiusOfInhabitingCreature = min(minimumOfAdjacentsMaximumRadiusOfInhabitingCreature, adjacentPositionMaximumRadiusOfInhabitingCreature);
                }
            }
            
            assert(minimumOfAdjacentsMaximumRadiusOfInhabitingCreature >= 0);
            assert(minimumOfAdjacentsMaximumRadiusOfInhabitingCreature < tiles->getSize().row * tiles->getSize().col);
            
            grid[currentPosition].maximumRadiusOfInhabitingCreature = minimumOfAdjacentsMaximumRadiusOfInhabitingCreature + 1;
            biggestRadiusThatCanFitSomewhereOnTheMap = max(biggestRadiusThatCanFitSomewhereOnTheMap, grid[currentPosition].maximumRadiusOfInhabitingCreature);
            
            uncalculatedPositions.erase(currentPosition);
        }
    }
    
    for (int i = 0; i < biggestRadiusThatCanFitSomewhereOnTheMap; i++)
        walkablePositionsByRadius.push_back(std::set<Position, Position::Before>());
    
    for (int currentX = 0; currentX < size.x; currentX++) {
        for (int currentY = 0; currentY < size.y; currentY++) {
            Position currentPosition(currentX, currentY);
            for (int sizeThatCanFit = 0; sizeThatCanFit < grid[currentPosition].maximumRadiusOfInhabitingCreature; sizeThatCanFit++) {
                walkablePositionsByRadius[sizeThatCanFit].insert(currentPosition);
            }
        }
    }
    
    for (int i = 0; i < biggestRadiusThatCanFitSomewhereOnTheMap; i++) {
        std::cout << "There are " << walkablePositionsByRadius[i].size() << " places where someone of radius " << i << " can fit " << std::endl;
    }
}

class CloserToDestination {
    Position destination;
public:
    CloserToDestination(const Position &destination_) :
    destination(destination_) { }
    
    bool operator()(const Position &a, const Position &b) const {
        int aDistSquared = (a.x - destination.x) * (a.x - destination.x) + (a.y - destination.y) * (a.y - destination.y);
        int bDistSquared = (b.x - destination.x) * (b.x - destination.x) + (b.y - destination.y) * (b.y - destination.y);
        return aDistSquared < bDistSquared;
    }
};

struct Node {
    const Position position;
    Node *parent;
    float distanceTraveled;
    const float distanceToDestination;
    
    Node(const Position &position_, int distanceToDestination_) :
    position(position_),
    parent(NULL),
    distanceTraveled(-1),
    distanceToDestination(distanceToDestination_) { }
    
    struct EstimatedBetterPath {
        bool operator()(const Node *a, const Node *b) const {
            return a->distanceTraveled + a->distanceToDestination < b->distanceTraveled + b->distanceToDestination;
        }
    };
};

bool Map::findPath(const Creature *creature, const Position &destination, std::list<Position> *finalPath) const {
    OrderedPairMap<Position, Position::XGetter, Position::YGetter, Node *> nodesMap(size, NULL);
    for (int x = 0; x < size.x; x++) {
        for (int y = 0; y < size.y; y++) {
            Position pos(x, y);
            nodesMap[pos] = new Node(pos, distance(pos, destination));
        }
    }
    
    std::set<Node *, Node::EstimatedBetterPath> openList;
    nodesMap[creature->center]->distanceTraveled = 0;
    openList.insert(nodesMap[creature->center]);
    
    std::set<Node *> closedList;
    
    while (!openList.empty()) {
        Node *currentNode = *openList.begin();
        openList.erase(openList.begin());
        
        closedList.insert(currentNode);
        
        if (currentNode->position == destination)
            break;
        
        for (int deltaX = -1; deltaX <= 1; deltaX++) {
            for (int deltaY = -1; deltaY <= 1; deltaY++) {
                if (deltaX == 0 && deltaY == 0)
                    continue;
                
                Position adjacentSquare(currentNode->position.x + deltaX, currentNode->position.y + deltaY);
                if (adjacentSquare.x < 0 || adjacentSquare.x >= size.x || adjacentSquare.y < 0 || adjacentSquare.y >= size.y)
                    continue;
                
                Node *adjacentNode = nodesMap[adjacentSquare];
                
                if (closedList.count(adjacentNode))
                    continue;
                
                bool adjacentWalkable = creature->radius < grid[adjacentSquare].maximumRadiusOfInhabitingCreature;
                if (!adjacentWalkable)
                    continue;
                
                float adjacentNodeDistanceTraveledViaCurrentNode = currentNode->distanceTraveled + deltaX * deltaY ? 1.414 : 1.0; // if diagonal, 1.414, else, 1.0
                
                if (!openList.count(adjacentNode)) {
                    adjacentNode->parent = currentNode;
                    adjacentNode->distanceTraveled = adjacentNodeDistanceTraveledViaCurrentNode;
                    openList.insert(adjacentNode);
                }
                else {
                    float adjacentNodeDistanceTraveledViaExistingParent = adjacentNode->distanceTraveled;
                    
                    if (adjacentNodeDistanceTraveledViaCurrentNode < adjacentNodeDistanceTraveledViaExistingParent) {
                        openList.erase(adjacentNode);
                        adjacentNode->distanceTraveled = adjacentNodeDistanceTraveledViaCurrentNode;
                        adjacentNode->parent = currentNode;
                        openList.insert(adjacentNode);
                    }
                }
            }
        }
    }
    
    std::cout << "openList size: " << openList.size() << std::endl;
    
    if (closedList.count(nodesMap[destination])) {
        std::cout << "Found path" << std::endl;
        
        finalPath->clear();
        for (Node *node = nodesMap[destination]; node; node = node->parent)
            finalPath->push_front(node->position);
        
        
        for (int currentY = 0; currentY < size.y; currentY++) {
            for (int currentX = 0; currentX < size.x; currentX++) {
                Position currentPosition(currentX, currentY);
                
                if (std::count(finalPath->begin(), finalPath->end(), currentPosition) == 1)
                    std::cout << 'X';
                else
                    std::cout << (grid[currentPosition].isWalkable() ? '.' : 'n');
            }
            std::cout << std::endl;
        }
        
        return true;
    }
    else {
        return false;
    }
}

