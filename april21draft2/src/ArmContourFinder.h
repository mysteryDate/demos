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
	vector< ofPolyline > hands;

	// Matrix of how many skips have happened, first dimension is id, then end1, end2, tip
	vector< vector< int > > skippedFrames;

	vector< unsigned int > oldLabels;



	void update();
	
	bool findHand(int n);

	float tolerance;
	float SMOOTHING_RATE;

	int MIN_HAND_SIZE;
	int MAX_HAND_SIZE;
	int MAX_WRIST_WIDTH;
	int MIN_WRIST_WIDTH;
	int MAX_MOVEMENT_DISTANCE;
	int SURVIVAL_FRAMES;

	vector< ofPoint > oldCentroids;
	vector< ofPoint > oldTips;

	vector< int > bounds;

private:



	vector< ofPoint > 	findEnds(int n);
	ofPoint				findTip(int n, vector< ofPoint > newEnds);
	vector< ofPoint > 	findWrists(int n, ofPoint newTip, vector< ofPoint > newEnds);

	void updateArm(int n);
	void addHand(int n);

};