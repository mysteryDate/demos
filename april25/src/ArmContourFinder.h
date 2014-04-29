#include "ContourFinder.h"
#include "ofMain.h"

class ArmContourFinder : public ofxCv::ContourFinder
{
public:

	ArmContourFinder();

	// The keys are the labels
	map< unsigned int, ofPoint > ends;
	map< unsigned int, ofPoint > tips;
	map< unsigned int, vector< ofPoint > > wrists;
	map< unsigned int, bool > handFound;
	map< unsigned int, int > side;

	void update();
	ofPolyline getHand(int n);
	
	int MIN_HAND_SIZE;
	int MAX_HAND_SIZE;
	int MAX_WRIST_WIDTH;
	int MIN_WRIST_WIDTH;
	int MAX_MOVEMENT_DISTANCE;

	vector< int > bounds;

private:

	bool findHand(int n);
	ofPoint			 	findEnd(int n);
	ofPoint				findTip(int n);
	vector< ofPoint > 	findWrists(int n);
	ofPoint 			refitTip(int n);

	void addHand(int n);

};