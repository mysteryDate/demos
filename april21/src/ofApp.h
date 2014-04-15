#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxKinect.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed(int key);

		// Variables for input processing
		ofxKinect 	kinect;
		int 		nearThreshold;
		int 		farThreshold;
		
};
