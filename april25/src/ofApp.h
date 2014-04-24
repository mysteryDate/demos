#pragma once

// Calibration variables
// -----------------------------------
// Cropping out kinect data
#define KINECT_CROP_LEFT 28
#define KINECT_CROP_RIGHT 10
#define KINECT_CROP_TOP 14
#define KINECT_CROP_BOTTOM 27

// Transforming kinect data to fit "real" world
#define INPUT_DATA_ZOOM 2.57
#define INPUT_DATA_DX 40
#define	INPUT_DATA_DY -64
#define INPUT_DATA_R 0

// Transforming video data to fit maquette
#define VIDEO_X -78
#define VIDEO_Y -82
#define VIDEO_W 1884
#define VIDEO_H 1058
#define VIDEO_R 1
// -----------------------------------

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxKinect.h"
#include "ArmContourFinder.h"
#include "ofxRipples.h"
#include "ofxBounce.h" 
#include <cmath>

class ofApp : public ofBaseApp{

	public:
		void setup();
		void transformInput();
		void fillInRiverRegions();

		void update();
		void updateRipples();
		void updateHands();

		void draw();
		void drawHandOverlay();
		void drawFeedback();

		void exit();

		void keyPressed(int key);
		void mousePressed(int x, int y, int button);

		// Test input


		// Input Processing
		ofxKinect 			kinect;
		ofVideoPlayer 		video;		
		cv::Mat 			input;
		cv::Mat 			croppedInput;
		ofxCvGrayscaleImage	kinectImg;
		int 				nearThreshold;
		int 				farThreshold;

		// Cv
        ArmContourFinder	contourFinder;
        struct Hand
        {
        	ofPolyline line;
        	ofPoint centroid;
        	ofPoint tip;
        	vector< ofPoint > ends;
        	vector< ofPoint > wrists;
        	int index;
        	unsigned int label;

        	//For sorting by label
        	bool operator < (const Hand& str) const
        	{
        		return (label < str.label);
        	}
        };
        vector< Hand >		hands;

		// Display variables
		int 				lineSmoothing;
		float 				armScaleUp;


		// For defining regions
		ofPolyline			waterRegion;
		vector< ofPolyline>	riverRegions;
		const char * 		riverNames[5];
		ofTrueTypeFont		myfont;

		// For water effect
		ofxRipples	ripples;
		ofxBounce 	bounce;
		ofImage 	riverMask;

		// Calibration
		float x, y, w, h, r;
		// Feedback
		bool bFeedback;

};

