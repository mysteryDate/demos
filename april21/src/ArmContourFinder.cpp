#include "ArmContourFinder.h"

ArmContourFinder::ArmContourFinder() {

	bounds.push_back(0);
	bounds.push_back(0);
	bounds.push_back(630);
	bounds.push_back(480);

	setMinArea(50);

}

void ArmContourFinder::update() {

	//To run every frame
	int size = polylines.size();

	simplifiedPolylines.resize(size);
	ends.resize(size);
	wrists.resize(size);
	tips.resize(size);

	for (int i = 0; i < size; ++i)
	{
		simplifiedPolylines[i] = polylines[i];
		simplifiedPolylines[i].simplify(tolerance);
	}

}

void ArmContourFinder::findHand(int n) {

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

	if(endPoints.size() != 2) return;

	ends[n] = endPoints;

	//Now, get the tip
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

	tips[n] = simplifiedPolylines[n].getClosestPoint(mostDistant);

}

