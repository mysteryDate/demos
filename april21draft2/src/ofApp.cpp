#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	kinect.init();
	kinect.open();
	rawKinectInput.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);

	video.loadMovie("MapBasicV1.mov");
	video.play();

}

//--------------------------------------------------------------
void ofApp::update(){

	kinect.update();
	video.update();

	rawVideoInput.setFromPixels(video.getPixels(), video.width, video.height, OF_IMAGE_COLOR);


	if(kinect.isFrameNew()) {

		rawKinectInput.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height, OF_IMAGE_GRAYSCALE, false);
		rawKinectInput.crop(KINECT_CROP_LEFT, KINECT_CROP_TOP, 
			kinect.width - KINECT_CROP_LEFT - KINECT_CROP_RIGHT, kinect.height - KINECT_CROP_TOP - KINECT_CROP_BOTTOM);

	}

}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetColor(255, 255, 255);
	rawKinectInput.draw(0,0);

	// ofPushMatrix();
	// 	ofRotateZ(VIDEO_R);
	// 	rawVideoInput.draw(VIDEO_X, VIDEO_Y, VIDEO_W, VIDEO_H);
	// ofPopMatrix();


	ofPushStyle();
		ofSetColor(0, 255, 0);
		stringstream reportStream;
		// reportStream
		// << "x: " << x << endl
		// << "y: " << y << endl
		// << "w: " << w << endl
		// << "h: " << h << endl
		// << "r: " << r << endl;
		ofDrawBitmapString(reportStream.str(), 20, 652);
	ofPopStyle();

}

//--------------------------------------------------------------
void ofApp::exit(){

	kinect.close();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	// switch (key) {

	// 	case OF_KEY_UP:
	// 		y--;
	// 		break;
	// 	case OF_KEY_DOWN:
	// 		y++;
	// 		break;
	// 	case OF_KEY_LEFT:
	// 		x--;
	// 		break;
	// 	case OF_KEY_RIGHT:
	// 		x++;
	// 		break;
            
 //        case 'W':
 //            w++;
 //            break;
            
 //        case 'w':
 //            w--;
 //            break;

 //        case 'H':
 //            h++;
 //            break;
            
 //        case 'h':
 //            h--;
 //            break;

 //        case 'R':
 //            r += 0.1;
 //            break;
            
 //        case 'r':
 //            r -= 0.1;
 //            break;
            
            
		

	// }

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}


