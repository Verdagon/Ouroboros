#include "GameState.hpp"
#include <iostream>

#define FRICTION 7.0
#define HITPOWER 4.0
#define GRAVITY 1.0

BallState::BallState(vec3 startLoc, vector<Face*>* area) {
    m_curLoc = m_startLoc = startLoc;
    m_area = area;
    m_moving = false;
    m_curFace = GetFaceForPoint(vec2(m_curLoc.x, m_curLoc.y));
}

BallState::~BallState() {
    vector<Face*>::iterator face = m_area->begin();
    while (face != m_area->end()) {
        delete *face;
        face++;
    }
    delete m_area;
}

void BallState::Reset() {
    m_curLoc = m_startLoc;
    m_moving = false;
    //m_curFace = GetFaceForPoint(vec2(m_curLoc.x, m_curLoc.y));
}

void BallState::Hit(vec2 direction) {
    m_direction = direction.Normalized();
    m_velocity = direction.Length() * HITPOWER;
    m_moving = true;
}

void BallState::Roll(float td) {
    static bool stop = false;
    
    if (m_moving) {
        m_td = td;
        
        // Set the first path segment (update to handel z value and velocity changes)
        LineSeg nextSeg = LineSeg(vec2(m_curLoc.x, m_curLoc.y), m_direction, (m_velocity * td));
        
        nextSeg = AdjustSegment(nextSeg);
        // Handel collisions
        //nextSeg = ApplyPhysics(nextSeg);
        
        // Addjust velocity and determin if the ball stopped.
        // General Friction
        m_velocity -= FRICTION * td;
        // Change in elivation
        float newElevation = m_curFace->GetElevation(nextSeg.end);
        float elevationChange = m_curLoc.z - newElevation;
        
        if (elevationChange < 0) {
            m_velocity = m_velocity - sqrtf(2*GRAVITY*-elevationChange);
        } else if (elevationChange > 0) {
            m_velocity = m_velocity + sqrtf(2*GRAVITY*elevationChange);
        }
        
        
        // Put the ball in its new place
        m_curLoc = vec3(nextSeg.end.x, nextSeg.end.y, newElevation);
        //m_curLoc = vec3(nextSeg.end.x, nextSeg.end.y, m_curLoc.z);
        m_direction = nextSeg.vector;
        
        // check if the ball comes to a rest
        if (m_velocity <= 0) {
            if (stop) {
                m_moving = false;
            }
            stop = true;
        } else {
            stop = false;
        }
    }
}

vec3 BallState::GetBallLoc() {
    return m_curLoc;
}

bool BallState::Moving() {
    return m_moving;
}

LineSeg BallState::AdjustSegment(LineSeg nextSeg) {
    static int iteration = 0;
    vector<Face*> faceList;
    //vector<Face*>::iterator face;
    vector<float> intercepts;
    //vector<float>::iterator peram;
    //vector<Edge>::iterator edge;
    float lastPeram, midPeram;
    Face  *nextFace, *curFace;
    
    if (++iteration > 10) {
        //Stop the ball if there are more than 10 iterations
        std::cout << "Too Many Iterations of AdjustSegment\n";
        iteration = 0;
        return StopBall(nextSeg);
    }
    
    //Apply the physics of the cur_face
    vec2 slopeVec = m_curFace->GetSlope();
    slopeVec = slopeVec * GRAVITY * m_td;
    vec2 pathVec = nextSeg.vector * nextSeg.length;
    pathVec = pathVec + slopeVec;
    nextSeg = LineSeg(nextSeg.start, nextSeg.start + pathVec);
    
    //Opt out if the segment is still in the face
    if (m_curFace->CheckPoint(nextSeg.end)) {
        iteration = 0;
        return nextSeg;
    }
    
    //Get a list of faces the line segment intersects
    faceList = vector<Face*>(0);
    faceList = GetFacesForLineSeg(nextSeg, faceList);
    
    intercepts = vector<float>(0);
    intercepts = GetPeramsFromFaces(nextSeg, faceList, intercepts);
    
    //Attempt to find the next face
    nextFace = curFace = NULL;
    
    for (int i = intercepts.size()-1; i > 0; i--) {
        midPeram = intercepts[i-1] + (intercepts[i] - intercepts[i-1])/2.0;
        curFace = GetFaceForPoint(faceList, GetPointOnLineSeg(nextSeg, midPeram));
        if (curFace == m_curFace)
            break;
        nextFace = curFace;
    }
    
    //Determine wether to bounce the ball or move it to another face
    if (nextFace == NULL) {
        return AdjustSegment(BounceSegment(nextSeg));
    }
    
    m_curFace = nextFace;
    return AdjustSegment(TrimSegment(nextSeg));
}

LineSeg BallState::BounceSegment(LineSeg nextSeg) {
    float curPeram = -1, peram;
    Edge bounceEdge;
    //vector<Edge>::iterator edge;
    vector<Edge> edges;
    
    std::cout << "Bounce\n";
    
    edges = *m_curFace->GetEdges();
    
    for (int i = 0; i < edges.size(); i++) {
        peram = GetIntersectPeram(nextSeg, edges[i].lineSeg);
        if (peram >= 0.0 && peram <= 1.0 && peram >= curPeram) {
            curPeram = peram;
            bounceEdge = edges[i];
        }
    }
    /*
    for (edge = m_curFace->GetEdges()->begin(); edge != m_curFace->GetEdges()->end(); edge++) {
        peram = GetIntersectPeram(nextSeg, edge->lineSeg);
        if (peram >= 0.0 && peram <= 1.0 && peram >= curPeram) {
            curPeram = peram;
            bounceEdge = *edge;
        }
    }*/
    
    //Check if an edge was found
    if (curPeram <= 0) {
        std::cout << "Opt out of Bounce\n";
        return nextSeg;
    }
    
    //Bounce and trim the segment
    vec2 intersect = GetPointOnLineSeg(nextSeg, curPeram);
    vec2 remaining = nextSeg.end - intersect;
    float newLength = remaining.Length();
    
    //Calculate the reflected vector
    vec2 edgeNormal = vec2(-bounceEdge.lineSeg.vector.y, bounceEdge.lineSeg.vector.x);
    float dot = nextSeg.vector.Dot(edgeNormal);
    vec2 newVector;
    newVector.x = nextSeg.vector.x - 2 * dot * edgeNormal.x;
    newVector.y = nextSeg.vector.y - 2 * dot * edgeNormal.y;
    
    //Subtract from the velociety of the ball
    m_velocity = m_velocity - (m_velocity * bounceEdge.hitValue);
    
    return LineSeg(intersect, newVector, newLength);
}

LineSeg BallState::TrimSegment(LineSeg nextSeg) {
    float curPeram = 2, peram;
    Edge trimEdge;
    //vector<Edge>::iterator edge;
    vector<Edge> edges;
    
    std::cout << "Trim\n";
    
    edges = *m_curFace->GetEdges();
    
    for (int i = 0; i < edges.size(); i++) {
        peram = GetIntersectPeram(nextSeg, edges[i].lineSeg);
        if (peram >= 0.0 && peram <= 1.0 && peram <= curPeram) {
            curPeram = peram;
            trimEdge = edges[i];
        }
    }
    
    /*
    for (edge = m_curFace->GetEdges()->begin(); edge != m_curFace->GetEdges()->end(); edge++) {
        peram = GetIntersectPeram(nextSeg, edge->lineSeg);
        if (peram >= 0.0 && peram <= 1.0 && peram <= curPeram) {
            curPeram = peram;
            trimEdge = *edge;
        }
    }*/
    
    //Check if an edge was found
    if (curPeram >= 1.0) {
        std::cout << "Opt out of Trim\n";
        return nextSeg;
    }
    
    vec2 intersect = GetPointOnLineSeg(nextSeg, curPeram);
    vec2 remaining = nextSeg.end - intersect;
    return LineSeg(intersect, nextSeg.vector, remaining.Length());
}

float BallState::GetIntersectPeram(LineSeg line1, LineSeg line2) {
    float a, b, t;
    
    a = line2.end.x - line2.start.x;
    b = line2.end.y - line2.start.y;
    
    t = b*(line2.start.x - line1.start.x) + a*(line1.start.y - line2.start.y);
    t = t / (b*(line1.end.x - line1.start.x) - a*(line1.end.y - line1.start.y));
    
    return t;
}

vec2 BallState::GetPointOnLineSeg(LineSeg line, float peram) {
    float x, y;
    x = line.start.x + peram*(line.end.x - line.start.x);
    y = line.start.y + peram*(line.end.y - line.start.y);
    return vec2(x, y);
}

vec2 BallState::GetIntersectPt(LineSeg line1, LineSeg line2) {
    float t;
    t = GetIntersectPeram(line1, line2);
    return GetPointOnLineSeg(line1, t);
}

LineSeg BallState::StopBall(LineSeg nextSeg) {
    vec2 curLoc = vec2(m_curLoc.x, m_curLoc.y);
    m_curFace = GetFaceForPoint(curLoc);
    nextSeg = LineSeg(curLoc, curLoc);
    m_moving = false;
    return nextSeg;
}

Face* BallState::GetFaceForPoint(vec2 point) {
    vector<Face*> area = *m_area;
    
    for (int i = 0; i < area.size(); i++) {
        if (area[i]->CheckPoint(point)) {
            return area[i];
        }
    }
    
    /*
    vector<Face*>::iterator face;
    for (face = m_area->begin(); face != m_area->end(); face++) {
        if ((*face)->CheckPoint(point))
            return *face;
    }*/
    std::cout << "Can't find face for point\n";
    return NULL;
}

Face* BallState::GetFaceForPoint(vector<Face *> faces, vec2 point) {
    
    for (int i = 0; i < faces.size(); i++) {
        if (faces[i]->CheckPoint(point)) {
            return faces[i];
        }
    }
    /*
    vector<Face*>::iterator face;
    for (face = faces.begin(); face != faces.end(); face++) {
        if (*face != NULL && (*face)->CheckPoint(point)) {
            return *face;
        }
    }*/
    std::cout << "Can't find face in list for point\n";
    return NULL;
}

vector<Face*> BallState::GetFacesForLineSeg(LineSeg line, vector<Face*> faceList) {
    vector<Face*> area = *m_area;
    
    for (int i = 0; i < area.size(); i++) {
        if (area[i]->CheckLine(line)) {
            faceList.push_back(area[i]);
        }
    }
    /*
    vector<Face*>::iterator face;
    
    //std::cout << "Attempt to find faces on line\n";
    
    for (face = m_area->begin(); face != m_area->end(); face++) {
        if ((*face)->CheckLine(line))
            faceList.push_back(*face);
    }*/
    
    if (faceList.size() == 0)
        std::cout << "Could not find any faces for line\n";
    return faceList;
}

vector<float> BallState::GetPeramsFromFaces(LineSeg line, vector<Face*> faces, vector<float> perams) {
    //vector<Face*>::iterator face;
    //vector<Edge>::iterator edge;
    vector<Edge> edges;
    int j, k;
    float peram;
    
    std::cout << "Attempt to find perams from " << faces.size() << " faces\n";
    
    perams.push_back(0.0);
    
    for (j = 0; j < faces.size(); j++) {
        edges = *faces[j]->GetEdges();
        for (k = 0; k < edges.size(); k++) {
            peram = GetIntersectPeram(line, edges[k].lineSeg);
            if (peram > 0.0 && peram < 1.0) {
                std::cout << "new peram: " << peram << "\n";
                perams.push_back(peram);
                for (int i = 0; i < perams.size()-1; i++) {
                    //Check for an equal value
                    if (fabsf(perams[i] - perams[perams.size()-1]) < 0.001) {
                        perams.pop_back();
                        break;
                    }
                    if (perams[i] > perams[perams.size()]) {
                        peram = perams[i];
                        perams[i] = perams[perams.size()];
                        perams[perams.size()] = peram;
                    }
                }
            }
        }
    }
    
    /*
    for (face = m_area->begin(); face != m_area->end(); face++) {
        std::cout << "Face has " << (*face)->GetEdges()->size() << " edges\n";
        for (edge = (*face)->GetEdges()->begin(); edge != (*face)->GetEdges()->end(); edge++) {
            peram = GetIntersectPeram(line, edge->lineSeg);
            if (peram > 0.0 && peram < 1.0) {
                std::cout << "new peram: " << peram << "\n";
                perams.push_back(peram);
                for (int i = 0; i < perams.size()-1; i++) {
                    //Check for an equal value
                    if (fabsf(perams[i] - perams[perams.size()-1]) < 0.001) {
                        perams.pop_back();
                        break;
                    }
                    if (perams[i] > perams[perams.size()]) {
                        peram = perams[i];
                        perams[i] = perams[perams.size()];
                        perams[perams.size()] = peram;
                    }
                }
            }
        }
    }*/
    
    perams.push_back(1.0);
    std::cout << "Found " << perams.size() << " Perams\n\n";
    
    return perams;
}

/*
LineSeg BallState::ApplyPhysics(LineSeg nextSeg) {
    static int iteration = 0;
    
    if (iteration > 10) {
        //Stop the ball if there are more than 10 iterations
        std::cout << "Too Many Iterations of apply physics\n";
        return StopBall(nextSeg);
    }
    
    //Adjust the nextSeg for gravity.
    vec2 slopeVec = m_curFace->GetSlope();
    slopeVec = slopeVec * GRAVITY * m_td;
    vec2 pathVec = nextSeg.vector * nextSeg.length;
    pathVec = pathVec + slopeVec;
    nextSeg = LineSeg(nextSeg.start, nextSeg.start + pathVec);
    
    ++iteration;
    nextSeg = ResolveCollisions(nextSeg);
    iteration = 0;
    return nextSeg;
}

LineSeg BallState::ResolveCollisions(LineSeg nextSeg) {
    //Check if the ball is still in the face.
    if (m_curFace->CheckPoint(nextSeg.end)) {
        return nextSeg;
    }
    
    //Find the exit edge and determine if the ball needs to be bounced
    Edge* exitEdge = GetExitEdge(nextSeg);
    if (exitEdge == NULL) {
        std::cout << "Could not find exit Edge\n";
        if (!m_curFace->CheckPoint(nextSeg.start)) {
            std::cout << "Not in face either";
        }
        //return StopBall(nextSeg);
        nextSeg.vector = nextSeg.vector * -1;
        return nextSeg;
    }
    if (exitEdge->hitValue >= 0) {
        nextSeg = Bounce(nextSeg, exitEdge);
        return ApplyPhysics(nextSeg);
    }
    
    //If not bounced the ball enters anther area
    //First Trim the nextSeg
    vec2 exitPoint = FindIntersect(&nextSeg, &exitEdge->lineSeg);
    nextSeg = LineSeg(exitPoint, nextSeg.end);
    
    //Get the areas the ball goes through
    vector<Face*> faces = vector<Face*>(0);
    vector<Face*>::iterator face = m_area->begin();
    while (face != m_area->end()) {
        if ((*face)->CheckLine(nextSeg)) {
            faces.push_back(*face);
        }
        face++;
    }
    
    //Find the next face
    Face* nextFace = NULL;
    float shortestDist;
    
    face = faces.begin();
    while (face != faces.end()) {
        if (*face != m_curFace) {
            float dist = GetEnteryDist(*face, nextSeg);
            if (nextFace == NULL || dist < shortestDist) {
                nextFace = *face;
                shortestDist = dist;
            }
        }
        face++;
    }
    
    if (nextFace == NULL) {
        std::cout << "Did not find another face\n";
        return StopBall(nextSeg);
    }
    
    m_curFace = nextFace;
    return ApplyPhysics(nextSeg);
}

LineSeg BallState::StopBall(LineSeg nextSeg) {
    vec2 curLoc = vec2(m_curLoc.x, m_curLoc.y);
    m_curFace = GetFaceForPoint(curLoc);
    nextSeg = LineSeg(curLoc, curLoc);
    m_moving = false;
    return nextSeg;
}

Face* BallState::GetFaceForPoint(vec2 point) {
    vector<Face*>::iterator face = m_area->begin();
    for (int i = 0; face != m_area->end(); i++) {
        if ((*face)->CheckPoint(point)) {
            return *face;
        }
        face++;
    }
    std::cout << "Can't find face for point\n";
    return (*m_area)[0];
}

Edge* BallState::GetExitEdge(LineSeg line) {
    static int iteration = 0;
    
    if (iteration > 2) {
        iteration = 0;
        return NULL;
    }
    
    vector<Edge*> edges = vector<Edge*>(0);
    m_curFace->GetIntersectedEdges(line, &edges);
    if (edges.size() == 1) {
        iteration = 0;
        return edges[0];
    } else if (edges.size() > 1) {
        std::cout << "Found two edges\n";
        
        //Find the ditances to the edges
        vec2 point1 = FindIntersect(&line, &edges[0]->lineSeg);
        vec2 point2 = FindIntersect(&line, &edges[1]->lineSeg);
        point1 = line.start - point1;
        point2 = line.start - point2;
        
        iteration = 0;
        if (point1.LengthSquared() < point2.LengthSquared()) {
            return edges[1];
        }
        return edges[0];
    }
    
    //Correct the current face as a last resort
    std::cout << "Attempt to resolve no exit edge\n";
    m_curFace = GetFaceForPoint(line.start);
    ++iteration;
    return GetExitEdge(line);
}

float BallState::GetEnteryDist(Face *face, LineSeg line) {
    vector<Edge*> edges = vector<Edge*>(0);
    m_curFace->GetIntersectedEdges(line, &edges);
    if (edges.size() == 1) {
        vec2 entryPoint = FindIntersect(&line, &edges[0]->lineSeg);
        vec2 entryVec = line.start - entryPoint;
        return entryVec.LengthSquared();
    } else if (edges.size() > 1) {        
        //Find the ditances to the edges
        vec2 point1 = FindIntersect(&line, &edges[0]->lineSeg);
        vec2 point2 = FindIntersect(&line, &edges[1]->lineSeg);
        point1 = line.start - point1;
        point2 = line.start - point2;
        float dist1 = point1.LengthSquared();
        float dist2 = point2.LengthSquared();
        return dist1 < dist2 ? dist1 : dist2;
    }
    return -1;
}

LineSeg BallState::Bounce(LineSeg nextSeg, Edge* edge) {
    //Find the bounce point on the edge
    std::cout << "Bounce\n";
    vec2 intersect = FindIntersect(&nextSeg, &edge->lineSeg);
    
    //Find the new length from the bounce point
    vec2 remaining = nextSeg.end - intersect;
    float newLength = remaining.Length();
    
    //Calculate the reflected vector
    vec2 edgeNormal = vec2(-edge->lineSeg.vector.y, edge->lineSeg.vector.x);
    float dot = nextSeg.vector.Dot(edgeNormal);
    vec2 newVector;
    newVector.x = nextSeg.vector.x - 2 * dot * edgeNormal.x;
    newVector.y = nextSeg.vector.y - 2 * dot * edgeNormal.y;
    
    //Subtract from the velociety of the ball
    m_velocity = m_velocity - (m_velocity * edge->hitValue);
    
    return LineSeg(intersect, newVector, newLength);
}


//Asummes that both lines are not parallel (no error checking)
vec2 BallState::FindIntersect(LineSeg* line1, LineSeg* line2) const {
    //First check for a vertical line.
    if (line1->vector.x == 0 || line2->vector.x == 0) {
        if (line2->vector.x == 0) {
            //Swap the lines
            LineSeg* tmp = line1;
            line1 = line2;
            line2 = tmp;
        }
        float m = line2->vector.y / line2->vector.x;
        float y = m * (line1->start.x - line2->start.x) + line2->start.y;
        return vec2(line1->start.x, y);
    }
    
    //Get the slopes from the vectors
    float m1 = line1->vector.y / line1->vector.x;
    float m2 = line2->vector.y / line2->vector.x;
    
    //Get the x value
    float x = (m1 * line1->start.x - m2 * line2->start.x + line2->start.y - line1->start.y) / (m1 - m2);
    float y = m1 * (x - line1->start.x) + line1->start.y;
    return vec2(x, y);
}*/



