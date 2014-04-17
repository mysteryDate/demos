#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	kinect.init();
	kinect.open();

	inputImg.allocate(kinect.width, kinect.height);
	threshImg.allocate(kinect.width, kinect.height);

	background.loadImage("rivers.jpg");
	background.setImageType(OF_IMAGE_COLOR);

	nearThreshold = 250;
	farThreshold = 165;

	ofSetFrameRate(60);

	// Good for kinect in present orientation
	DX = -35;
	DY = -84;
	ZOOM = 2.57;
	DEGREES = 0;

	handSmoother = 4;

	scaleup = 1.1;

	dispMode = 0;
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

	ofPushMatrix();
	ofTranslate(137, 85);
	ofScale(0.426, 0.426);
	ofRotateZ(-10);
	background.draw(0, 0);
	ofPopMatrix();

	ofPushStyle();
	drawHandOverlay();

	ofSetColor(0, 0, 0);
	stringstream reportStream;
	reportStream 
	<< "nearThreshold: " << nearThreshold << ", farThreshold: " << farThreshold << endl
	<< "DX: " << DX << ", DY: " << DY << endl
	<< "ZOOM: " << ZOOM << ", DEGREES: " << DEGREES << endl
	<< "scaleup: " << scaleup << endl
	<< "handSmoother: " << handSmoother 
	// << "MAX_HAND_SIZE: " << contourFinder.MAX_HAND_SIZE << endl
	// << "MIN_HAND_SIZE: " << contourFinder.MIN_HAND_SIZE
	// << "Near threshold: " << nearThreshold << endl
	// << "Far threshold: " << farThreshold 
	<< endl;
	ofDrawBitmapString(reportStream.str(), 20, 652);
	ofPopStyle();

}

void ofApp::drawHandOverlay() {

	ofPushMatrix();
	ofTranslate(DX, DY);
	ofScale(ZOOM, ZOOM);
	ofRotateZ(DEGREES);

	if( dispMode == 1 ) {
		inputImg.draw(0,0);
		return;
	}

	if( dispMode == 2 ) {
		threshImg.draw(0,0);
		return;
	}

	ofSetColor(0, 255, 0);
	//contourFinder.draw();
	drawLabels();

	for (int i = 0; i < contourFinder.size(); ++i)
	{
		if(contourFinder.handFound[i]) {
			ofPushStyle();
			ofPushMatrix();

			//float scaleup = 1.5;
			ofPolyline smoothHand = contourFinder.hands[i].getSmoothed(handSmoother);
			ofPoint centroid = smoothHand.getCentroid2D();
			ofTranslate(centroid.x*(1-scaleup), centroid.y*(1-scaleup));
			ofScale(scaleup, scaleup);

			smoothHand.draw();
			ofCircle(centroid, 3);
			ofSetColor(255,0,0);
			ofCircle(contourFinder.tips[i], 2);
			ofCircle(contourFinder.wrists[i][0], 2);
			ofCircle(contourFinder.wrists[i][1], 2);

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

		case 'm':
			dispMode++;
			if( dispMode > 2 )
				dispMode = 0;
			break;
		
	}

}
