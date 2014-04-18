#pragma once

#define KINECT_CROP_LEFT 28
#define KINECT_CROP_RIGHT 10
#define KINECT_CROP_TOP 14
#define KINECT_CROP_BOTTOM 27

#define VIDEO_X -78
#define VIDEO_Y -82
#define VIDEO_W 1884
#define VIDEO_H 1058
#define VIDEO_R 1

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxKinect.h"
#include <cmath>

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed(int key);
		void mousePressed(int x, int y, int button);

		ofxKinect kinect;

		ofVideoPlayer video;		

		ofImage		rawKinectInput;
		ofImage		rawVideoInput;
		
};