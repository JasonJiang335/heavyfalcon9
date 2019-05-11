#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "box.h"
#include "ray.h"
#include "Octree.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include "Falcon.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
        void loadVboL();
        void loadVboM();
        void loadVboR();
		glm::vec3 getMousePointOnPlane();
		bool  doPointSelection();
        bool doOctreeSelection();
        bool inBetweenPoints(const glm::vec3 & camPos, const glm::vec3 & p1, const glm::vec3 & p2);
		void drawBox(const Box &box);
		Box meshBounds(const ofMesh &);
        glm::vec3 contactPoint();
    void moveFalcon(MoveDir dir);
		

    ofImage  backgroundImage;
		ofEasyCam cam;
		ofCamera topCam;
        ofCamera sideCam;
        ofCamera fixCam;
    ofCamera obSideCam;
    ofCamera obDownCam;
		ofCamera *theCam;
    ofxAssimpModelLoader mars;
        Falcon falconMain;
    Falcon falconLeft;
    Falcon falconRight;
		ofLight light;
		Box boundingBox;
		Box roverBounds;
    
    bool bBackgroundLoaded = false;
		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
        bool bDisplayCam;
        bool bCamMotion;
		bool bPointSelected;
		bool bDrawOctree;
    	bool bDrawLeafNode;
    
		bool bRoverLoaded = false;
		bool bTerrainSelected;
		bool bRoverSelected = false;
		bool bInDrag = false;
        bool bSelectPathMode = false;
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;
		
    glm::vec3 exhaustOffsetL = glm::vec3(18,0,18);
    glm::vec3 exhaustOffsetM = glm::vec3(50,0,18);
    glm::vec3 exhaustOffsetR = glm::vec3(80,0,18);
    glm::vec3 falconOffset = glm::vec3(30,0,0);
        glm::vec3 contPt;
		glm::vec3 mouseDownPos;
		glm::vec3 mouseLastPos;
        vector<glm::vec3> pathPoint;
        Octree octree;
		const float selectionRange = 4.0;
    
        ParticleEmitter exhaustL;
        ParticleEmitter exhaustM;
        ParticleEmitter exhaustR;
        // textures
        //
        ofTexture  particleTex;
    
        // shaders
        //
        ofVbo vboL;
        ofVbo vboM;
        ofVbo vboR;
        ofShader shader;
    
    TurbulenceForce *turbForce;
    GravityForce *gravityForce;
    
    ofxPanel gui;
    ofxFloatSlider gravity;
    ofxFloatSlider damping;
    ofxFloatSlider radius;
    ofxVec3Slider velocity;
    ofxIntSlider numParticles;
    ofxFloatSlider lifespan;
    ofxVec2Slider lifespanRange;
    ofxVec3Slider turbMin;
    ofxVec3Slider turbMax;
    ofxFloatSlider mass;
    
};
