#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ArmContourFinder.h"
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

		ofxCvGrayscaleImage		inputImg;	// Raw input form kinect
		ofxCvGrayscaleImage		threshImg;	// After thresholding

		ofImage background;

		ArmContourFinder		contourFinder;

		void drawHandOverlay();
		void drawLabels();

		//For Calibration
		int DX, DY;
		float ZOOM, DEGREES;

		int handSmoother;
		float scaleup;

		
		// Just for showing raw
		int dispMode;
		
};
