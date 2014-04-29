#include "ofApp.h"
#define RIVER_NUMBER 2
#define FILE_NAME "ouest.txt"

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0,0,0);
	ofSetFrameRate(60);

	//kinect instructions
	kinect.init();
	kinect.open();
	kinectImg.allocate(kinect.width, kinect.height);
	kinectBackground.allocate(kinect.width, kinect.height);
	kinectBackground.set(0);

	nearThreshold = 192;
	farThreshold = 2;

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
	riverMask.loadImage("river_mask_processed.png");

	bFeedback = true;

	MIN_CONTOUR_AREA = 1000;
	MAX_CONTOUR_AREA = 16000;
	CONTOUR_THRESHOLD = 1;
	contourFinder.setMinArea(MIN_CONTOUR_AREA);
	// contourFinder.setMaxArea(MAX_CONTOUR_AREA);

	contourFinder.bounds[0] = 1;
	contourFinder.bounds[1] = 1;
	contourFinder.bounds[2] = kinect.width - KINECT_CROP_LEFT - KINECT_CROP_RIGHT - 1;
	contourFinder.bounds[3] = kinect.height - KINECT_CROP_TOP - KINECT_CROP_BOTTOM - 1;

	noiseDist = 0;
	smoothingRate = 0.5;

	x = VIDEO_X;
	y = VIDEO_Y;
	w = VIDEO_W;
	h = VIDEO_H;
	r = VIDEO_R;

	video.setFrame(1300);
	video.setPaused(true);


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

	riverNames[0] = "Rivière des\nOutaouais";
	riverNames[1] = "Rivière\ndu Nord";
	riverNames[2] = "Rivière\nOuest";
	riverNames[3] = "Rivière\nCalumet";
	riverNames[4] = "Rivière\nRouge";

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
	unsigned char *bpix = kinectBackground.getPixels();

	int numPix = kinectImg.getWidth() * kinectImg.getHeight();

	for (int i = 0; i < numPix; ++i)
	{
		unsigned char diff = abs(pix[i] - bpix[i]);
		if(diff >= farThreshold && diff <= nearThreshold)
			continue;
		else
			pix[i] = 0;
	}

	kinectImg.flagImageChanged();

}

// Update the rippling effect
void ofApp::updateRipples(){

	// Ice starts breaking at frame 552 
	// Breakage reaches the top aat 947 	
	// Ice is gone at 1103
	// Rivers exist at 1190 
	int frame = video.getCurrentFrame();
	int ICE_START 		= 552;
	int ALL_BROKEN 		= 947;
	int ICE_STOP 		= 1103;
	int RIVERS_START 	= 1150;

	// Water ripples
	ripples.begin();
		ofPushStyle();
		ofPushMatrix();
			ofFill();
			// Put the data reference frame into the untransformed video reference
			ofTranslate(INPUT_DATA_DX - VIDEO_X, INPUT_DATA_DY - VIDEO_Y);
			ofScale(INPUT_DATA_ZOOM * video.getWidth() / VIDEO_W 
				, INPUT_DATA_ZOOM * video.getHeight() / VIDEO_H);
			ofRotateZ(-VIDEO_R);

			for (int i = 0; i < hands.size(); ++i)
			{
				float color = ofMap(hands[i].velocity.length(), 1, 8, 0, 255, true);
				ofSetColor(color, color, color);
				ofBeginShape();
				for (int j = 0; j < hands[i].line.size(); ++j)
				{
					ofVertex(hands[i].line[j]);
				}
				ofEndShape();
			}

		ofPopMatrix();

		ofSetColor(0,0,0);
		ofFill();
		if(frame < ICE_START)
			ofRect(0,0,video.getWidth(),video.getHeight());
		if(frame >= ICE_START and frame <= ICE_START + 120) {
			ripples.damping = ofMap(frame, ICE_START, ICE_START + 120, 0, 0.995);
		}
		if(frame > ALL_BROKEN and frame <= RIVERS_START) {
			ripples.damping = ofMap(frame, ALL_BROKEN, RIVERS_START, 0.995, 0);
		}
		if(frame > RIVERS_START) {
			ripples.damping = 0.995;
			riverMask.draw(0,0);
		}
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
			blob.label = contourFinder.getLabel(i);
			blob.line = contourFinder.getHand(i);
			blob.centroid = blob.line.getCentroid2D();
			blob.tip = contourFinder.tips[blob.label];
			blob.wrists = contourFinder.wrists[blob.label];
			blob.end = contourFinder.ends[blob.label];
			blob.boxCenter = ofxCv::toOf(contourFinder.getCenter(i));
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
	int ignoreDist = noiseDist * noiseDist;
	for (int i = 0; i < hands.size(); ++i)
	{
		Hand handCopy = newHands[i];
		// Doesn't copy vectors, do it by 'hand' for now
		handCopy.wrists = newHands[i].wrists;
		handCopy.velocity = newHands[i].velocity;

		ofPoint oldKeypoints[] = {hands[i].centroid, hands[i].end, hands[i].tip, hands[i].wrists[0], hands[i].wrists[1]};
		ofPoint * keypoints[] = {&handCopy.centroid, &handCopy.end, &handCopy.tip, &handCopy.wrists[0], &handCopy.wrists[1]};

		if( !(newHands[i].centroid.x == 0 and newHands[i].centroid.y == 0) ) 
		{
			for (int j = 0; j < 5; ++j)
			{
				float smoothedX = ofLerp(keypoints[j]->x, oldKeypoints[j].x, smoothingRate);
				float smoothedY = ofLerp(keypoints[j]->y, oldKeypoints[j].y, smoothingRate);
				*keypoints[j] = ofPoint(smoothedX, smoothedY);
			}
			handCopy.velocity = ofVec2f((keypoints[0]->x - oldKeypoints[0].x)/2, (keypoints[0]->y - oldKeypoints[0].y)/2);
		}

		ofPoint oldCentroid = hands[i].centroid;
		ofPoint newCentroid = handCopy.centroid;
		ofPoint oldTip 		= hands[i].tip;
		ofPoint newTip 		= handCopy.tip;

		hands[i] = handCopy;

		int centDist = ofDistSquared(oldCentroid.x, oldCentroid.y, newCentroid.x, newCentroid.y);
		int tipDist = ofDistSquared(oldTip.x, oldTip.y, newTip.x, newTip.y);

		if(centDist < ignoreDist) 
			hands[i].centroid 	= oldCentroid;
		if(tipDist < ignoreDist)
			hands[i].tip 		= oldTip;

	}
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofRotateZ(VIDEO_R);
		bounce.draw(VIDEO_X, VIDEO_Y, VIDEO_W, VIDEO_H);
	ofRotateZ(-VIDEO_R);

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

    if(video.getCurrentFrame() > 1100) {
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
			// Figure out which side the person is standing on
			float m = (hands[i].boxCenter.y - hands[i].centroid.y) / (hands[i].boxCenter.x - hands[i].centroid.x);
			int side = -1;
			int direction = 1;
			if(hands[i].centroid.x > hands[i].boxCenter.x)
				direction *= -1;
			if(hands[i].centroid.x == hands[i].boxCenter.x) {
				if(hands[i].centroid.y > hands[i].boxCenter.y )
					side = 3;
				else
					side = 1;
			}
			ofPoint pos = hands[i].boxCenter;
			while(side == -1) {
				pos.x += direction;
				pos.y += direction * m;
				if(pos.x >= contourFinder.bounds[2]) 
					side = 2;
				if(pos.x <= contourFinder.bounds[0])
					side = 0;
				if(pos.y <= contourFinder.bounds[1])
					side = 1;
				if(pos.y >= contourFinder.bounds[3])
					side = 3;
			}

			if (side == 1) angle += 180;
			if ( (side == 0 or side == 2 ) and tip.y < center.y ) 
				angle += 180;
			ofRotateZ(angle);

			ofPoint textCenter = myfont.getStringBoundingBox(palmText, 0, 0).getCenter();
			float width = myfont.getStringBoundingBox(palmText, 0, 0).getWidth();
			ofTranslate(-textCenter.x, -textCenter.y);
			float size = ofDist(tip.x, tip.y, center.x, center.y);
			ofScale(size/width*0.75, size/width*0.75);
			myfont.drawString(palmText, 0, 0);
		ofPopMatrix();

	}
    }

	ofPopStyle();
	ofPopMatrix();
}

void ofApp::drawFeedback() {

	ofPushStyle();
	contourFinder.draw();
	// kinectImg.draw(0,0);
	ofSetColor(0,255,0);
	for (int i = 0; i < contourFinder.size(); ++i)
	{
		unsigned int l = contourFinder.getLabel(i);
		ofDrawBitmapString(ofToString(contourFinder.side[l]),100,100);

		ofPushStyle();
			ofSetColor(255,255,0);
			ofCircle(contourFinder.tips[l], 3);
		ofPopStyle();
		
		ofPolyline rotatedRect = ofxCv::toOf(contourFinder.getMinAreaRect(i));
		// ofCircle(contourFinder.ends[i], 3);
		rotatedRect.draw();

	}

	ofSetColor(255,255,255);
	for (int i = 0; i < hands.size(); ++i)
	{
		// ofDrawBitmapString(ofToString(hands[i].velocity.length()),100,130);
		ofCircle(hands[i].centroid, 3);
		ofFill();
		ofCircle(hands[i].end, 3);
		ofCircle(hands[i].tip, 3);
		ofNoFill();
		ofCircle(hands[i].tip, contourFinder.MAX_HAND_SIZE);
		ofCircle(hands[i].tip, contourFinder.MIN_HAND_SIZE);
		ofCircle(hands[i].wrists[0], contourFinder.MAX_WRIST_WIDTH);
		ofCircle(hands[i].wrists[0], contourFinder.MIN_WRIST_WIDTH);
		ofFill();
		ofCircle(hands[i].wrists[0], 3);
		ofCircle(hands[i].wrists[1], 3);

		int d1 = ofDistSquared(hands[i].wrists[0].x, hands[i].wrists[0].y, hands[i].tip.x, hands[i].tip.y);
		ofDrawBitmapString(ofToString(d1),100,130);
	}

	ofSetColor(0,255,0);
	for (int i = 0; i < riverRegions.size(); ++i)
	{
		riverRegions[i].draw();
	}

	stringstream reportStream;

	// if (contourFinder.size() != 0)
	// {
	// 	ofRectangle rect = ofxCv::toOf(contourFinder.getBoundingRect(0));
	// 	reportStream
	// 	<< "left: " << rect.getLeft() << endl
	// 	<< "right: " << rect.getRight() << endl
	// 	<< "top: " << rect.getTop() << endl
	// 	<< "bottom: " << rect.getBottom() << endl;
	// }

	reportStream 
	// << "x: " << x << ", y: " << y << endl
	// << "w: " << w << ", h: " << h << endl
	// << "r: " << r << endl
	<< "nearThreshold: " << nearThreshold << endl
	<< "farThreshold: " << farThreshold << endl
	<< "MAX_HAND_SIZE: " << contourFinder.MAX_HAND_SIZE << endl
	<< "MIN_HAND_SIZE: " << contourFinder.MIN_HAND_SIZE << endl
	<< "MAX_WRIST_WIDTH: " << contourFinder.MAX_WRIST_WIDTH << endl
	// << "hands found: " << hands.size() << endl
	// << "contourFinder.size(): " << contourFinder.size() << endl
	// << "noiseDist: " << noiseDist << endl
	// << "MAX_CONTOUR_AREA: " << MAX_CONTOUR_AREA << endl
	// << "MIN_CONTOUR_AREA: " << MIN_CONTOUR_AREA << endl
	// << "CONTOUR_THRESHOLD: " << CONTOUR_THRESHOLD << endl
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

		// case 'H':
		// 	contourFinder.MAX_HAND_SIZE++;
		// 	break;

		// case 'h':
		// 	contourFinder.MAX_HAND_SIZE--;
		// 	break;

		// case 'G':
		// 	contourFinder.MIN_HAND_SIZE++;
		// 	break;

		// case 'g':
		// 	contourFinder.MIN_HAND_SIZE--;
		// 	break;

		// case 'S':
		// 	contourFinder.MAX_WRIST_WIDTH++;
		// 	break;

		// case 's':
		// 	contourFinder.MAX_WRIST_WIDTH--;
			// break;

		case 'f':
			bFeedback = !bFeedback;
			break;

		case ' ': 
			if(video.isPaused())
				video.setPaused(false);
			else
				video.setPaused(true);
			break;

		// case 'D':
		// 	noiseDist++;
		// 	break;

		// case 'd':
		// 	noiseDist--;
		// 	break;

		// case OF_KEY_LEFT:
		// 	x--;
		// 	break;

		// case OF_KEY_RIGHT:
		// 	x++;
		// 	break;

		// case OF_KEY_UP:
		// 	y--;
		// 	break;

		// case OF_KEY_DOWN:
		// 	y++;
		// 	break;

		// case 'w':
		// 	w--;
		// 	break;

		// case 'W':
		// 	w++;
		// 	break;

		// case 'h':
		// 	h--;
		// 	break;

		// case 'H':
		// 	h++;
		// 	break;

		// case 'r':
		// 	r-=0.1;
		// 	break;

		// case 'R':
		// 	r+=0.1;
		// 	break;

		case 'A':
			MAX_CONTOUR_AREA *= 1.1;
			contourFinder.setMaxArea(MAX_CONTOUR_AREA);
			break;

		case 'a':
			MAX_CONTOUR_AREA *= 0.9;
			contourFinder.setMaxArea(MAX_CONTOUR_AREA);
			break;

		case 'Q':
			MIN_CONTOUR_AREA *= 1.1;
			contourFinder.setMinArea(MIN_CONTOUR_AREA);
			break;

		case 'q':
			MIN_CONTOUR_AREA *= 0.9;
			contourFinder.setMinArea(MIN_CONTOUR_AREA);
			break;

		case 'T':
			CONTOUR_THRESHOLD *= 1.1;
			contourFinder.setThreshold(CONTOUR_THRESHOLD);
			break;

		case 't':
			CONTOUR_THRESHOLD *= 0.9;
			contourFinder.setThreshold(CONTOUR_THRESHOLD);
			break;

		
		case 'B':
			kinectBackground.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
			break;

		case 'b':
			kinectBackground.set(0);
			break;
		

		// case 'W': {
		// 	string riverVerteces;
		// 	for (int i = 0; i < riverRegions[RIVER_NUMBER].size(); ++i)
		// 	{
		// 		riverVerteces.append(ofToString(riverRegions[RIVER_NUMBER][i].x));
		// 		riverVerteces.append("\n");
		// 		riverVerteces.append(ofToString(riverRegions[RIVER_NUMBER][i].y));
		// 		riverVerteces.append("\n");
		// 	}
		// 	ofBuffer buff;
		// 	buff.set(riverVerteces.c_str(), riverVerteces.size());
		// 	ofBufferToFile(FILE_NAME, buff);
		// 	break;
		// }
	}

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

	// riverRegions[RIVER_NUMBER].addVertex(x, y);
	return;

}

