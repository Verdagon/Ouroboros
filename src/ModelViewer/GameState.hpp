#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "State.hpp"
#include "GameView.hpp"

struct LineSeg {
    LineSeg() {};
    LineSeg(vec2 s, vec2 v, float l) {
        start = s;
        vector = v;
        length = l;
        end = s + v * l;
    };
    LineSeg(vec2 s, vec2 e) {
        start = s;
        end = e;
        vec2 v = e - s;
        vector = v.Normalized();
        length = v.Length();
    }
    vec2 start;
    vec2 vector;
    vec2 end;
    float length;
};

struct Edge {
    Edge() {};
    Edge(LineSeg line, float hitV, float pLine, float pMin, float pMax) {
        lineSeg = line;
        hitValue = hitV;
        projLine = pLine;
        projMin = pMin;
        projMax = pMax;
    }
    LineSeg lineSeg;
    float hitValue;
    float projLine;
    float projMin;
    float projMax;
};

class Face {
public:
    Face() {};
    Face(vec4 planeEqu, vec2 slopeVector, vector<Edge>* sides) {
        m_planeEqu = planeEqu;
        m_slopeVector = slopeVector;
        m_sides = sides;
    };
    ~Face() {
        delete m_sides;
    }
    bool CheckPoint(vec2 point) {
        //Check that the point lies inside of the face
        vector<Edge>::iterator side = m_sides->begin();
        while (side != m_sides->end()) {
            vec2 vec = side->lineSeg.vector;
            float projection = point.x * -vec.y + point.y * vec.x;
            if (projection < side->projMin || projection > side->projMax) {
                return false;
            }
            side++;
        }
        return true;
    }
    bool CheckLine(LineSeg line) {
        //Check if the line intersects the face
        vector<Edge>::iterator side = m_sides->begin();
        while (side != m_sides->end()) {
            vec2 vec = side->lineSeg.vector;
            float point1 = line.start.x * -vec.y + line.start.y * vec.x;
            float point2 = line.end.x * -vec.y + line.end.y * vec.x;
            
            bool underRange = point1 < side->projMin && point2 < side->projMin;
            bool overRange = point1 > side->projMax && point2 > side->projMax;
            if (underRange || overRange) {
                return false;
            }
            side++;
        }
        return true;
    }
    void GetIntersectedEdges(LineSeg line, vector<Edge*>* edges) {
        vector<Edge>::iterator side = m_sides->begin();
        while (side != m_sides->end()) {
            vec2 vec = side->lineSeg.vector;
            float point1 = line.start.x * -vec.y + line.start.y * vec.x;
            float point2 = line.end.x * -vec.y + line.end.y * vec.x;
            
            bool test1 = point1 < side->projLine && point2 > side->projLine;
            bool test2 = point1 > side->projLine && point2 < side->projLine;
            if (test1 || test2) {
                vec = line.vector;
                float projLine = line.start.x * -vec.y + line.start.y * vec.x;
                point1 = side->lineSeg.start.x * -vec.y + side->lineSeg.start.y * vec.x;
                point2 = side->lineSeg.end.x * -vec.y + side->lineSeg.end.y * vec.x;
                
                test1 = point1 < projLine && point2 > projLine;
                test2 = point1 > projLine && point2 < projLine;
                if (test1 || test2) {
                    edges->push_back(&(*side));
                }
            }
            side++;
        }
    }
    float GetElevation(vec2 point) {
        return -(m_planeEqu.x * point.x + m_planeEqu.y * point.y + m_planeEqu.w) / m_planeEqu.z;
    }
    vec2 GetSlope() {
        return m_slopeVector;
    }
    vector<Edge>* GetEdges() {
        return m_sides;
    }
private:
    vec4 m_planeEqu;
    vec2 m_slopeVector;
    vector<Edge>* m_sides; //Should be only three sides
};

class BallState {
public:
    BallState(vec3 startLoc, vector<Face*>* area);
    ~BallState();
    void Reset();
    void Hit(vec2 direction);
    void Roll(float td);
    vec3 GetBallLoc();
    bool Moving();
private:
    LineSeg AdjustSegment(LineSeg nextSeg);
    LineSeg TrimSegment(LineSeg nextSeg);
    LineSeg BounceSegment(LineSeg nextSeg);
    float GetIntersectPeram(LineSeg line1, LineSeg line2);
    vec2 GetPointOnLineSeg(LineSeg line, float peram);
    vec2 GetIntersectPt(LineSeg line1, LineSeg line2);
    //LineSeg ApplyPhysics(LineSeg nextSeg);
    //LineSeg ResolveCollisions(LineSeg nextSeg);
    LineSeg StopBall(LineSeg nextSeg);
    Face* GetFaceForPoint(vec2 point);
    Face* GetFaceForPoint(vector<Face*> faces, vec2 point);
    vector<Face*> GetFacesForLineSeg(LineSeg line, vector<Face*> faces);
    vector<float> GetPeramsFromFaces(LineSeg line, vector<Face*> faces, vector<float> perams);
    //Edge* GetExitEdge(LineSeg line);
    //float GetEnteryDist(Face* face, LineSeg line);
    //LineSeg Bounce(LineSeg nextSeg, Edge edge);
    //vec2 FindIntersect(LineSeg* line1, LineSeg* line2) const;
    vec3 m_startLoc;
    vec3 m_curLoc;
    LineSeg m_nextSeg;
    float m_velocity;
    float m_td;
    vec2 m_direction;
    bool m_moving;
    vector<Face*>* m_area;
    Face* m_curFace;
};

class GameState: public State {
public:
    GameState(IState* oldState, IMap map, ivec2 screen);
    ~GameState();
    void SetMap(IMap map);
    vec3 GetViewLoc();
    void SetViewLoc(vec3 loc);
    void UpdateAnimations(float td);
    void HitBall(vec2 direction);
    void Restart();
    
    vec3 GetScreenLoc(vec3 point);
    vec3 GetPickLoc(vec2 coords, float planeZ);
    vec2 GetBallScreenLoc();
private:
    void LoadMap();
    IMap m_curMap;
    vec3 m_startLoc;
    vec3 m_holeLoc;
    BallState* m_ball;
    GameView* m_gameView;
    
    //Help save some processing power.
    mat4 m_lastTranslation;
    mat4 m_lastProjection;
    mat4 m_inverted;
};


#endif
