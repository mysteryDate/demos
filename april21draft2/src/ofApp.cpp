#include "ofApp.h"
#define RIVER_NUMBER 4

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0,0,0);

	kinect.init();
	kinect.open();
	kinectImg.allocate(kinect.width, kinect.height);
	processedImg.allocate(kinect.width*INPUT_DATA_ZOOM, kinect.height*INPUT_DATA_ZOOM);

	video.loadMovie("MapBasicV1.mov");
	video.play();

	nearThreshold = 250;
	farThreshold = 165;

	lineSmoothing = 4;
	handScaleUp = 1.1;

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
		//riverRegions[i] = riverRegions[i].getSmoothed(3);
	}

	riverNames[0] = "Riviere des\nOutaouais";
	riverNames[1] = "Riviere\ndu Nord";
	riverNames[2] = "Riviere\nOuest";
	riverNames[3] = "Riviere\nCalumet";
	riverNames[4] = "Riviere\nRouge";

	myfont.loadFont("FrutigerLTStd-Roman.ttf", 12);

}

//--------------------------------------------------------------
void ofApp::update(){


	kinect.update();
	video.update();

	videoImg.setFromPixels(video.getPixels(), video.width, video.height, OF_IMAGE_COLOR);


	if(kinect.isFrameNew()) {

		kinectImg.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		transformInput();

		contourFinder.findContours(processedImg);
		contourFinder.update();

	}

}

void ofApp::transformInput()
{
	// Crop the image and threshold
	processedImg.set(0);

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

	processedImg.transform(INPUT_DATA_R, 0, 0, INPUT_DATA_ZOOM, INPUT_DATA_ZOOM, 
		INPUT_DATA_DX, INPUT_DATA_DY);

	processedImg.flagImageChanged();

}

//--------------------------------------------------------------
void ofApp::draw(){

	ofPushMatrix();
		ofRotateZ(VIDEO_R);
		videoImg.draw(VIDEO_X, VIDEO_Y, VIDEO_W, VIDEO_H);
	ofPopMatrix();

	drawHandOverlay();


	ofPushStyle();
		ofSetColor(0, 255, 0);
		stringstream reportStream;
		reportStream
		// << "x: " << x << endl
		// << "y: " << y << endl
		// << "w: " << w << endl
		// << "h: " << h << endl
		// << "r: " << r << endl
		// << "nearThreshold: " << nearThreshold << endl
		// << "farThreshold: " << farThreshold << endl
		<< "MAX_HAND_SIZE: " << contourFinder.MAX_HAND_SIZE << endl
		<< "MIN_HAND_SIZE: " << contourFinder.MIN_HAND_SIZE << endl
		<< "MAX_WRIST_WIDTH: " << contourFinder.MAX_WRIST_WIDTH 
		<< endl;
		if(contourFinder.size() > 0) {
			ofRectangle rect = ofxCv::toOf(contourFinder.getBoundingRect(0));
			reportStream
			<< "left: " << rect.getLeft() << endl
			<< "right: " << rect.getRight() << endl
			<< "top: " << rect.getTop() << endl
			<< "bottom: " << rect.getBottom() << endl;
		}
		ofDrawBitmapString(reportStream.str(), 20, 652);

		// ofSetColor(0,255,0);
		// riverRegions[RIVER_NUMBER].draw();
	ofPopStyle();

}

//--------------------------------------------------------------
void ofApp::drawHandOverlay(){

	// contourFinder.draw();

	for (int i = 0; i < contourFinder.size(); ++i)
	{
		ofRectangle bounds = ofxCv::toOf(contourFinder.getBoundingRect(i));

		if(contourFinder.handFound[i]) {

			ofPolyline hand = contourFinder.hands[i].getSmoothed(lineSmoothing);
			ofPoint center = contourFinder.oldCentroids[i];
			ofPoint tip = contourFinder.oldTips[i];

			ofPushMatrix();
			ofPushStyle();
				ofSetColor(255,255,255);

				ofCircle(contourFinder.ends[i][0], 3);
				ofCircle(contourFinder.ends[i][1], 3);
				ofCircle(contourFinder.tips[i], 3);
				ofNoFill();
				ofCircle(contourFinder.tips[i], contourFinder.MAX_HAND_SIZE);
				ofCircle(contourFinder.tips[i], contourFinder.MIN_HAND_SIZE);
				ofCircle(contourFinder.wrists[i][0], contourFinder.MAX_WRIST_WIDTH);
				ofFill();
				ofCircle(contourFinder.wrists[i][0], 3);
				ofCircle(contourFinder.wrists[i][1], 3);

				ofTranslate(center.x*(1-handScaleUp), center.y * (1 - handScaleUp ));
				ofScale(handScaleUp, handScaleUp);

				ofBeginShape();
					for (int i = 0; i < hand.size(); ++i)
					{
						ofVertex(hand[i].x, hand[i].y);
					}
				ofEndShape();

			ofPopStyle();
			ofPopMatrix();

			ofPushMatrix();
			ofPushStyle();
				ofSetColor(0,0,0);

				string palmText;
				for (int i = 0; i < riverRegions.size(); ++i)
				{
					if( riverRegions[i].inside(center) ) {
						palmText = riverNames[i];
					}
				}
				palmText = ofToString(palmText);
				ofTranslate(center.x, center.y);

				float hypotenuse = sqrt( pow(center.x - tip.x, 2) + pow(center.y - tip.y, 2) );
				float angle =  ofRadToDeg( asin( (tip.y - center.y) / hypotenuse ));
				if(tip.x < center.x) angle *= -1;
				ofPoint exit = contourFinder.ends[i][0];
				if (exit.y <= contourFinder.bounds[1] + 5) angle += 180;
				if ( (exit.x <= contourFinder.bounds[0] + 5 or exit.x >= contourFinder.bounds[2] - 5 ) and tip.y < center.y ) 
					angle += 180;
				ofRotateZ(angle);

				myfont.drawString(palmText, -10, -10);

			ofPopStyle();
			ofPopMatrix();

		}
	}

}

//--------------------------------------------------------------
void ofApp::exit(){

	kinect.close();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	switch (key) {

		// case OF_KEY_UP:
		// 	y--;
		// 	break;
		// case OF_KEY_DOWN:
		// 	y++;
		// 	break;
		// case OF_KEY_LEFT:
		// 	x--;
		// 	break;
		// case OF_KEY_RIGHT:
		// 	x++;
		// 	break;
			
		// case 'W':
		// 	w *= 1.01;
		// 	break;
			
		// case 'w':
		// 	w *= 0.99;
		// 	break;

		// case 'H':
		// 	h *= 1.01;
		// 	break;
			
		// case 'h':
		// 	h *= 0.99;
		// 	break;

		// case 'R':
		// 	r += 0.1;
		// 	break;
			
		// case 'r':
		// 	r -= 0.1;
		// 	break;
			
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

		// case ' ':{
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
		// 	ofBufferToFile("rouge.txt", buff);
		// 	break;
		// }

		// case 'W':
		// 	riverRegions[RIVER_NUMBER].clear();
			// ofBuffer buff2 = ofBufferFromFile("verteces.txt");
			// while(!buff2.isLastLine() ) {
			// 	int x = std::atoi( buff2.getNextLine().c_str() );
			// 	int y = std::atoi( buff2.getNextLine().c_str() );
			// 	riverRegions[2].addVertex(x, y);
			// }
			// riverRegions[2].close();

	}

}

//--------------------------------------------------------------
// void ofApp::mousePressed(int x, int y, int button){

// 	// riverRegions[RIVER_NUMBER].addVertex(x, y);

// }


