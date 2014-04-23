#include "ofApp.h"
#define RIVER_NUMBER 4

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0,0,0);
	ofSetFrameRate(60);

	//kinect instructions
	kinect.init();
	kinect.open();
	kinectImg.allocate(kinect.width, kinect.height);
	// kinectImg.setROI(KINECT_CROP_LEFT, KINECT_CROP_RIGHT, 
	// 	kinect.width - KINECT_CROP_LEFT - KINECT_CROP_RIGHT,
	// 	kinect.height - KINECT_CROP_TOP - KINECT_CROP_BOTTOM);
	// processedImg.allocate(kinect.width*INPUT_DATA_ZOOM, kinect.height*INPUT_DATA_ZOOM);

	nearThreshold = 250;
	farThreshold = 170;

	//video instructions
	video.loadMovie("Map_Argenteuil_v5.mov");
	video.play();

	// For hand display
	fillInRiverRegions();
	lineSmoothing = 4;
	armScaleUp = 1.1;
	myfont.loadFont("AltoPro-Normal.ttf", 12);

	//for water ripples
	ofEnableAlphaBlending();
	ripples.allocate(1920, 1080);
	bounce.allocate(1920, 1080);
	riverMask.loadImage("riviere_masque_alpha.png");

}

// Read in the proper regions for river display
void ofApp::fillInRiverRegions() {

	ofBuffer buffer;
	riverRegions.resize(5);
	for (int i = 0; i < riverRegions.size(); ++i)
	{
		string filename;
		switch (i) {
			case 0:
				filename = "outaouais.txt";
				break;
			case 1:
				filename = "nord.txt";
				break;
			case 2:
				filename = "ouest.txt";
				break;
			case 3:
				filename = "calumet.txt";
				break;
			case 4:
				filename = "rouge.txt";
				break;
			
		}
		buffer = ofBufferFromFile(filename);
		while(!buffer.isLastLine() ) {
			int x = std::atoi( buffer.getNextLine().c_str() );
			int y = std::atoi( buffer.getNextLine().c_str() );
			riverRegions[i].addVertex(x, y);
		}
		riverRegions[i].close();
	}

	riverNames[0] = "Riviere des\nOutaouais";
	riverNames[1] = "Riviere\ndu Nord";
	riverNames[2] = "Riviere\nOuest";
	riverNames[3] = "Riviere\nCalumet";
	riverNames[4] = "Riviere\nRouge";

}

//--------------------------------------------------------------
void ofApp::update(){

	kinect.update();
	video.update();

	bounce.setTexture(video.getTextureReference(), 1);

	if(kinect.isFrameNew()) {

		kinectImg.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		transformInput();

		input = ofxCv::toCv(kinectImg);
		cv::Rect crop_roi = cv::Rect(KINECT_CROP_LEFT, KINECT_CROP_TOP, 
			kinect.width - KINECT_CROP_LEFT - KINECT_CROP_RIGHT,
			kinect.height - KINECT_CROP_TOP - KINECT_CROP_BOTTOM);
		croppedInput = input(crop_roi).clone();

		contourFinder.findContours(croppedInput);
		contourFinder.update();

		updateHands();

	}

	updateRipples();

}

// Crop the image and threshold
void ofApp::transformInput()
{
	unsigned char *pix = kinectImg.getPixels();

	int numPix = kinectImg.getWidth() * kinectImg.getHeight();

	for (int i = 0; i < numPix; ++i)
	{
		if(pix[i] > nearThreshold or pix[i] < farThreshold) {
			pix[i] = 0;
		} 
	}

	kinectImg.flagImageChanged();

}

// Update the rippling effect
void ofApp::updateRipples(){

	// Water ripples
	ripples.begin();
		ofPushStyle();
		ofPushMatrix();
			ofFill();
			ofSetColor(255,255,255);

			ofEllipse(mouseX, mouseY, 10, 10);
			for (int i = 0; i < contourFinder.size(); ++i)
			{
				ofBeginShape();
				for (int j = 0; j < contourFinder.getPolyline(i).size(); ++j)
				{
					ofVertex(contourFinder.getPolyline(i)[j]);
				}
				ofEndShape();
			}
			riverMask.draw(0,0);
		ofPopStyle();
		ofPopMatrix();
	ripples.end();
	ripples.update();
	bounce << ripples;

}

// For hand text display
void ofApp::updateHands(){

	vector < Hand > newHands;

	// Newhand -> oldhand association
	map < int, int > association;

	for (int i = 0; i < contourFinder.size(); ++i)
	{
		if(contourFinder.handFound[i]) {
			Hand blob;
			blob.line = contourFinder.getHand(i);
			blob.centroid = blob.line.getCentroid2D();
			blob.tip = contourFinder.tips[i];
			blob.wrists = contourFinder.wrists[i];
			blob.ends = contourFinder.ends[i];
			blob.label = contourFinder.getLabel(i);
			newHands.push_back(blob);
		}
	}

	// Remove dead ones
	for (int i = 0; i < hands.size(); ++i)
	{
		bool found = false;
		unsigned int label = hands[i].label;
		for (int j = 0; j < newHands.size(); ++j)
		{
			if(newHands[j].label == label) {
				association[j] = i;
				found = true;
				break;
			}
		}
		if(!found) {
			hands.erase( hands.begin() + i );
			i--; //So that it doesn't skip the next one
		}
	}

	//Add new ones
	for (int i = 0; i < newHands.size(); ++i)
	{
		bool found = false;
		unsigned int label = newHands[i].label;
		for (int j = 0; j < hands.size(); ++j)
		{
			if(hands[j].label == label) {
				association[i] = j;
				found = true;
				break;
			}
		}
		if(!found) {
			hands.push_back(newHands[i]);
			association[i] = hands.size() - 1;
		}
	}

	//Finally, the magic
	int noiseDist = 0;
	for (int i = 0; i < newHands.size(); ++i)
	{
		int j = association[i];
		// ofPoint oldCentroid = hands[j].centroid;
		// ofPoint newCentroid = newHands[i].centroid;
		// ofPoint oldTip 		= hands[j].tip;
		// ofPoint newTip 		= newHands[i].tip;

		// int centDist = ofDistSquared(oldCentroid.x, oldCentroid.y, newCentroid.x, newCentroid.y);
		// int tipDist = ofDistSquared(oldTip.x, oldTip.y, newTip.x, newTip.y);

		// if(centDist > noiseDist or tipDist > noiseDist) {
		// 	hands[j].centroid 	= newCentroid;
		// 	hands[j].tip 		= newTip;
		// }

		hands[j] = newHands[i];

	}
}

//--------------------------------------------------------------
void ofApp::draw(){

	// videoImg.draw(0,0);
	bounce.draw(VIDEO_X, VIDEO_Y, VIDEO_W, VIDEO_H);
	contourFinder.draw();

	drawHandOverlay();

	ofSetColor(0,255,0);
	for (int i = 0; i < riverRegions.size(); ++i)
	{
		riverRegions[i].draw();
	}

	drawFeedback();

}

void ofApp::drawHandOverlay(){

	// Arm (or should I just say blob) masking
	ofPushStyle();
	ofSetColor(0,0,0);
	ofFill();

	ofPushMatrix();
	ofTranslate(INPUT_DATA_DX, INPUT_DATA_DY);
	ofScale(INPUT_DATA_ZOOM, INPUT_DATA_ZOOM);

	for (int i = 0; i < contourFinder.size(); ++i)
	{
		ofPolyline blob = contourFinder.getPolyline(i);
		blob = blob.getSmoothed(lineSmoothing);
		ofPoint center = blob.getCentroid2D();

		ofBeginShape();
			for (int j = 0; j < blob.size(); ++j) {
				ofVertex(blob[j]);
			}
		ofEndShape();
	}

	ofPopMatrix();

	// Drawing text onto hands
	ofSetColor(255,255,255);
	for (int i = 0; i < hands.size(); ++i)
	{
		string palmText;
		ofPoint center 	= hands[i].centroid;
		ofPoint tip 	= hands[i].tip;
		center.x = center.x * INPUT_DATA_ZOOM + INPUT_DATA_DX;
		center.y = center.y * INPUT_DATA_ZOOM + INPUT_DATA_DY;
		tip.x = tip.x * INPUT_DATA_ZOOM + INPUT_DATA_DX;
		tip.y = tip.y * INPUT_DATA_ZOOM + INPUT_DATA_DY;

		ofPushStyle();
			ofSetColor(255,255,255);
			ofCircle(center, 3);
			if(contourFinder.ends[i].size() == 2) {
				ofFill();
				ofCircle(contourFinder.ends[i][0], 3);
				ofCircle(contourFinder.ends[i][1], 3);
			}
			if(true) {
				ofCircle(contourFinder.tips[i], 3);
				ofNoFill();
				ofCircle(contourFinder.tips[i], contourFinder.MAX_HAND_SIZE);
				ofCircle(contourFinder.tips[i], contourFinder.MIN_HAND_SIZE);
			}
			if(contourFinder.wrists[i].size() == 2) {
				ofCircle(contourFinder.wrists[i][0], contourFinder.MAX_WRIST_WIDTH);
				ofFill();
				ofCircle(contourFinder.wrists[i][0], 3);
				ofCircle(contourFinder.wrists[i][1], 3);
			}
		ofPopStyle();

		for (int j = 0; j < riverRegions.size(); ++j)
		{
			if( riverRegions[j].inside(center) ) {
				palmText = riverNames[j];
			}
		}

		ofPushMatrix();
			ofTranslate(center.x, center.y);

			// Proper rotation
			float h = sqrt( pow(center.x - tip.x, 2) + pow(center.y - tip.y, 2) );
			float angle =  ofRadToDeg( asin( (tip.y - center.y) / h ));
			if(tip.x < center.x) angle *= -1;
			// ofPoint exit = contourFinder.ends[i][0];
			// if (exit.y <= contourFinder.bounds[1] + 5) angle += 180;
			// if ( (exit.x <= contourFinder.bounds[0] + 5 or exit.x >= contourFinder.bounds[2] - 5 ) and tip.y < center.y ) 
			// 	angle += 180;
			ofRotateZ(angle);

			ofPoint textCenter = myfont.getStringBoundingBox(palmText, 0, 0).getCenter();
			ofTranslate(-textCenter.x, -textCenter.y);
			myfont.drawString(palmText, 0, 0);
		ofPopMatrix();

	}

	ofPopStyle();
	ofPopMatrix();
}

void ofApp::drawFeedback() {

	ofPushStyle();
	ofSetColor(0,255,0);

	stringstream reportStream;

	if (contourFinder.size() != 0)
	{
		ofRectangle rect = ofxCv::toOf(contourFinder.getBoundingRect(0));
		reportStream
		<< "left: " << rect.getLeft() << endl
		<< "right: " << rect.getRight() << endl
		<< "top: " << rect.getTop() << endl
		<< "bottom: " << rect.getBottom() << endl;
	}

	reportStream 
	<< "nearThreshold: " << nearThreshold << endl
	<< "farThreshold: " << farThreshold << endl
	<< "MAX_HAND_SIZE: " << contourFinder.MAX_HAND_SIZE << endl
	<< "MIN_HAND_SIZE: " << contourFinder.MIN_HAND_SIZE << endl
	<< "MAX_WRIST_WIDTH: " << contourFinder.MAX_WRIST_WIDTH << endl
	<< "hands found: " << hands.size() << endl
	<< "contourFinder.size(): " << contourFinder.size() << endl
	<< ofToString(ofGetFrameRate()) << endl;

	ofDrawBitmapString(reportStream.str(), 20, 652);
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::exit(){

	kinect.close();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	switch(key) {

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

		case 'S':
			contourFinder.MAX_WRIST_WIDTH++;
			break;

		case 's':
			contourFinder.MAX_WRIST_WIDTH--;
			break;

	}

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

