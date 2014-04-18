#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	kinect.init();
	kinect.open();
	kinectImg.allocate(kinect.width, kinect.height);
	processedImg.allocate(kinect.width*INPUT_DATA_ZOOM, kinect.height*INPUT_DATA_ZOOM);

	video.loadMovie("MapBasicV1.mov");
	video.play();

	nearThreshold = 250;
	farThreshold = 100;

	x = 0;
	y = 0;
	w = 1;
	h = 1;
	r = 0;

}

//--------------------------------------------------------------
void ofApp::update(){

	kinect.update();
	video.update();

	videoImg.setFromPixels(video.getPixels(), video.width, video.height, OF_IMAGE_COLOR);


	if(kinect.isFrameNew()) {

		kinectImg.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		transformInput();

	}

}

void ofApp::transformInput()
{
	// Crop the image and threshold
	unsigned char * inPix = kinectImg.getPixels();
	unsigned char * outPix = processedImg.getPixels();

	int width = kinectImg.getWidth() - KINECT_CROP_LEFT - KINECT_CROP_RIGHT;
	int height = kinectImg.getHeight() - KINECT_CROP_RIGHT - KINECT_CROP_BOTTOM;

	int j = 0;
	for (int y = KINECT_CROP_TOP; y < height; ++y)
	{
		for (int x = KINECT_CROP_LEFT; x < width; ++x)
		{
			int i = y*kinectImg.getWidth() + x;
			int j = (y - KINECT_CROP_TOP )* processedImg.getWidth() + (x - KINECT_CROP_LEFT);
			if(inPix[i] < nearThreshold && inPix[i] > farThreshold) {
				outPix[j] = 255;
			} else {
				outPix[j] = 0;
			}
			j++;
		}
	}

	processedImg.transform(INPUT_DATA_R, width/2, height/2, INPUT_DATA_ZOOM, INPUT_DATA_ZOOM, 
		INPUT_DATA_DY, INPUT_DATA_DX);

	processedImg.flagImageChanged();

}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetColor(255, 255, 255);
	kinectImg.draw(0,0);
	processedImg.draw(700, 0);
	// threshImg.draw(0,0);

	// ofPushMatrix();
	// 	ofRotateZ(VIDEO_R);
	// 	videoImg.draw(VIDEO_X, VIDEO_Y, VIDEO_W, VIDEO_H);
	// ofPopMatrix();


	ofPushStyle();
		ofSetColor(0, 255, 0);
		stringstream reportStream;
		reportStream
		<< "x: " << x << endl
		<< "y: " << y << endl
		<< "w: " << w << endl
		<< "h: " << h << endl
		<< "r: " << r << endl
		<< "nearThreshold: " << nearThreshold << endl
		<< "farThreshold: " << farThreshold << endl;
		ofDrawBitmapString(reportStream.str(), 20, 652);
	ofPopStyle();

}

//--------------------------------------------------------------
void ofApp::exit(){

	kinect.close();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	switch (key) {

		case OF_KEY_UP:
			y--;
			break;
		case OF_KEY_DOWN:
			y++;
			break;
		case OF_KEY_LEFT:
			x--;
			break;
		case OF_KEY_RIGHT:
			x++;
			break;
			
		case 'W':
			w *= 1.01;
			break;
			
		case 'w':
			w *= 0.99;
			break;

		case 'H':
			h *= 1.01;
			break;
			
		case 'h':
			h *= 0.99;
			break;

		case 'R':
			r += 0.1;
			break;
			
		case 'r':
			r -= 0.1;
			break;
			
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;
			
		case '<':
		case ',':
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;
			
		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;
			
		case '-':
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
		

	}

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}


