#include "ArmContourFinder.h"

ArmContourFinder::ArmContourFinder() {

	bounds.push_back(0);
	bounds.push_back(0);
	bounds.push_back(630);
	bounds.push_back(470);

	setMinArea(50);

	MIN_HAND_SIZE = 50;
	MAX_HAND_SIZE = 90;
	MAX_WRIST_WIDTH = 40;

	MAX_MOVEMENT_DISTANCE = 20;
	SMOOTHING_RATE = 0.5;

}

void ArmContourFinder::update() {

	//To run every frame
	int size = polylines.size();

	ends.resize(size);
	wrists.resize(size);
	tips.resize(size);
	hands.resize(size);
	handFound.resize(size, false);


	// First put the info with the right blob (HACK, this whole experiment should probably be a linked list)
	oldLabels.resize(size);
	vector< vector< ofPoint > > oldEnds = ends;
	vector< ofPoint > oldTips = tips;
	vector< vector< ofPoint > > oldWrists = wrists;
	vector< bool > oldHandFound = handFound;
	vector< ofPolyline > oldHands = hands;

	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			if(oldLabels[i] == getLabel(j)) {
				ends[i] = oldEnds[j];
				tips[i] = oldTips[j];
				wrists[i] = oldWrists[j];
				handFound[i] = handFound[j];
				hands[i] = hands[j];
			}
		}
	}


	for (int i = 0; i < size; ++i)
	{
		polylines[i] = polylines[i].getSmoothed(4);
		oldLabels[i] = getLabel(i);
		if(handFound[i]) {
			updateArm(i);
		}
		else {
			handFound[i] = findHand(i);
			if(handFound[i]) {
				addHand(i);
			}
		}
	}
}

void ArmContourFinder::updateArm(int n) {

	ofPoint * keypoints[] = {&ends[n][0], &ends[n][1], &tips[n], &wrists[n][0], &wrists[n][1]};
	ofVec2f velocity = ofxCv::toOf(getVelocity(n));
	
	vector< ofPoint > newEnds = findEnds(n);
	if( newEnds.size() != 2 ) {
		handFound[n] = false;
		return;
	}
	ofPoint newTip = findTip(n, newEnds);
	vector< ofPoint > newWrists = findWrists(n, newTip, newEnds);
	if( newWrists.size() != 2 ) {
		handFound[n] = false;
		return;
	}

	ofPoint newKeypoints [] = {newEnds[0], newEnds[1], newTip, newWrists[0], newWrists[1]};

	//if within a minimum dist, don't move at all
	int noiseDist = 100;
	float changed = false;

	for (int i = 0; i < 5; ++i)
	{
		if( ofDistSquared(keypoints[i]->x, keypoints[i]->y, newKeypoints[i].x, newKeypoints[i].y ) > noiseDist ) {
			float smoothedX = ofLerp(keypoints[i]->x, newKeypoints[i].x, SMOOTHING_RATE);
			float smoothedY = ofLerp(keypoints[i]->y, newKeypoints[i].y, SMOOTHING_RATE);
			newKeypoints[i] = ofPoint(smoothedX, smoothedY);
			*keypoints[i] = newKeypoints[i];
			changed = true;
		}
	}
	
	if(changed)
		addHand(n);

}

void ArmContourFinder::addHand(int n) {

	hands[n].clear();
	unsigned int start, end;
	polylines[n].getClosestPoint(wrists[n][1], &start);
	polylines[n].getClosestPoint(wrists[n][0], &end);

	int i = start;
	while( i != end ) {
		hands[n].addVertex( polylines[n][i] );
		i++;
		if( i == polylines[n].size() )
			i = 0;
	}
	// So that it closes up;
	hands[n].setClosed(true);



}

bool ArmContourFinder::findHand(int n) {

	//First, find ends
	vector< ofPoint > newEnds = findEnds(n);
	//If failure
	if(newEnds.size() != 2) return false;

	//Now, get the tip
	ofPoint newTip = findTip(n, newEnds);
	//See if it's far enough away
	float d1 = ofDistSquared(newTip.x, newTip.y, newEnds[0].x, newEnds[0].y);
	float d2 = ofDistSquared(newTip.x, newTip.y, newEnds[1].x, newEnds[1].y);
	if( d1 < MIN_HAND_SIZE * MIN_HAND_SIZE and d2 < MIN_HAND_SIZE * MIN_HAND_SIZE )
		return false; // Too small!

	//Now find the wrists
	vector< ofPoint > newWrists = findWrists(n, newTip, newEnds);
	if( newWrists.size() != 2 ) return false;

	ends[n] = newEnds;
	tips[n] = newTip;
	wrists[n] = newWrists;

	return true;

}

vector< ofPoint > ArmContourFinder::findEnds(int n) {

	//First, find ends
	vector< ofPoint > pts = polylines[n].getVertices();
	vector< ofPoint > endPoints;

	for (int i = 0; i < pts.size(); ++i)
	{
		if(pts[i].x <= bounds[0] + 5 || pts[i].y <= bounds[1] + 5
 			|| pts[i].x >= bounds[2] - 5 || pts[i].y >=  bounds[3] - 5) {
			endPoints.push_back(pts[i]);
		}
	}
	if(endPoints.size() >= 2) {
		//Assume the first one is right, find the most distant second one
		float maxDist = 0;
		for (int j = 1; j < endPoints.size(); ++j)
		{
			float dist = ofDistSquared(endPoints[0].x, endPoints[0].y, endPoints[j].x, endPoints[j].y);
			if(dist > maxDist) {
				maxDist = dist;
				endPoints[1] = endPoints[j];
			}
		}
		endPoints.resize(2);
	}

	return endPoints;

}

ofPoint ArmContourFinder::findTip(int n, vector< ofPoint > newEnds) {

	//Create a line connecting the center of the base of the arm to the center of the bounding box
	ofPoint boxCenter = ofxCv::toOf(getCenter(n));
	ofPoint baseCenter = ofPoint((newEnds[0].x + newEnds[1].x)/2, (newEnds[0].y + newEnds[1].y)/2 );
	// Slope of the line
	float m = (boxCenter.y - baseCenter.y) / (boxCenter.x - baseCenter.x);
	float yn = 5000;	// New y coordinate (far off)
	if(boxCenter.y < baseCenter.y) 
		yn *= -1;
	float xn = (yn - baseCenter.y) / m + baseCenter.x;
	ofPoint mostDistant = ofPoint(xn, yn);

	return polylines[n].getClosestPoint(mostDistant);

}

vector < ofPoint > ArmContourFinder::findWrists(int n, ofPoint newTip, vector< ofPoint > newEnds) {

	//One polyline for each side of the hand
	ofPolyline sideOne, sideTwo;

	//The indeces at which the split will occur
	unsigned int start, endOne, endTwo;

	//Find those indeces, not sure if this is computationally expensive (premature optimization, anyone?)
	polylines[n].getClosestPoint(newTip, &start);
	polylines[n].getClosestPoint(newEnds[0], &endOne);
	polylines[n].getClosestPoint(newEnds[1], &endTwo);

	//Square our distances now, because premature optimization
	int minSquared = MIN_HAND_SIZE * MIN_HAND_SIZE;
	int maxSquared = MAX_HAND_SIZE * MAX_HAND_SIZE;

	//Put all verteces within the right distance in one set
	int i = start;
	float distSquared; // The distance between points and the tip
	while(i != endOne and i != endTwo) {
		distSquared = ofDistSquared(newTip.x, newTip.y, polylines[n][i].x, polylines[n][i].y);
		if(distSquared <= maxSquared and distSquared >= minSquared)
			sideOne.addVertex( polylines[n][i] );
		i++;
		if( i == polylines[n].size() )
			i = 0;
	}

	//Now grab all the suitable ones on the other side
	i = start;
	while(i != endOne and i != endTwo) {
		distSquared = ofDistSquared(newTip.x, newTip.y, polylines[n][i].x, polylines[n][i].y);
		if(distSquared <= maxSquared and distSquared >= minSquared)
			sideTwo.addVertex( polylines[n][i] );
		i--;
		if( i < 0 )
			i = polylines[n].size() - 1;
	}

	// Now find the closest two points on these lines
	float shortestDist = MAX_WRIST_WIDTH * MAX_WRIST_WIDTH + 1;
	vector< ofPoint > possibleWrists;
	for (int i = 0; i < sideOne.size(); ++i)
	{
		for (int j = 0; j < sideTwo.size(); ++j)
		{
			distSquared = ofDistSquared(sideOne[i].x, sideOne[i].y, sideTwo[j].x, sideTwo[j].y);
			if(distSquared < shortestDist and distSquared <= MAX_WRIST_WIDTH * MAX_WRIST_WIDTH) {
				possibleWrists.resize(2);
				possibleWrists[0] = sideOne[i];
				possibleWrists[1] = sideTwo[j];
				shortestDist = distSquared;
			}
		}
	}

	return possibleWrists;

}

