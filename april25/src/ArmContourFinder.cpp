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
	MIN_WRIST_WIDTH = 15;

}

void ArmContourFinder::update() {

	//To run every frame

	for (int i = 0; i < polylines.size(); ++i)
	{
		handFound[getLabel(i)] = findHand(i);
	}
}

ofPolyline ArmContourFinder::getHand(int n) {

	if(!handFound[getLabel(n)]) return;

	ofPolyline hand;

	unsigned int start, end;
	polylines[n].getClosestPoint(wrists[getLabel(n)][1], &start);
	polylines[n].getClosestPoint(wrists[getLabel(n)][0], &end);


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

	unsigned int l = getLabel(n);
	//First, find ends
	ends[l] = findEnd(n);
	if( ends[l].x == -1 and ends[l].y == -1)
		return false;

	//Now, get the tip
	tips[l] = findTip(n);
	//See if it's far enough away
	float d1 = ofDistSquared(tips[l].x, tips[l].y, ends[l].x, ends[l].y);
	float d2 = ofDistSquared(tips[l].x, tips[l].y, ends[l].x, ends[l].y);
	if( d1 < MIN_HAND_SIZE * MIN_HAND_SIZE and d2 < MIN_HAND_SIZE * MIN_HAND_SIZE )
		return false; // Too small!

	//Now find the wrists
	wrists[l] = findWrists(n);
	if( wrists[l].size() != 2 ) return false;

	tips[l] = refitTip(n);

	d1 = ofDistSquared(wrists[l][0].x, wrists[l][0].y, tips[l].x, tips[l].y);
	d2 = ofDistSquared(wrists[l][1].x, wrists[l][1].y, tips[l].x, tips[l].y);
	if(d1 <= MIN_HAND_SIZE * MIN_HAND_SIZE or d1 >= MAX_HAND_SIZE * MAX_HAND_SIZE 
		or d2 <= MIN_HAND_SIZE * MIN_HAND_SIZE or d2 >= MAX_HAND_SIZE * MAX_HAND_SIZE) {
		wrists[l] = findWrists(n);
		if(wrists[l].size() != 2)
			return false;
	}

	return true;

}

ofPoint ArmContourFinder::findEnd(int n) {

	vector< ofPoint > pts = polylines[n].getVertices();
	vector< ofPoint > endPoints;
	ofPoint center = ofxCv::toOf(getCenter(n));
	unsigned int l = getLabel(n);

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

		if(endPoints[0].x <= bounds[0] + 10) side[l] = 0;
		else if(endPoints[0].y <= bounds[1] + 10) side[l] = 1;
		else if(endPoints[0].x >= bounds[2] - 10) side[l] = 2;
		else if(endPoints[0].y >= bounds[3] - 10) side[l] = 3;
	}


	if(endPoints.size() == 0) {
		if(handFound[l]) {
			ofPoint centroid = polylines[n].getCentroid2D();
			int thisSide = side[l];
			// assume they're still on the same side
			ofPoint mark;
			if(thisSide == 0 or thisSide == 2)
				mark = ofPoint(bounds[thisSide], centroid.y); // TODO, any side
			else
				mark = ofPoint(centroid.x, bounds[thisSide]);
			endPoints.push_back(polylines[n].getClosestPoint(mark));
		}
		else return ofPoint(-1, -1);
	}



	// New tactic!
	ofPolyline rotatedRect = ofxCv::toOf(getMinAreaRect(n));
	vector< ofPoint > verts = rotatedRect.getVertices();

	// Remove two farthest from endpoint
	for (int i = 0; i < 2; ++i)
	{
		float maxDist = 0;			
		int indexToRemove;
		for (int i = 0; i < verts.size(); ++i)
		{
			float dist = ofDistSquared(endPoints[0].x, endPoints[0].y, verts[i].x, verts[i].y);
			if(dist > maxDist) {
				maxDist = dist;
				indexToRemove = i;
			}
		}
		verts.erase(verts.begin() + indexToRemove);
	}


	ofPoint end;

	float maxDist = 0;
	vector< float > distances;
	for (int i = 0; i < verts.size(); ++i)
	{
		float dist = ofDistSquared(verts[i].x, verts[i].y, center.x, center.y);
		distances.push_back(dist);
		if(dist > maxDist) {
			maxDist = dist;
			end = verts[i];
		}
	}

	if(distances.size() == 2) {
		if( abs(distances[0] - distances[1]) < 400 ) {
			float d1 = ofDistSquared(ends[l].x, ends[l].y, verts[0].x, verts[0].y);
			float d2 = ofDistSquared(ends[l].x, ends[l].y, verts[1].x, verts[1].y);
			if(d1 < d2)
				end = verts[0];
			else
				end = verts[1];
		}
	}


	return end;

}

ofPoint ArmContourFinder::findTip(int n) {

	//Create a line connecting the center of the base of the arm to the center of the bounding box
	ofPoint boxCenter = ofxCv::toOf(getCenter(n));
	unsigned int l = getLabel(n);

	float xn = 3*boxCenter.x - 2*ends[l].x;
	float yn = 3*boxCenter.y - 2*ends[l].y;
	
	ofPoint mark = ofPoint(xn, yn);

	ofPoint newTip = polylines[n].getClosestPoint(mark);

	//If our old tip is still good, keep it
	ofPoint closestTip = polylines[n].getClosestPoint(tips[getLabel(n)]);
	float dist = ofDistSquared(closestTip.x, closestTip.y, newTip.x, newTip.y);
	if(dist < 100) { //TODO change magic number
		return closestTip;
	}

	return newTip;

}

vector < ofPoint > ArmContourFinder::findWrists(int n) {

	unsigned int l = getLabel(n);
	//Square our distances now, because premature optimization
	int minSquared = MIN_HAND_SIZE * MIN_HAND_SIZE;
	int maxSquared = MAX_HAND_SIZE * MAX_HAND_SIZE;
	int maxWrist = MAX_WRIST_WIDTH * MAX_WRIST_WIDTH;
	int minWrist = MIN_WRIST_WIDTH * MIN_WRIST_WIDTH;
	float distSquared;
	if(wrists[l].size() == 2) {
		//If the old wrists still work, keep em
		vector< ofPoint > closestWrists;
		closestWrists.push_back(polylines[n].getClosestPoint(wrists[l][0]));
		closestWrists.push_back(polylines[n].getClosestPoint(wrists[l][1]));
		distSquared = ofDistSquared(closestWrists[0].x, closestWrists[0].y, closestWrists[1].x, closestWrists[1].y);
		if(distSquared <= maxWrist and distSquared >= minWrist) {
			float d1 = ofDistSquared(closestWrists[0].x, closestWrists[0].y, tips[l].x, tips[l].y);
			float d2 = ofDistSquared(closestWrists[1].x, closestWrists[1].y, tips[l].x, tips[l].y);
			if(d1 >= minSquared and d1 <= maxSquared and d2 >= minSquared and d2 <= maxSquared)
				return closestWrists;
		}
	}

	//One polyline for each side of the hand
	ofPolyline sideOne, sideTwo;

	//The indeces at which the split will occur
	unsigned int start, end;

	//Find those indeces, not sure if this is computationally expensive (premature optimization, anyone?)
	polylines[n].getClosestPoint(tips[l], &start);
	polylines[n].getClosestPoint(ends[l], &end);


	//Put all verteces within the right distance in one set
	int i = start;
	while(i != end) {
		distSquared = ofDistSquared(tips[l].x, tips[l].y, polylines[n][i].x, polylines[n][i].y);
		if(distSquared <= maxSquared and distSquared >= minSquared)
			sideOne.addVertex( polylines[n][i] );
		i++;
		if( i == polylines[n].size() )
			i = 0;
	}

	//Now grab all the suitable ones on the other side
	i = start;
	while(i != end) {
		distSquared = ofDistSquared(tips[l].x, tips[l].y, polylines[n][i].x, polylines[n][i].y);
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

ofPoint ArmContourFinder::refitTip(int n)
{	
	unsigned int l = getLabel(n);
	ofPoint midWrist = ofPoint( (wrists[l][0].x + wrists[l][1].x)/2, (wrists[l][0].y + wrists[l][1].y)/2 );

	unsigned int start, end;
	polylines[n].getClosestPoint(wrists[l][1], &start);
	polylines[n].getClosestPoint(wrists[l][0], &end);

	ofPoint newTip;
	float maxDist = 0;
	int i = start;
	while( i != end ) {
		float dist = ofDistSquared(midWrist.x, midWrist.y, polylines[n][i].x, polylines[n][i].y);
		if(dist > maxDist) {
			maxDist = dist;
			newTip = polylines[n][i];
		}
		i++;
		if( i == polylines[n].size() )
			i = 0;
	}

	ofPoint closestTip = polylines[n].getClosestPoint(tips[getLabel(n)]);
	float dist = ofDistSquared(closestTip.x, closestTip.y, newTip.x, newTip.y);
	if(dist < 100) { //TODO change magic number
		return closestTip;
	}
	
	return newTip;
}

