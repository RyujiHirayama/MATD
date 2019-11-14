#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <limits>
#include <gtc/quaternion.hpp>
#include <gtc/matrix_transform.hpp>

// this class has an animation content (e.g. butterfly, smiley face and number)
// a display content consists of position, amplitude, phase, colour and twin trap flag (twin trap when it's true, just focusing point when false)

class DisplayContent {
	bool changeTiming;									// the MATD can update the next display content only when the trap position is at the start position of the animation (posAddress = 0)
	bool posFin, ampFin, phaseFin, colourFin, twinFin;	// when every data stored in a buffer of each variable have been updated, a finish flag turn to true
	glm::vec3 minPosition, maxPosition;
public:
	std::vector<glm::vec3> positions, colours;			// the range of position is (-128.0mm.. + 128.0mm), colour is [0..255].
	std::vector<float> amplitudes, phases;				// the ranage of  amplitude is [0..1], phase usually is [0..2PI) and rgb [0..255].
	std::vector<bool> twinFlags;						// flag incidates creating twin trap when it's true, just focusing point when false
	int posAddress, ampAddress, phaseAddress, colourAddress, twinAddress; // each variable need to have their own address because the sizes of the memories are different 

	DisplayContent() { resetAddressAndFlag(); };

	/*
		fill the buffers and change the current addresses
	*/
	void fillBuffers(int numFramesPerUpdate, glm::vec3 *posBuffer, float *ampBuffer, float *phaseBuffer, glm::vec3 *colourBuffer, bool *twinBuffer);

	void resetAddressAndFlag();

	bool updateFinished();


	bool isChangeTiming();

	/*
		get the minimum and maximum values of each axis of positions
	*/
	void getMinMaxPositions();

	void normalisePositions(float drawingSize);

	void shiftPositions(glm::vec3 shift);

	void rotatePositions(glm::mat3 rotate);

	void interporatePositions(float constantVelocity);

};