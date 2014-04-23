#include "ArmContourFinder.h"

ArmContourFinder::ArmContourFinder() {

	// Find from the bounding boxes
	bounds.push_back(39);
	bounds.push_back(1);
	bounds.push_back(1514);
	bounds.push_back(1038);

	setMinArea(50);

	MIN_HAND_SIZE = 129;
	MAX_HAND_SIZE = 208;
	MAX_WRIST_WIDTH = 79;

}

void ArmContourFinder::update() {

	//To run every frame
	int size = polylines.size();

	ends.resize(size);
	wrists.resize(size);
	tips.resize(size);
	handFound.resize(size, false);

	for (int i = 0; i < size; ++i)
	{
		handFound[i] = findHand(i);
	}
}

ofPoint ArmContourFinder::getHandCentroid(int n) {

	// unsigned int n;
	// for (int i = 0; i < polylines.size(); ++i)
	// {
	// 	if(getLabel(i) == label) {
	// 		n = i;
	// 		break;
	// 	}
	// }

	if(!handFound[n]) return;

	ofPolyline hand;

	unsigned int start, end;
	polylines[n].getClosestPoint(wrists[n][1], &start);
	polylines[n].getClosestPoint(wrists[n][0], &end);


	int i = start;
	while( i != end ) {
		hand.addVertex( polylines[n][i] );
		i++;
		if( i == polylines[n].size() )
			i = 0;
	}
	// So that it closes up;
	hand.setClosed(true);

	return hand.getCentroid2D();

}

bool ArmContourFinder::findHand(int n) {

	ends[n].clear();
	tips[n] = ofPoint(0,0,0);
	wrists[n].clear();

	//First, find ends
	ends[n] = findEnds(n);
	//If failure
	if(ends[n].size() != 2) return false;

	//Now, get the tip
	tips[n] = findTip(n);
	//See if it's far enough away
	float d1 = ofDistSquared(tips[n].x, tips[n].y, ends[n][0].x, ends[n][0].y);
	float d2 = ofDistSquared(tips[n].x, tips[n].y, ends[n][1].x, ends[n][1].y);
	if( d1 < MIN_HAND_SIZE * MIN_HAND_SIZE and d2 < MIN_HAND_SIZE * MIN_HAND_SIZE )
		return false; // Too small!

	//Now find the wrists
	wrists[n] = findWrists(n);
	if( wrists[n].size() != 2 ) return false;

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

ofPoint ArmContourFinder::findTip(int n) {

	//Create a line connecting the center of the base of the arm to the center of the bounding box
	ofPoint boxCenter = ofxCv::toOf(getCenter(n));
	ofPoint baseCenter = ofPoint((ends[n][0].x + ends[n][1].x)/2, (ends[n][0].y + ends[n][1].y)/2 );
	// Slope of the line
	float m = (boxCenter.y - baseCenter.y) / (boxCenter.x - baseCenter.x);
	float yn = 5000;	// New y coordinate (far off)
	if(boxCenter.y < baseCenter.y) 
		yn *= -1;
	float xn = (yn - baseCenter.y) / m + baseCenter.x;
	ofPoint mostDistant = ofPoint(xn, yn);

	return polylines[n].getClosestPoint(mostDistant);

}

vector < ofPoint > ArmContourFinder::findWrists(int n) {

	//One polyline for each side of the hand
	ofPolyline sideOne, sideTwo;

	//The indeces at which the split will occur
	unsigned int start, endOne, endTwo;

	//Find those indeces, not sure if this is computationally expensive (premature optimization, anyone?)
	polylines[n].getClosestPoint(tips[n], &start);
	polylines[n].getClosestPoint(ends[n][0], &endOne);
	polylines[n].getClosestPoint(ends[n][1], &endTwo);

	//Square our distances now, because premature optimization
	int minSquared = MIN_HAND_SIZE * MIN_HAND_SIZE;
	int maxSquared = MAX_HAND_SIZE * MAX_HAND_SIZE;

	//Put all verteces within the right distance in one set
	int i = start;
	float distSquared; // The distance between points and the tip
	while(i != endOne and i != endTwo) {
		distSquared = ofDistSquared(tips[n].x, tips[n].y, polylines[n][i].x, polylines[n][i].y);
		if(distSquared <= maxSquared and distSquared >= minSquared)
			sideOne.addVertex( polylines[n][i] );
		i++;
		if( i == polylines[n].size() )
			i = 0;
	}

	//Now grab all the suitable ones on the other side
	i = start;
	while(i != endOne and i != endTwo) {
		distSquared = ofDistSquared(tips[n].x, tips[n].y, polylines[n][i].x, polylines[n][i].y);
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

