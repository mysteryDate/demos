#include "ArmContourFinder.h"

ArmContourFinder::ArmContourFinder() {

	// Find from the bounding boxes
	bounds.push_back(1);
	bounds.push_back(1);
	bounds.push_back(601);
	bounds.push_back(438);

	setMinArea(50);

	MIN_HAND_SIZE = 56;
	MAX_HAND_SIZE = 99;
	MAX_WRIST_WIDTH = 33;

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

ofPolyline ArmContourFinder::getHand(int n) {

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

	return hand;

}

bool ArmContourFinder::findHand(int n) {

	// ends[n].clear();
	// tips[n] = ofPoint(0,0,0);
	// wrists[n].clear();

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

	// if ends are still good, use em
	if(ends[n].size() == 2) {
		vector < ofPoint > closestEnds;
		closestEnds.push_back(polylines[n].getClosestPoint(ends[n][0]));
		closestEnds.push_back(polylines[n].getClosestPoint(ends[n][1]));
		if( (closestEnds[0].x <= bounds[0] + 0 || closestEnds[0].y <= bounds[1] + 0
 			|| closestEnds[0].x >= bounds[2] - 2 || closestEnds[0].y >=  bounds[3] - 2) and 
			(closestEnds[0].x <= bounds[0] + 0 || closestEnds[0].y <= bounds[1] + 0
 			|| closestEnds[0].x >= bounds[2] - 2 || closestEnds[0].y >=  bounds[3] - 2) )
				//Biggest conditional statement ever? Seeing if the two are still on the edge
				return closestEnds;
	}

	vector< ofPoint > pts = polylines[n].getVertices();
	vector< ofPoint > endPoints;

	for (int i = 0; i < pts.size(); ++i)
	{
		if(pts[i].x <= bounds[0] + 0 || pts[i].y <= bounds[1] + 0
 			|| pts[i].x >= bounds[2] - 2 || pts[i].y >=  bounds[3] - 2) {
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

	ofPoint newTip = polylines[n].getClosestPoint(mostDistant);

	//If our old tip is still good, keep it
	if(tips.size() > n) {
		ofPoint closestTip = polylines[n].getClosestPoint(tips[n]);
		float dist = ofDistSquared(closestTip.x, closestTip.y, tips[n].x, tips[n].y);
		if(dist < 10) { //TODO change magic number
			return closestTip;
		}
	}

	return newTip;

}

vector < ofPoint > ArmContourFinder::findWrists(int n) {

	//Square our distances now, because premature optimization
	int minSquared = MIN_HAND_SIZE * MIN_HAND_SIZE;
	int maxSquared = MAX_HAND_SIZE * MAX_HAND_SIZE;
	int maxWrist = MAX_WRIST_WIDTH * MAX_WRIST_WIDTH;
	float distSquared;
	if(wrists[n].size() == 2) {
		//If the old wrists still work, keep em
		vector< ofPoint > closestWrists;
		closestWrists.push_back(polylines[n].getClosestPoint(wrists[n][0]));
		closestWrists.push_back(polylines[n].getClosestPoint(wrists[n][1]));
		distSquared = ofDistSquared(closestWrists[0].x, closestWrists[0].y, closestWrists[1].x, closestWrists[1].y);
		if(distSquared <= maxWrist) {
			float d1 = ofDistSquared(closestWrists[0].x, closestWrists[0].y, tips[n].x, tips[n].y);
			float d2 = ofDistSquared(closestWrists[1].x, closestWrists[1].y, tips[n].x, tips[n].y);
			if(d1 >= minSquared and d1 <= maxSquared and d2 >= minSquared and d2 <= maxSquared)
				return closestWrists;
		}
	}

	//One polyline for each side of the hand
	ofPolyline sideOne, sideTwo;

	//The indeces at which the split will occur
	unsigned int start, endOne, endTwo;

	//Find those indeces, not sure if this is computationally expensive (premature optimization, anyone?)
	polylines[n].getClosestPoint(tips[n], &start);
	polylines[n].getClosestPoint(ends[n][0], &endOne);
	polylines[n].getClosestPoint(ends[n][1], &endTwo);


	//Put all verteces within the right distance in one set
	int i = start;
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
	float shortestDist = maxWrist + 1;
	vector< ofPoint > possibleWrists;
	for (int i = 0; i < sideOne.size(); ++i)
	{
		for (int j = 0; j < sideTwo.size(); ++j)
		{
			distSquared = ofDistSquared(sideOne[i].x, sideOne[i].y, sideTwo[j].x, sideTwo[j].y);
			if(distSquared < shortestDist and distSquared <= maxWrist) {
				possibleWrists.resize(2);
				possibleWrists[0] = sideOne[i];
				possibleWrists[1] = sideTwo[j];
				shortestDist = distSquared;
			}
		}
	}

	return possibleWrists;

}

