#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	kinect.init();
	kinect.open();

	inputImg.allocate(kinect.width, kinect.height);
	threshImg.allocate(kinect.width, kinect.height);

	nearThreshold = 250;
	farThreshold = 165;

	ofSetFrameRate(60);

}

//--------------------------------------------------------------
void ofApp::update(){

	kinect.update();

	if(kinect.isFrameNew()) {

		inputImg.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		threshImg.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);

		// Threshold the image
		unsigned char * pix = threshImg.getPixels();
		int numPixels = threshImg.getWidth() * threshImg.getHeight();
		for (int i = 0; i < numPixels; ++i)
		{
			if(pix[i] < nearThreshold && pix[i] > farThreshold) {
				pix[i] = 255;
			} else {
				pix[i] = 0;
			}
		}
	}

	threshImg.flagImageChanged();

	contourFinder.findContours(threshImg);
	contourFinder.update();

	for (int i = 0; i < contourFinder.size(); ++i)
	{
		contourFinder.findHand(i);
	}

}

//--------------------------------------------------------------
void ofApp::draw(){

	//inputImg.draw(0, 0, kinect.width, kinect.height);
	threshImg.draw(0, 0, kinect.width, kinect.height);
	contourFinder.draw();

	ofSetColor(255, 0, 255);
	for (int i = 0; i < contourFinder.size(); ++i)
	{
		if(contourFinder.handFound[i]) {
			ofCircle(contourFinder.ends[i][0], 3);
			ofCircle(contourFinder.ends[i][1], 3);
			ofCircle(contourFinder.tips[i], 3);
			ofCircle(contourFinder.wrists[i][0], 3);
			ofCircle(contourFinder.wrists[i][1], 3);
			ofNoFill();
			ofCircle(contourFinder.tips[i], contourFinder.MAX_HAND_SIZE);
			ofCircle(contourFinder.tips[i], contourFinder.MIN_HAND_SIZE);
		}
	}

	ofSetColor(255, 255, 255);
	stringstream reportStream;
	reportStream 
	<< "MAX_HAND_SIZE: " << contourFinder.MAX_HAND_SIZE << endl
	<< "MIN_HAND_SIZE: " << contourFinder.MIN_HAND_SIZE
	// << "Near threshold: " << nearThreshold << endl
	// << "Far threshold: " << farThreshold 
	<< endl;

	ofDrawBitmapString(reportStream.str(), 20, 652);

}

//--------------------------------------------------------------
void ofApp::exit(){

	kinect.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	switch (key) {
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

		case 'H':
			contourFinder.MAX_HAND_SIZE++;
			break;

		case 'h':
			contourFinder.MAX_HAND_SIZE--;
			break;

		case 'G':
			contourFinder.MIN_HAND_SIZE++;
			break;

		case 'g':
			contourFinder.MIN_HAND_SIZE--;
			break;
	}

}
