
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
//	ofSetWindowShape(1024, 768);
	cam.setDistance(10);
	cam.setNearClip(.1);
    cam.setFarClip(100000.0);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();

	top.setPosition(0, 25, 0);
	top.lookAt(glm::vec3(0, 0, 0));
	top.setNearClip(.1);
	top.setFov(65.5);   // approx equivalent to 28mm in 35mm format

    pathCam.setPosition(20, 0, 0);
    pathCam.lookAt(glm::vec3(0, 0, 0));
    pathCam.setNearClip(.1);
    pathCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
    
    spotCam.setPosition(20, 0, 0);
    spotCam.lookAt(glm::vec3(0, 0, 0));
    spotCam.setNearClip(.1);
    spotCam.setFov(65.5);
    
	theCam = &cam;

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	mars.loadModel("geo/mars-landing-old.obj");
	mars.setScaleNormalization(false);
	boundingBox = meshBounds(mars.getMesh(0));
	octree.create(mars.getMesh(0), 100);
}

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
    if(pathPoint.size() > 1 && !bCamMotion){
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
    }
}

bool ofApp::inBetweenPoints(const glm::vec3 & camPos, const glm::vec3 & p1, const glm::vec3 & p2){
    if(abs((camPos.x - p1.x)/(p2.x - p1.x) - (camPos.y - 0.5 - p1.y)/(p2.y - p1.y)) < 0.01 &&
       abs((camPos.x - p1.x)/(p2.x - p1.x) - (camPos.z - p1.z)/(p2.z - p1.z)) < 0.01 &&
       abs((camPos.z - p1.z)/(p2.z - p1.z) - (camPos.y - 0.5 - p1.y)/(p2.y - p1.y)) < 0.01)
        return true;
    return false;
}
//--------------------------------------------------------------
void ofApp::draw(){

	ofBackground(0,0,30);
    //ofEnableLighting();

	theCam->begin();

	ofPushMatrix();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bRoverLoaded) {
			rover.drawWireframe();
			if (!bTerrainSelected) drawAxis(rover.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();

		if (bRoverLoaded) {
			rover.drawFaces();
			if (!bTerrainSelected) drawAxis(rover.getPosition());

			ofVec3f min = rover.getSceneMin() + rover.getPosition();
			ofVec3f max = rover.getSceneMax() + rover.getPosition();

			Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

			if (bRoverSelected) {
				ofSetColor(ofColor::green);
			}
			else {
				ofSetColor(ofColor::white);
			}
			drawBox(bounds);
            ofDrawLine(rover.getSceneMin()+rover.getPosition(), contactPoint());
            ofDrawSphere(contactPoint(), 10);
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}


	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, 10);
		ofSetColor(255,255,255);
	}
        
    //draw point path
    if(pathPoint.size() > 0){
        auto iter = pathPoint.begin();
        for(;iter != pathPoint.end(); iter++){
            ofSetColor(ofColor::green);
            ofDrawSphere((*iter), .1);
        }
        ofSetColor(255,255,255);
    }
    
    if(pathPoint.size() > 1){
        ofSetLineWidth(5);
        for(int i = 0; i < pathPoint.size()-1; i++){
            ofSetColor(ofColor::green);
            ofDrawLine(pathPoint[i].x, pathPoint[i].y+0.2, pathPoint[i].z, pathPoint[i+1].x, pathPoint[i+1].y+0.2, pathPoint[i+1].z);
        }
        if(bDisplayCam){
            ofSetColor(255,255,255);
            pathCam.setScale(0.2);
            pathCam.draw();
        }
        ofSetLineWidth(1);
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
}

// 

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
		theCam = &cam;
		break;
	case OF_KEY_F2:
		theCam = &top;
		break;
    case OF_KEY_F3:
        theCam = &pathCam;
        break;
    case OF_KEY_F4:
        theCam = &spotCam;
        break;
    case ' ':
        bCamMotion = !bCamMotion;
        break;
    default:
		break;
	}
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
	if (bRoverLoaded) {
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
	}
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
		glm::vec3 roverPos = rover.getPosition();
		roverPos += delta;
        glm::vec3 roverLow = rover.getSceneMin()+rover.getPosition();
        if(roverLow.y < contactPoint().y)
            roverPos.y = contPt.y - rover.getSceneMin().y;
        
		rover.setPosition(roverPos.x, roverPos.y, roverPos.z);
		mouseLastPos = mousePos;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}

glm::vec3 ofApp::contactPoint(){
    if(bRoverLoaded){
        glm::vec3 roverMin = rover.getSceneMin() + rover.getPosition();
        glm::vec3 roverDir(0,-1,0);
        Ray roverRay(Vector3(roverMin.x, roverMin.y, roverMin.z), Vector3(roverDir.x, roverDir.y, roverDir.z));
        
        TreeNode retNode;
        bool hit = octree.intersect(roverRay, octree.root, retNode);
        if(hit){
            Vector3 ret = retNode.box.center();
            contPt = glm::vec3(ret.x(), ret.y(), ret.z());
        }
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
        cout << selectedPoint << endl;
    }
    return bPointSelected;
}

//
//  ScreenSpace Selection Method: 
//  This is not the octree method, but will give you an idea of comparison
//  of speed between octree and screenspace.
//
//  Select Target Point on Terrain by comparing distance of mouse to 
//  vertice points projected onto screenspace.
//  if a point is selected, return true, else return false;
//
bool ofApp::doPointSelection() {

	ofMesh mesh = mars.getMesh(0);
	int n = mesh.getNumVertices();
	float nearestDistance = 0;
	int nearestIndex = 0;

	bPointSelected = false;

	ofVec2f mouse(mouseX, mouseY);
	vector<ofVec3f> selection;

	// We check through the mesh vertices to see which ones
	// are "close" to the mouse point in screen space.  If we find 
	// points that are close, we store them in a vector (dynamic array)
	//
	for (int i = 0; i < n; i++) {
		ofVec3f vert = mesh.getVertex(i);
		ofVec3f posScreen = cam.worldToScreen(vert);
		float distance = posScreen.distance(mouse);
		if (distance < selectionRange) {
			selection.push_back(vert);
			bPointSelected = true;
		}
	}

	//  if we found selected points, we need to determine which
	//  one is closest to the eye (camera). That one is our selected target.
	//
	if (bPointSelected) {
		float distance = 0;
		for (int i = 0; i < selection.size(); i++) {
			ofVec3f point =  cam.worldToCamera(selection[i]);

			// In camera space, the camera is at (0,0,0), so distance from 
			// the camera is simply the length of the point vector
			//
			float curDist = point.length(); 

			if (i == 0 || curDist < distance) {
				distance = curDist;
				selectedPoint = selection[i];
			}
		}
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
	if (rover.loadModel(dragInfo.files[0])) {
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
