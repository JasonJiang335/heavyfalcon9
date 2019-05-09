//
//  Falcon.hpp
//  terrainTest
//
//  Created by Jason Jiang on 5/3/19.
//
#pragma once

#include "ofMain.h"
#include  "ofxAssimpModelLoader.h"
#include "box.h"
#include "ray.h"
#include "Octree.h"

enum MoveDir { MoveStop, MoveLeft, MoveRight, MoveUp, MoveDown };
class Falcon{
public:
    void draw();
    void drawWire();
    void update();
    void integrate();
    
    glm::vec3 position = glm::vec3(0,500,0);
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 totalForce;
    vector<glm::vec3> forceList;
    
    ofxAssimpModelLoader rocket;
    float speed = 0;
    float damping = 0.99;
    float mass = 1000;
    MoveDir moveDir;
};
