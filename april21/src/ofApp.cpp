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
		if(contourFinder.ends[i].size() == 2) {
			ofCircle(contourFinder.ends[i][0], 10);
			ofCircle(contourFinder.ends[i][1], 10);
			ofCircle(contourFinder.tips[i], 3);
		}
	}

	ofSetColor(255, 255, 255);
	stringstream reportStream;
	reportStream << "Near threshold: " << nearThreshold << endl
	<< "Far threshold: " << farThreshold << endl;

	//ofDrawBitmapString(reportStream.str(), 20, 652);

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
	}

}
