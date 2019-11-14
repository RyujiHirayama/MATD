#pragma once
#include <sstream>
#include <opencv2/opencv.hpp>	// we use openCV to import image files, which are used for the texture of the 3D shape


#include "DisplayContent.h"
#include "../Levitator/MATDHandle.h"
#include "../Levitator/StateControl.h"
#include "../Utilities/AudioProcessing.h"

class MATDAnimation {
protected:
	int contentNumber;
	std::vector<DisplayContent> dContents;
	MATDHandle &mHandle;
public:

	MATDAnimation(MATDHandle& mHandle) : mHandle(mHandle), contentNumber(0) { ; };

	/*
		create an animation (pre-compute the position, colour etc)
		this function needs to be changed depending on the animation
	*/
	virtual void createAnimation();

	/*
		run the animation 
	*/
	virtual void runAnimation();

	/*
		wait until user put a bead and go from the initial position to the start position of the animation
	*/
	void goStartPosition(glm::vec3 initPosition = glm::vec3(0.0f, 0.0f, 0.0f));

	/* 
		when animation stops, the bead automatically goes back to the initial position
	*/
	void goBackInitPosition(glm::vec3 initPosition = glm::vec3(0.0f, 0.0f, 0.0f));

	void positionCorrection();

	/*
		multiplex a tacitle content in the display content
	*/
	virtual	void multiplexMovingTactile(DisplayContent &visualContent, glm::vec3 tactilePosition, int frequency, int numLevitation = 3, int numTactile = 1);

	/*
		get the RGB values from a hue value (actually this method does not use S and V...)
	*/
	glm::vec3 HSV2RGB(float hue);

	/*
		get the colour of the image plane according to the pixel position
	*/
	glm::vec3 getImageColor(cv::Mat img, float px, float py, float width, float height);
};