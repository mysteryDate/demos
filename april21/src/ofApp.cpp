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

	DX = -28;
	DY = -8;
	ZOOM = 2.57;
	DEGREES = 0;

	handSmoother = 4;

	scaleup = 1;
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

}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetColor(0,0,0);
	ofRect(0, 0, 1920, 1080);

	drawHandOverlay();

	ofSetColor(255, 255, 255);
	stringstream reportStream;
	reportStream 
	<< "DX: " << DX << ", DY: " << DY << endl
	<< "ZOOM: " << ZOOM << ", DEGREES: " << DEGREES << endl
	<< "handSmoother: " << handSmoother
	<< "scaleup: " << scaleup
	// << "MAX_HAND_SIZE: " << contourFinder.MAX_HAND_SIZE << endl
	// << "MIN_HAND_SIZE: " << contourFinder.MIN_HAND_SIZE
	// << "Near threshold: " << nearThreshold << endl
	// << "Far threshold: " << farThreshold 
	<< endl;
	ofDrawBitmapString(reportStream.str(), 20, 652);

}

void ofApp::drawHandOverlay() {

	ofPushMatrix();
	ofTranslate(DX, DY);
	ofScale(ZOOM, ZOOM);
	ofRotateZ(DEGREES);

	//inputImg.draw(0,0);
	contourFinder.draw();
	drawLabels();

	for (int i = 0; i < contourFinder.size(); ++i)
	{
		if(contourFinder.handFound[i]) {
			ofPushStyle();
			ofPushMatrix();

			//float scaleup = 1.5;
			ofSetColor(0, 255, 3);
			ofPolyline smoothHand = contourFinder.hands[i].getSmoothed(handSmoother);
			ofPoint centroid = smoothHand.getCentroid2D();
			ofTranslate(centroid.x*(1-scaleup), centroid.y*(1-scaleup));
			ofScale(scaleup, scaleup);

			smoothHand.draw();
			ofCircle(centroid, 3);

			ofPopMatrix();
			ofPopStyle();

		}
	}

	ofPopMatrix();
}

void ofApp::drawLabels() {

	ofPushStyle();
	ofSetColor(0,255,0);
	ofxCv::RectTracker& tracker = contourFinder.getTracker();

	for (int i = 0; i < contourFinder.size(); ++i)
	{
		ofPoint center = ofxCv::toOf(contourFinder.getCenter(i));
		ofPushMatrix();
		ofTranslate(center.x, center.y);
		int label = contourFinder.getLabel(i);
		string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label));
		ofDrawBitmapString(msg, 0, 0);
		ofPopMatrix();
	}
	ofPopStyle();

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

		case OF_KEY_UP:
			DY--;
			break;

		case OF_KEY_LEFT:
			DX--;
			break;

		case OF_KEY_DOWN:
			DY++;
			break;

		case OF_KEY_RIGHT:
			DX++;
			break;

		case 'Z':
			ZOOM *= 1.1;
			break;

		case 'z':
			ZOOM *= 0.9;
			break;

		case 'R':
			DEGREES += 1;
			break;

		case 'r':
			DEGREES -= 1;
			break;

		case 'S':
			handSmoother++;
			break;

		case 's':
			handSmoother--;
			break;

		case 'U':
			scaleup *= 1.1;
			break;

		case 'u':
			scaleup *= 0.9;
			break;
		
	}

}
