#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){

	#ifdef _USE_LIVE_VIDEO
		kinect.init();
		kinect.open();
		inputImg.allocate(kinect.width, kinect.height);
		threshImg.allocate(kinect.width, kinect.height);
	#else
		kinect.loadMovie("onehand.mov");
		kinect.play();
		colorImg.allocate(640, 480);
		inputImg.allocate(640, 480);
		threshImg.allocate(640, 480);
	#endif


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

	myfont.loadFont("helveticaneue.ttf", 5);
}

//--------------------------------------------------------------
void ofApp::update(){

	kinect.update();

	if(kinect.isFrameNew()) {

		#ifdef _USE_LIVE_VIDEO
		inputImg.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		threshImg.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		#else
		colorImg.setFromPixels(kinect.getPixels(), 640, 480);
		inputImg = colorImg;
		threshImg = colorImg;
		#endif

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
	//drawLabels();

	for (int i = 0; i < contourFinder.size(); ++i)
	{
		if(contourFinder.handFound[i]) {
			ofPushStyle();
			ofPushMatrix();

			// ofSetColor(255,0,0);
			// ofCircle(contourFinder.ends[i][0], 1);
			// ofCircle(contourFinder.ends[i][1], 1);
			// ofSetColor(0,255,255);
			// ofCircle(contourFinder.tips[i], 1);
			// ofCircle(contourFinder.wrists[i][0], 1);
			// ofCircle(contourFinder.wrists[i][1], 1);

			//float scaleup = 1.5;
			ofPolyline smoothHand = contourFinder.hands[i].getSmoothed(handSmoother);
			ofPoint centroid = smoothHand.getCentroid2D();
			ofTranslate(centroid.x*(1-scaleup), centroid.y*(1-scaleup));
			ofScale(scaleup, scaleup);

			ofPushStyle();
			ofSetColor(0,0,0);
			ofFill();
			ofBeginShape();
			for (int i = 0; i < smoothHand.size(); ++i)
			{
				ofVertex(smoothHand[i].x, smoothHand[i].y);
			}
			ofEndShape();
			ofPopStyle();

			smoothHand.draw();
			// ofCircle(centroid, 3);
			
			ofPopMatrix();
			ofPopStyle();

			ofPushMatrix();
			ofPushStyle();

			ofTranslate(centroid.x, centroid.y);

			// float hypotenuse = sqrt( pow(centroid.x - contourFinder.tips[i].x, 2) + pow(centroid.y - contourFinder.tips[i].y, 2) );
			// float angle =  ofRadToDeg( asin( (contourFinder.tips[i].y - centroid.y) / hypotenuse ));
			// if(contourFinder.tips[i].x < centroid.x ) angle *= -1;
			// ofRotateZ(angle);
			ofPoint exit = contourFinder.ends[i][0];
			if (exit.x <= 5) ofRotateZ(90);
			else if (exit.y <= 5) ofRotateZ(180);
			else if (exit.x >= 625) ofRotateZ(-90);

			ofScale(0.5, 0.5);
			ofSetColor(255,255,255);
			ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL);
			ofDrawBitmapString("Riviere\ndes Outaouais", -30, 0);

			// myfont.drawString("Rivière des Outaouais", -20, 0);

			ofPopMatrix();
			ofPopStyle();

			// vector< ofPoint > orientationEnds = findMostDistantPoints(smoothHand);
			// ofLine(orientationEnds[0], orientationEnds[1]);


		}
	}

	ofPopMatrix();
}

vector< ofPoint > ofApp::findMostDistantPoints(ofPolyline line)
{

	float maxDsquared = 0;
	vector< ofPoint > mostDistant;
	mostDistant.resize(2);
	for (int i = 0; i < line.size(); ++i)
	{
		for (int j = 0; j < line.size(); ++j)
		{
			float d = ofDistSquared(line[i].x, line[i].y, line[j].x, line[j].y);
			if( d > maxDsquared ) {
				maxDsquared = d;
				mostDistant[0] = line[i];
				mostDistant[1] = line[j];
			}
		}
	}

	return mostDistant;

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
