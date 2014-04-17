#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ArmContourFinder.h"
#include "ofxKinect.h"
#include <cmath>

#define _USE_LIVE_VIDEO		// uncomment this to use live kinect data

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed(int key);

		// Variables for input processing
		int 		nearThreshold;
		int 		farThreshold;

		ofxCvGrayscaleImage		inputImg;	// Raw input form kinect
		ofxCvGrayscaleImage		threshImg;	// After thresholding
		ofxCvColorImage			colorImg;	// Because video

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

		vector< ofPoint > findMostDistantPoints(ofPolyline line);

		// For text
		ofTrueTypeFont myfont;

		// For video
		#ifdef _USE_LIVE_VIDEO
		ofxKinect 	kinect;
		#else
		ofVideoPlayer	kinect;
		#endif

		// For getting water region
		void mousePressed(int x, int y, int button);
		ofPolyline waterRegion;

		// For displaying the proper texts
		const char *riverNames [5];
		vector < ofPolyline > riverRegions;
		
};
