//
//  Falcon.cpp
//  terrainTest
//
//  Created by Jason Jiang on 5/3/19.
//

#include "Falcon.h"
/*
 
 enum MoveDir { MoveStop, MoveLeft, MoveRight, MoveUp, MoveDown };
 class Falcon{
 public:
 void draw();
 void update();
 void integrate();
 
 glm::vec3 position;
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
 */

void Falcon::update() {
    totalForce = glm::vec3(0,0,0);
    for(int i = 0; i < forceList.size(); i++)
        totalForce += forceList[i];
    
    float dist = speed * 1 / ofGetFrameRate();
    glm::vec3 dir;
    switch (this->moveDir)
    {
        case MoveUp:
            //dir = glm::vec3(0, -dist, 0);
            break;
        case MoveDown:
            //dir = glm::vec3(0, dist, 0);
            break;
        case MoveLeft:
            dir = glm::vec3(-dist, 0, 0);
            break;
        case MoveRight:
            dir = glm::vec3(dist, 0, 0);
            break;
        case MoveStop:
            //dir = glm::vec3(0, 0, 0);
            break;
    }
    
    integrate();
    position += dir;
    if(velocity.y > 50)
        velocity.y = 50;
    if(velocity.y < -50)
        velocity.y = -50;
    rocket.setPosition(position.x, position.y, position.z);
}

void Falcon::draw(){
    rocket.drawFaces();
}

void Falcon::drawWire() {
    rocket.drawWireframe();
}

void Falcon::integrate() {
    // interval for this step
    //
    float dt = 1.0; /// ofGetFrameRate();
    // update position based on velocity
    //
    position += (velocity * dt);
    
    // update acceleration with accumulated paritcles forces
    // remember :  (f = ma) OR (a = 1/m * f)
    //
    glm::vec3 accel = acceleration;    // start with any acceleration already on the particle
    accel += (totalForce * (1.0 / mass));
    //cout << "accel: " << acceleration << endl;
    velocity += accel * dt;
    
    // add a little damping for good measure
    //
    velocity *= damping;
    //cout << "velocity: " << velocity << endl;
    // clear forces on particle (they get re-added each step)
    //
    totalForce = glm::vec3(0, 0, 0);
}
