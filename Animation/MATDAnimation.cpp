#include "MATDAnimation.h"

void MATDAnimation::createAnimation() {
	// this is an example of the function to create an animation
	// this function create a rainbow-coloured circle
	
	DisplayContent dContent;

	// some parameters
	int numSamples = 4000;	// number of samples in total. Because it takes 25us to update a sample, updating 4000 samples takes 100ms (POV time)
	float radius = 0.015f;	// radius of the circle [m]

	dContent.positions.resize(numSamples);
	dContent.colours.resize(numSamples);
	for (int s = 0; s < numSamples; s++) {
		float radian = s * 2.0f * M_PI / numSamples;
		dContent.positions[s] = glm::vec3(radius * cos(radian), radius * sin(radian), 0);
		dContent.colours[s] = HSV2RGB(s * 360.0f / numSamples);
	}

	// the size of every variable does not have to be the same
	dContent.amplitudes.push_back(1.0f);
	dContent.phases.push_back(0.0f);
	dContent.twinFlags.push_back(true);

	dContents.push_back(dContent);
}

void MATDAnimation::runAnimation() {
	static glm::vec3 posBuffer[250], colourBuffer[250];
	static float ampBuffer[250], phaseBuffer[250];
	static bool twinBuffer[250];

	static glm::vec3 targetPosition, realPosition, currentPosition;
	static float stepSize = 0.0001f;

	int numFramesPerUpdate = 20;
	StateControl s(25 * numFramesPerUpdate);
	while (s.isWorking()) {
		 if (s.isReady()) {
			dContents[contentNumber].fillBuffers(numFramesPerUpdate, posBuffer, ampBuffer, phaseBuffer, colourBuffer, twinBuffer);
			mHandle.updateMultipleFrames(numFramesPerUpdate, posBuffer, ampBuffer, phaseBuffer, colourBuffer, twinBuffer);

			if (dContents[contentNumber].isChangeTiming() && dContents[contentNumber].updateFinished()) {
				dContents[contentNumber].resetAddressAndFlag();
				if (contentNumber == dContents.size() - 1) contentNumber = 0;
				else contentNumber++;
			}

			s.countTimer();
		}
	}
}


void MATDAnimation::goStartPosition(glm::vec3 initPosition) {
	// wait for the users until they put a bead at the initial position
	std::cout << "put a bead and then press any key to go to the start position" << std::endl;
	_getch();
	if (dContents[contentNumber].positions.size() != 0)
		mHandle.goToNextPosition(initPosition, dContents[contentNumber].positions[dContents[contentNumber].posAddress]);
}

void MATDAnimation::goBackInitPosition(glm::vec3 initPosition) {
	if (dContents[contentNumber].positions.size() != 0)
		mHandle.goToNextPosition(dContents[contentNumber].positions[dContents[contentNumber].posAddress], initPosition);
}

void MATDAnimation::positionCorrection() {

}


void MATDAnimation::multiplexMovingTactile(DisplayContent &visualContent, glm::vec3 tactilePosition, int frequency, int numLevitation, int numTactile) {
	float tactileRadius = 0.005f;
	int numTactileSamples = 40000 / frequency;

	for (int s = 0; s < visualContent.positions.size(); s++) {
		if (s % (numLevitation + numTactile) >= numLevitation) {
			float radian = s * 2.0f * M_PI / numTactileSamples;
			glm::vec3 tactileMovement(0.0f, tactileRadius * sin(radian), tactileRadius * cos(radian));

			visualContent.positions[s] = tactilePosition + tactileMovement;
		}
	}
}

glm::vec3 MATDAnimation::HSV2RGB(float hue) {
	glm::vec3 RGBColour;
	if (hue < 60) {
		RGBColour.r = 255;
		RGBColour.g = round((hue / 60.0) * 255);
		RGBColour.b = 0;
	}
	else if (hue < 120) {
		RGBColour.r = round(((120 - hue) / 60.0) * 255);
		RGBColour.g = 255;
		RGBColour.b = 0;
	}
	else if (hue < 180) {
		RGBColour.r = 0;
		RGBColour.g = 255;
		RGBColour.b = round(((hue - 120) / 60.0) * 255);
	}
	else if (hue < 240) {
		RGBColour.r = 0;
		RGBColour.g = round(((240 - hue) / 60.0) * 255);
		RGBColour.b = 255;
	}
	else if (hue < 300) {
		RGBColour.r = round(((hue - 240) / 60.0) * 255);
		RGBColour.g = 0;
		RGBColour.b = 255;
	}
	else {
		RGBColour.r = 255;
		RGBColour.g = 0;
		RGBColour.b = round(((360 - hue) / 60.0) * 255);;
	}
	return RGBColour;
}

glm::vec3 MATDAnimation::getImageColor(cv::Mat img, float px, float py, float width, float height) {

	unsigned int xIndex = round((px + width / 2.0) * img.cols / width); // x and y are fliped
	unsigned int yIndex = round((-py + height / 2.0) * img.rows / height);

	if (xIndex >= img.cols) xIndex = img.cols - 1;
	if (yIndex >= img.rows) yIndex = img.rows - 1;

	glm::vec3 colour;
	if (px < -width / 2.0 || px >= width / 2.0)
		colour = glm::vec3(0, 0, 0);
	else {
		colour.r = img.data[xIndex * img.channels() + yIndex * img.step + 2];
		colour.g = img.data[xIndex * img.channels() + yIndex * img.step + 1];
		colour.b = img.data[xIndex * img.channels() + yIndex * img.step + 0];
	}
	
	return colour;
}
