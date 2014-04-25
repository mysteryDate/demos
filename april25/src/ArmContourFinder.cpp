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
	MIN_WRIST_WIDTH = 10;

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
	hand.addVertex( polylines[n][end] );
	
	// So that it closes up;
	hand.setClosed(true);

	return hand;

}

bool ArmContourFinder::findHand(int n) {

	// ends[n].clear();
	// tips[n] = ofPoint(0,0,0);
	// wrists[n].clear();

	//First, find ends
	ends[n] = findEnd(n);

	//Now, get the tip
	tips[n] = findTip(n);
	//See if it's far enough away
	float d1 = ofDistSquared(tips[n].x, tips[n].y, ends[n].x, ends[n].y);
	float d2 = ofDistSquared(tips[n].x, tips[n].y, ends[n].x, ends[n].y);
	if( d1 < MIN_HAND_SIZE * MIN_HAND_SIZE and d2 < MIN_HAND_SIZE * MIN_HAND_SIZE )
		return false; // Too small!

	//Now find the wrists
	wrists[n] = findWrists(n);
	if( wrists[n].size() != 2 ) return false;

	return true;

}

ofPoint ArmContourFinder::findEnd(int n) {

	vector< ofPoint > pts = polylines[n].getVertices();
	vector< ofPoint > endPoints;
	ofPoint center = ofxCv::toOf(getCenter(n));

	for (int i = 0; i < pts.size(); ++i)
	{
		if(pts[i].x <= bounds[0] + 0 || pts[i].y <= bounds[1] + 0
 			|| pts[i].x >= bounds[2] - 2 || pts[i].y >=  bounds[3] - 2) {
			endPoints.push_back(pts[i]);
		}
	}
	if(endPoints.size() > 0) {
		// Just take the one that's the farthest from the center of the box
		float maxDist = 0;
		for (int i = 0; i < endPoints.size(); ++i)
		{
			float dist = ofDistSquared(center.x, center.y, endPoints[i].x, endPoints[i].y);
			if(dist > maxDist) {
				maxDist = dist;
				endPoints[0] = endPoints[i];
			}
		}

		endPoints.resize(1);
	}
	if(endPoints.size() == 0) {
		ofPoint centroid = polylines[n].getCentroid2D();
		ofPoint mark = ofPoint(centroid.x, bounds[3]); // TODO, any side
		endPoints.push_back(polylines[n].getClosestPoint(mark));
	}

	// New tactic!
	vector< ofPoint > rotatedRect = ofxCv::toOf(getMinAreaRect(n)).getVertices();

	// Remove two farthest from endpoint
	for (int i = 0; i < 2; ++i)
	{
		float maxDist = 0;			
		int indexToRemove;
		for (int i = 0; i < rotatedRect.size(); ++i)
		{
			float dist = ofDistSquared(endPoints[0].x, endPoints[0].y, rotatedRect[i].x, rotatedRect[i].y);
			if(dist > maxDist) {
				maxDist = dist;
				indexToRemove = i;
			}
		}
		rotatedRect.erase(rotatedRect.begin() + indexToRemove);
	}

	ofPoint end;

	float maxDist = 0;
	for (int i = 0; i < rotatedRect.size(); ++i)
	{
		float dist = ofDistSquared(rotatedRect[i].x, rotatedRect[i].y, center.x, center.y);
		if(dist > maxDist) {
			maxDist = dist;
			end = rotatedRect[i];
		}
	}

	return end;

}

ofPoint ArmContourFinder::findTip(int n) {

	//Create a line connecting the center of the base of the arm to the center of the bounding box
	ofPoint boxCenter = ofxCv::toOf(getCenter(n));

	// Slope of the line
	float m = (boxCenter.y - ends[n].y) / (boxCenter.x - ends[n].x);
	float yn = 2*(boxCenter.y - ends[n].y);	// New y coordinate (far off)
	// if(boxCenter.y < ends[n].y) 
	// 	yn *= -1;
	float xn = (yn - ends[n].y) / m + ends[n].x;
	ofPoint mostDistant = ofPoint(xn, yn);

	ofPoint newTip = polylines[n].getClosestPoint(mostDistant);

	//If our old tip is still good, keep it
	if(tips.size() > n) {
		ofPoint closestTip = polylines[n].getClosestPoint(tips[n]);
		float dist = ofDistSquared(closestTip.x, closestTip.y, newTip.x, newTip.y);
		if(dist < 100) { //TODO change magic number
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
	int minWrist = MIN_WRIST_WIDTH * MIN_WRIST_WIDTH;
	float distSquared;
	if(wrists[n].size() == 2) {
		//If the old wrists still work, keep em
		vector< ofPoint > closestWrists;
		closestWrists.push_back(polylines[n].getClosestPoint(wrists[n][0]));
		closestWrists.push_back(polylines[n].getClosestPoint(wrists[n][1]));
		distSquared = ofDistSquared(closestWrists[0].x, closestWrists[0].y, closestWrists[1].x, closestWrists[1].y);
		if(distSquared <= maxWrist and distSquared >= minWrist) {
			float d1 = ofDistSquared(closestWrists[0].x, closestWrists[0].y, tips[n].x, tips[n].y);
			float d2 = ofDistSquared(closestWrists[1].x, closestWrists[1].y, tips[n].x, tips[n].y);
			if(d1 >= minSquared and d1 <= maxSquared and d2 >= minSquared and d2 <= maxSquared)
				return closestWrists;
		}
	}

	//One polyline for each side of the hand
	ofPolyline sideOne, sideTwo;

	//The indeces at which the split will occur
	unsigned int start, end;

	//Find those indeces, not sure if this is computationally expensive (premature optimization, anyone?)
	polylines[n].getClosestPoint(tips[n], &start);
	polylines[n].getClosestPoint(ends[n], &end);


	//Put all verteces within the right distance in one set
	int i = start;
	while(i != end) {
		distSquared = ofDistSquared(tips[n].x, tips[n].y, polylines[n][i].x, polylines[n][i].y);
		if(distSquared <= maxSquared and distSquared >= minSquared)
			sideOne.addVertex( polylines[n][i] );
		i++;
		if( i == polylines[n].size() )
			i = 0;
	}

	//Now grab all the suitable ones on the other side
	i = start;
	while(i != end) {
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
			if(distSquared < shortestDist and distSquared <= maxWrist and distSquared >= minWrist) {
				possibleWrists.resize(2);
				possibleWrists[0] = sideOne[i];
				possibleWrists[1] = sideTwo[j];
				shortestDist = distSquared;
			}
		}
	}

	return possibleWrists;

}

