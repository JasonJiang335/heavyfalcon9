
//--------------------------------------------------------------
//
//  Kevin M. Smith 
//
//  Mars HiRise Project - startup scene
// 
//  This is an openFrameworks 3D scene that includes an EasyCam
//  and example 3D geometry which I have reconstructed from Mars
//  HiRis photographs taken the Mars Reconnaisance Orbiter
//
//  You will use this source file (and include file) as a starting point
//  to implement assignment 5  (Parts I and II)
//
//  Please do not modify any of the keymappings.  I would like 
//  the input interface to be the same for each student's 
//  work.  Please also add your name/date below.

//  Please document/comment all of your work !
//  Have Fun !!
//
//  Student Name:   < Your Name goes Here >
//  Date: <date of last version>


#include "ofApp.h"
#include "Util.h"

//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){

	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bRoverLoaded = false;
	bTerrainSelected = true;
	cam.setDistance(1000);
	cam.setNearClip(.1);
    cam.setFarClip(10000000000.0);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
    cam.lookAt(falcon.position);
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();
    
	// setup rudimentary lighting 
	//
	initLightingAndMaterials();
    
    // texture loading
    //
    ofDisableArbTex();     // disable rectangular textures
    
    // load textures
    //
    if (!ofLoadImage(particleTex, "images/dot.png")) {
        cout << "Particle Texture File: images/dot.png not found" << endl;
        ofExit();
    }
    
    // load the shader
    //
    #ifdef TARGET_OPENGLES
        shader.load("shaders_gles/shader");
    #else
        shader.load("shaders/shader");
    #endif

	mars.loadModel("geo/mars-landing-original.obj");
	mars.setScaleNormalization(false);
    falcon.rocket.loadModel("geo/Heavy_falcon.obj");
    falcon.rocket.setScaleNormalization(false);
    falcon.forceList.push_back(glm::vec3(0,-1,0));
    
	boundingBox = meshBounds(mars.getMesh(0));
	octree.create(mars.getMesh(0), 100);
    
    turbForce = new TurbulenceForce(ofVec3f(turbMin->x, turbMin->y, turbMin->z), ofVec3f(turbMax->x, turbMax->y, turbMax->z));
    gravityForce = new GravityForce(ofVec3f(0, -gravity, 0));
    
    exhaustL.sys->addForce(turbForce);
    exhaustL.sys->addForce(gravityForce);
    exhaustL.setVelocity(ofVec3f(0,-200,0));
    exhaustL.setOneShot(true);
    exhaustL.setEmitterType(DirectionalEmitter);
    exhaustL.setGroupSize(numParticles);
    exhaustL.setRandomLife(true);
    exhaustL.setLifespanRange(ofVec2f(lifespanRange->x, lifespanRange->y));
    exhaustL.setPosition(falcon.rocket.getSceneMin()+falcon.rocket.getPosition() + exhaustOffsetL);

    exhaustM.sys->addForce(turbForce);
    exhaustM.sys->addForce(gravityForce);
    exhaustM.setVelocity(ofVec3f(0,-200,0));
    exhaustM.setOneShot(true);
    exhaustM.setEmitterType(DirectionalEmitter);
    exhaustM.setGroupSize(numParticles);
    exhaustM.setRandomLife(true);
    exhaustM.setLifespanRange(ofVec2f(lifespanRange->x, lifespanRange->y));
    exhaustM.setPosition(falcon.rocket.getSceneMin()+falcon.rocket.getPosition() + exhaustOffsetM);

    exhaustR.sys->addForce(turbForce);
    exhaustR.sys->addForce(gravityForce);
    exhaustR.setVelocity(ofVec3f(0,-200,0));
    exhaustR.setOneShot(true);
    exhaustR.setEmitterType(DirectionalEmitter);
    exhaustR.setGroupSize(numParticles);
    exhaustR.setRandomLife(true);
    exhaustR.setLifespanRange(ofVec2f(lifespanRange->x, lifespanRange->y));
    exhaustR.setPosition(falcon.rocket.getSceneMin()+falcon.rocket.getPosition() + exhaustOffsetR);
    
    // some simple sliders to play with parameters
    //
    gui.setup();
    gui.add(numParticles.setup("Number of Particles", 200, 0, 25000));
    gui.add(lifespanRange.setup("Lifespan Range", ofVec2f(0.5, 1), ofVec2f(.1, .2), ofVec2f(3, 10)));
    gui.add(mass.setup("Mass", 1, .1, 10));
    gui.add(damping.setup("Damping", .99, .8, 1.0));
    gui.add(gravity.setup("Gravity", 1, -20, 20));
    gui.add(radius.setup("Radius", 20, 1, 100));
    gui.add(turbMin.setup("Turbulence Min", ofVec3f(-7, 0, -7), ofVec3f(-20, -20, -20), ofVec3f(20, 20, 20)));
    gui.add(turbMax.setup("Turbulence Max", ofVec3f(7, -20, 7), ofVec3f(-20, -20, -20), ofVec3f(20, 20, 20)));
    
    topCam.setPosition(falcon.position + glm::vec3(100, 300, 100));
    topCam.lookAt(falcon.position);
    topCam.setNearClip(.1);
    topCam.setFarClip(10000000000.0);
    topCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
    
    sideCam.setPosition(falcon.position + glm::vec3(0,0,1500));
    sideCam.lookAt(falcon.position);
    sideCam.setNearClip(.1);
    sideCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
    
    fixCam.setPosition(20, 0, 0);
    fixCam.lookAt(glm::vec3(0, 0, 0));
    fixCam.setNearClip(.1);
    fixCam.setFov(65.5);
    
    theCam = &cam;
}

// load vertex buffer in preparation for rendering
//
void ofApp::loadVboL() {
    if (exhaustL.sys->particles.size() < 1) return;
    
    vector<ofVec3f> sizes;
    vector<ofVec3f> points;
    for (int i = 0; i < exhaustL.sys->particles.size(); i++) {
        points.push_back(exhaustL.sys->particles[i].position);
        sizes.push_back(ofVec3f(radius));
    }
    // upload the data to the vbo
    //
    int total = (int)points.size();
    vboL.clear();
    vboL.setVertexData(&points[0], total, GL_STATIC_DRAW);
    vboL.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

void ofApp::loadVboM() {
    if (exhaustM.sys->particles.size() < 1) return;
    
    vector<ofVec3f> sizes;
    vector<ofVec3f> points;
    for (int i = 0; i < exhaustM.sys->particles.size(); i++) {
        points.push_back(exhaustM.sys->particles[i].position);
        sizes.push_back(ofVec3f(radius));
    }
    // upload the data to the vbo
    //
    int total = (int)points.size();
    vboM.clear();
    vboM.setVertexData(&points[0], total, GL_STATIC_DRAW);
    vboM.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

void ofApp::loadVboR() {
    if (exhaustR.sys->particles.size() < 1) return;
    
    vector<ofVec3f> sizes;
    vector<ofVec3f> points;
    for (int i = 0; i < exhaustR.sys->particles.size(); i++) {
        points.push_back(exhaustR.sys->particles[i].position);
        sizes.push_back(ofVec3f(radius));
    }
    // upload the data to the vbo
    //
    int total = (int)points.size();
    vboR.clear();
    vboR.setVertexData(&points[0], total, GL_STATIC_DRAW);
    vboR.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
    ofSeedRandom();
    exhaustL.setPosition(falcon.rocket.getSceneMin()+falcon.rocket.getPosition() + exhaustOffsetL);
    exhaustL.setParticleRadius(radius);
    exhaustL.setLifespanRange(ofVec2f(lifespanRange->x, lifespanRange->y));
    exhaustL.setMass(mass);
    exhaustL.setDamping(damping);
    exhaustL.setGroupSize(numParticles);
    exhaustL.update();

    exhaustM.setPosition(falcon.rocket.getSceneMin()+falcon.rocket.getPosition() + exhaustOffsetM);
    exhaustM.setParticleRadius(radius);
    exhaustM.setLifespanRange(ofVec2f(lifespanRange->x, lifespanRange->y));
    exhaustM.setMass(mass);
    exhaustM.setDamping(damping);
    exhaustM.setGroupSize(numParticles);
    exhaustM.update();

    exhaustR.setPosition(falcon.rocket.getSceneMin()+falcon.rocket.getPosition() + exhaustOffsetR);
    exhaustR.setParticleRadius(radius);
    exhaustR.setLifespanRange(ofVec2f(lifespanRange->x, lifespanRange->y));
    exhaustR.setMass(mass);
    exhaustR.setDamping(damping);
    exhaustR.setGroupSize(numParticles);
    exhaustR.update();
    
    gravityForce->set(ofVec3f(0, -gravity, 0));
    turbForce->set(ofVec3f(turbMin->x, turbMin->y, turbMin->z), ofVec3f(turbMax->x, turbMax->y, turbMax->z));
    
    falcon.update();
    glm::vec3 roverPos = falcon.rocket.getPosition();
    glm::vec3 roverLow = falcon.rocket.getSceneMin() + falcon.rocket.getPosition();
    if(roverLow.y < contactPoint().y){
        roverPos.y = contPt.y - falcon.rocket.getSceneMin().y;
        falcon.velocity = glm::vec3(0,0,0);
        falcon.acceleration = glm::vec3(0,0,0);
        falcon.forceList.clear();
    }
    falcon.rocket.setPosition(roverPos.x, roverPos.y, roverPos.z);
    
    //update cam position
    topCam.setPosition(falcon.position + glm::vec3(100, 300, 100));
    topCam.lookAt(falcon.position);
    sideCam.setPosition(falcon.position + glm::vec3(0,0,1500));
    sideCam.lookAt(falcon.position);
    /*if(pathPoint.size() > 1 && !bCamMotion){
        pathCam.setPosition(pathPoint[0].x, pathPoint[0].y + 0.5, pathPoint[0].z);
        pathCam.lookAt(pathPoint[1]);
    }
    else if(pathPoint.size() > 1 && bCamMotion){
        for(int i = 0; i < pathPoint.size()-1; i++){
            if(inBetweenPoints(pathCam.getPosition(), pathPoint[i], pathPoint[i+1])){
                glm::vec3 delta = pathPoint[i+1] - pathPoint[i];
                glm::vec3 oldCamPos = pathCam.getPosition();
                pathCam.setPosition(oldCamPos + delta/1500);
                spotCam.setPosition(oldCamPos + delta/1500);
                if(bRoverLoaded){
                    glm::vec3 rPos = rover.getPosition();
                    spotCam.lookAt(glm::vec3(rPos.x, rPos.y+0.5, rPos.z));
                }
                pathCam.lookAt(glm::vec3(pathPoint[i+1].x, pathPoint[i+1].y+0.5, pathPoint[i+1].z));
                oldCamPos = pathCam.getPosition();
            }
        }
    }*/
}

/*bool ofApp::inBetweenPoints(const glm::vec3 & camPos, const glm::vec3 & p1, const glm::vec3 & p2){
    if(abs((camPos.x - p1.x)/(p2.x - p1.x) - (camPos.y - 0.5 - p1.y)/(p2.y - p1.y)) < 0.01 &&
       abs((camPos.x - p1.x)/(p2.x - p1.x) - (camPos.z - p1.z)/(p2.z - p1.z)) < 0.01 &&
       abs((camPos.z - p1.z)/(p2.z - p1.z) - (camPos.y - 0.5 - p1.y)/(p2.y - p1.y)) < 0.01)
        return true;
    return false;
}*/
//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0,0,30);
    gui.draw();
    ofEnableLighting();
    loadVboL();
    loadVboM();
    loadVboR();
	theCam->begin();
	ofPushMatrix();
    
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
        falcon.rocket.drawWireframe();
        //if (!bTerrainSelected) drawAxis(rover.getPosition());
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();
        //glm::vec3 pos = falcon.position;
        //falcon.rocket.setPosition(pos.x, pos.y, pos.z);
        falcon.rocket.drawFaces();
        ofSetLineWidth(5);
        ofDrawLine(falcon.rocket.getSceneMin()+falcon.rocket.getPosition(), contactPoint());
        ofDrawSphere(contactPoint(), 10);
        ofSetLineWidth(1);
        
        glDepthMask(GL_FALSE);
        // this makes everything look glowy :)
        //
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        ofEnablePointSprites();
        // begin drawing in the camera
        //
        shader.begin();
        particleTex.bind();
        ofSetColor(255, 165, 70);
        vboL.draw(GL_POINTS, 0, (int)exhaustL.sys->particles.size());
        vboM.draw(GL_POINTS, 0, (int)exhaustM.sys->particles.size());
        vboR.draw(GL_POINTS, 0, (int)exhaustR.sys->particles.size());
        particleTex.unbind();
        ofSetColor(255,255,255);
        shader.end();
	}

	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}
    
	if(bDrawOctree){
        octree.draw(octree.root, 10, 0);
    }
    
    if(bDrawLeafNode){
        octree.drawLeafNodes(octree.root);
    }

	ofNoFill();
	ofSetColor(ofColor::white);
	drawBox(boundingBox);

	theCam->end();
    ofDisablePointSprites();
    ofDisableBlendMode();
    ofEnableAlphaBlending();
    
    // set back the depth mask
    //
    glDepthMask(GL_TRUE);
    
    // draw screen data
    //
    string fps, v, alt;
    fps += "Frame Rate: " + std::to_string(ofGetFrameRate());
    v += "Velocity: " + std::to_string(falcon.velocity.y);
    glm::vec3 p1 = falcon.rocket.getSceneMin()+falcon.rocket.getPosition();
    alt += "Altitude: " + std::to_string(glm::distance(p1, contactPoint())/100);
    ofSetColor(ofColor::white);
    ofDrawBitmapString(fps, ofGetWindowWidth() -170, 15);
    ofDrawBitmapString(v, ofGetWindowWidth() -170, 30);
    ofDrawBitmapString(alt, ofGetWindowWidth() -170, 45);
}

// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(2.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
    ofSetLineWidth(1.0);
}


void ofApp::keyPressed(int key) {

	switch (key) {
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'r':
		cam.reset();
		break;
	case 's':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
    case 'x':
            bDisplayCam = !bDisplayCam;
        break;
	case 'z':
        bSelectPathMode = !bSelectPathMode;
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'w':
		toggleWireframeMode();
		break;
	case '1':
        bDrawOctree = !bDrawOctree;
        break;
    case '2':
        bDrawLeafNode = !bDrawLeafNode;
        break;
    case OF_KEY_DEL:
        if(bSelectPathMode && pathPoint.size() > 0){
            pathPoint.pop_back();
        }
            
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_F1:
        cam.lookAt(falcon.position);
		theCam = &cam;
		break;
	case OF_KEY_F2:
		theCam = &topCam;
		break;
    case OF_KEY_F3:
        theCam = &sideCam;
        break;
    case OF_KEY_F4:
        theCam = &fixCam;
        break;
    case ' ':
        //bCamMotion = !bCamMotion;
        exhaustL.sys->reset();
        exhaustL.start();
        exhaustM.sys->reset();
        exhaustM.start();
        exhaustR.sys->reset();
        exhaustR.start();
        falcon.velocity += glm::vec3(0,1,0);
        falcon.forceList.push_back(glm::vec3(0,0.5,0));
        break;
    case OF_KEY_LEFT:
            moveFalcon(MoveLeft);
        break;
    case OF_KEY_RIGHT:
            moveFalcon(MoveRight);
        break;
    default:
		break;
	}
}

void ofApp::moveFalcon(MoveDir dir) {
    falcon.moveDir = dir;
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
            
    case ' ':
            falcon.forceList.clear();
            falcon.acceleration = glm::vec3(0,0,0);
            falcon.forceList.push_back(glm::vec3(0,-10,0));
        break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

    if(doOctreeSelection() && bSelectPathMode){
        glm::vec3 glmSelectedPoint(selectedPoint.x, selectedPoint.y, selectedPoint.z);
        pathPoint.push_back(glmSelectedPoint);
    }

	// if rover is loaded, test for selection
	//
	/*if (bRoverLoaded) {
		glm::vec3 origin = theCam->getPosition();
		glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = rover.getSceneMin() + rover.getPosition();
		ofVec3f max = rover.getSceneMax() + rover.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bRoverSelected = true;
			mouseDownPos = getMousePointOnPlane();
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bRoverSelected = false;
		}
	}*/
}


//draw a box from a "Box" class  
//
void ofApp::drawBox(const Box &box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box ofApp::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}



//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	if (cam.getMouseInputEnabled()) return;

	if (bInDrag) {
		glm::vec3 mousePos = getMousePointOnPlane();
		glm::vec3 delta = mousePos - mouseLastPos;
		glm::vec3 roverPos = falcon.rocket.getPosition();
		roverPos += delta;
        glm::vec3 roverLow = falcon.rocket.getSceneMin()+falcon.rocket.getPosition();
        if(roverLow.y < contactPoint().y)
            roverPos.y = contPt.y - falcon.rocket.getSceneMin().y;
        
		falcon.rocket.setPosition(roverPos.x, roverPos.y, roverPos.z);
		mouseLastPos = mousePos;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}

glm::vec3 ofApp::contactPoint(){
    glm::vec3 roverMin = falcon.rocket.getSceneMin() + falcon.rocket.getPosition();
    glm::vec3 roverDir(0,-1,0);
    Ray roverRay(Vector3(roverMin.x, roverMin.y, roverMin.z), Vector3(roverDir.x, roverDir.y, roverDir.z));
        
    TreeNode retNode;
    bool hit = octree.intersect(roverRay, octree.root, retNode);
    if(hit){
        Vector3 ret = retNode.box.center();
        contPt = glm::vec3(ret.x(), ret.y(), ret.z());
    }
    return contPt;
}

bool ofApp::doOctreeSelection() {
    bPointSelected = false;
    glm::vec3 origin = theCam->getPosition();
    glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
    glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
    Ray selectionRay(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z));
    
    TreeNode retNode;
    
    bool hit = octree.intersect(selectionRay, octree.root, retNode);
    if(hit){
        bPointSelected = true;
        Vector3 ret = retNode.box.center();
        selectedPoint = ofVec3f(ret.x(), ret.y(), ret.z());
        //cout << selectedPoint << endl;
    }
    return bPointSelected;
}

// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	/*if (rover.loadModel(dragInfo.files[0])) {
		bRoverLoaded = true;
		rover.setScaleNormalization(false);
	//	rover.setScale(.5, .5, .5);
		rover.setPosition(0, 0, 0);

		// We want to drag and drop a 3D object in space so that the model appears 
		// under the mouse pointer where you drop it !
		//
		// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
		// once we find the point of intersection, we can position the rover/lander
		// at that location.
		//

		// Setup our rays
		//
		glm::vec3 origin = theCam->getPosition();
		glm::vec3 camAxis = theCam->getZAxis();
		glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the rover's origin at that intersection point
			//
		    glm::vec3 min = rover.getSceneMin();
			glm::vec3 max = rover.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			rover.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for rover while we are at it
			//
			roverBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}
	
*/
}


//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(){//glm::vec3 point, glm::vec3 planeNormal) {
	// Setup our rays
	//
	glm::vec3 origin = theCam->getPosition();
	glm::vec3 camAxis = theCam->getZAxis();
	glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;
		
		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}
