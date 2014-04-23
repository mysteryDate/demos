#include "ContourFinder.h"
#include "ofMain.h"

class ArmContourFinder : public ofxCv::ContourFinder
{
public:

	ArmContourFinder();

	vector< vector< ofPoint > > ends;
	vector< ofPoint > tips;
	vector< vector< ofPoint > > wrists;
	vector< bool > handFound;

	void update();
	ofPolyline getHand(int n);
	
	int MIN_HAND_SIZE;
	int MAX_HAND_SIZE;
	int MAX_WRIST_WIDTH;
	int MAX_MOVEMENT_DISTANCE;

	vector< int > bounds;

private:

	bool findHand(int n);
	vector< ofPoint > 	findEnds(int n);
	ofPoint				findTip(int n);
	vector< ofPoint > 	findWrists(int n);

	void addHand(int n);

};