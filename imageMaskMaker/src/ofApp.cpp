#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	img.loadImage("riviere_masque_alpha.png");

	unsigned char *pix = img.getPixels();

	int numPix = img.getWidth() * img.getHeight() * 4;

	for (int i = 0; i < numPix; i+=4)
	{
		unsigned char r = pix[i];
		unsigned char g = pix[i + 1];
		unsigned char b = pix[i + 2];
		unsigned char a = pix[i + 3];

		// Anything but black
		if( !(r < 100 and g < 100 and b < 100) ) {
			pix[i + 3] = 0;
		}	
		else {
			pix[i] = 0;
			pix[i + 1] = 0;
			pix[i + 2] = 0;
		}


	}
    
    img2.setFromPixels(img.getPixels(), img.getWidth(), img.getHeight(), OF_IMAGE_COLOR_ALPHA);
    
    img2.saveImage("river_mask_processed.png");

}

//--------------------------------------------------------------
void ofApp::update(){



}

//--------------------------------------------------------------
void ofApp::draw(){

	img2.draw(0,0);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
