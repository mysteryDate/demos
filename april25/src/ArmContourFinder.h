#include "ContourFinder.h"
#include "ofMain.h"

class ArmContourFinder : public ofxCv::ContourFinder
{
public:

	ArmContourFinder();

	vector< ofPoint > ends;
	vector< ofPoint > tips;
	vector< vector< ofPoint > > wrists;
	vector< bool > handFound;

	void update();
	ofPolyline getHand(int n);
	
	int MIN_HAND_SIZE;
	int MAX_HAND_SIZE;
	int MAX_WRIST_WIDTH;
	int MIN_WRIST_WIDTH;
	int MAX_MOVEMENT_DISTANCE;

	vector< int > bounds;

	// Just so things remember their dies
	map< unsigned int, int > side;

private:

	bool findHand(int n);
	ofPoint			 	findEnd(int n);
	ofPoint				findTip(int n);
	vector< ofPoint > 	findWrists(int n);

	void addHand(int n);

};