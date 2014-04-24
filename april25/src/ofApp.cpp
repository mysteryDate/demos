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

	bFeedback = false;

	contourFinder.bounds[0] = 1;
	contourFinder.bounds[1] = 1;
	contourFinder.bounds[2] = kinect.width - KINECT_CROP_LEFT - KINECT_CROP_RIGHT - 1;
	contourFinder.bounds[3] = kinect.height - KINECT_CROP_TOP - KINECT_CROP_BOTTOM - 1;

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

	// Ice starts breaking at frame 552 
	// Breakage reaches the top aat 947 	
	// Ice is gone at 1103
	// Rivers exist at 1190 

	// Water ripples
	ripples.begin();
		ofPushStyle();
		ofPushMatrix();
			ofFill();
			ofSetColor(255,255,255);
			// Put the data reference frame into the untransformed video reference
			ofTranslate(INPUT_DATA_DX - VIDEO_X, INPUT_DATA_DY - VIDEO_Y);
			ofScale(INPUT_DATA_ZOOM * video.getWidth() / VIDEO_W 
				, INPUT_DATA_ZOOM * video.getHeight() / VIDEO_H);

		    float b = INPUT_DATA_ZOOM * video.getHeight() / VIDEO_W;
		    float v = INPUT_DATA_ZOOM * video.getWidth() / VIDEO_H;
		    float w = video.getWidth();
		    float h = video.getHeight();

			for (int i = 0; i < hands.size(); ++i)
			{
				ofBeginShape();
				for (int j = 0; j < hands[i].line.size(); ++j)
				{
					ofVertex(hands[i].line[j]);
				}
				ofEndShape();
			}

		ofPopMatrix();
		if(video.getCurrentFrame() > 1200)
			riverMask.draw(0,0);
		ofPopStyle();
	ripples.end();
	ripples.update();
	bounce << ripples;

}

// For hand text display
void ofApp::updateHands(){

	vector < Hand > newHands;

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
			blob.index = i;
			newHands.push_back(blob);
		}
	}

	sort(newHands.begin(), newHands.end());

	// Remove dead ones
	for (int i = 0; i < hands.size(); ++i)
	{
		bool found = false;
		unsigned int label = hands[i].label;
		for (int j = 0; j < newHands.size(); ++j)
		{
			if(newHands[j].label == label) {
				hands[i].index = newHands[j].index;
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
				hands[j].index = newHands[i].index;
				found = true;
				break;
			}
		}
		if(!found) {
			hands.push_back(newHands[i]);
		}
	}

	sort(hands.begin(), hands.end());

	//Finally, the magic
	int noiseDist = 0;
	for (int i = 0; i < hands.size(); ++i)
	{
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

		hands[i] = newHands[i];

	}
}

//--------------------------------------------------------------
void ofApp::draw(){

	// videoImg.draw(0,0);
	bounce.draw(VIDEO_X, VIDEO_Y, VIDEO_W, VIDEO_H);

	drawHandOverlay();

	if(bFeedback)
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
			ofPoint exit = hands[i].ends[0];
			if (exit.y <= contourFinder.bounds[1] + 5) angle += 180;
			if ( (exit.x <= contourFinder.bounds[0] + 5 or exit.x >= contourFinder.bounds[2] - 5 ) and tip.y < center.y ) 
				angle += 180;
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
	contourFinder.draw();

	ofSetColor(255,255,255);
	for (int i = 0; i < hands.size(); ++i)
	{
		ofCircle(hands[i].centroid, 3);
		ofFill();
		ofCircle(hands[i].ends[0], 3);
		ofCircle(hands[i].ends[1], 3);
		ofCircle(hands[i].tip, 3);
		ofNoFill();
		ofCircle(hands[i].tip, contourFinder.MAX_HAND_SIZE);
		ofCircle(hands[i].tip, contourFinder.MIN_HAND_SIZE);
		ofCircle(hands[i].wrists[0], contourFinder.MAX_WRIST_WIDTH);
		ofFill();
		ofCircle(hands[i].wrists[0], 3);
		ofCircle(hands[i].wrists[1], 3);
	}

	ofSetColor(0,255,0);
	for (int i = 0; i < riverRegions.size(); ++i)
	{
		riverRegions[i].draw();
	}

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
	<< "frame: " << video.getCurrentFrame() << endl
	<< ofToString(ofGetFrameRate()) << endl;

	ofDrawBitmapString(reportStream.str(), 20, 600);
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

		case 'f':
			bFeedback = !bFeedback;
			break;

		case ' ':
			if(video.isPaused())
				video.setPaused(false);
			else
				video.setPaused(true);

	}

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

