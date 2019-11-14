#pragma once
// This class is for the multimodal acoustic trap display (MATD)
// The paper title is "A volumetric display for visual, tactile and audio presentation using acoustic trapping".

#include <vector>
#include <gtc/quaternion.hpp>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h> 
#include "../Utilities/COMToolkit2.h"

#define Baudrate 12000000			// The baud rate is set to 12 MHz. To change this, you need to change the VHDL code. 
#define DiscretePhaseMax 128		// The maximum numbers of normalized phases 
#define DiscreteAmplitudeMax 255	// The maximum numbers of normalized amplitudes 
#define CalibCommand 252			// Commands for sending the calibration data
#define SwapCommand 251				// Commands for sending the swap signal
#define BoardHeight 0.2388f			// The acoustic distance between the bottom and top boards

class MATDHandle {
	enum MATDState { INIT = 0, CONNECTED = 1};
	int status;
	COMToolkit2 ctBottom, ctTop;

public:
	MATDHandle() :status(INIT) { ; };
	~MATDHandle() { disconnect(); };	// automatically disconnected from the boards

	/*
		Connects to the board, using the ports specified.
		This corresponds to the index of the COM port (e.g. COM7 would be port number 7).
		Returns true if connection was succesfull.
	*/
	bool connect(int portBottom, int portTop);

	/*
		This method disconnects the board, closing COM ports.
	*/
	void disconnect();


	/*
		This board uses discrete representations for the phases and amplitudes.
		That is, while our pahses can be any real number (we usually think of them in the range [0..2PI)), the board only uses 128 discrete values.
		This method computes the set of discretized phases from an input set of (real) phases.
		The number of elements in the array corresponds to the number of transducers in AsierInho.
	*/
	unsigned char _discretizePhase(float phase);

	/*
		Amplitudes are supposed to be in the range [0..1], and the board uses 256 discrete values.
	*/
	unsigned char _discretizeAmplitude(float amplitude);

	/*
		Each board needs its own calibration data.
		The data need to be sent to the boards first.
	*/
	void sendCalibData();

	/*
		MATD needs a position (XYZ), amplitude, phase and colour (RGB) for a frame.
		4 bytes for position (10bits to encode each XYZ, 0.25 mm resolution), 1 byte each for amplitude and phase and 3 bytes for colour (1 byte each for RGB), so 9 bytes per array of transducers.
		This method assumes x and y (-128.0mm..+128.0mm), z [0mm..+256.0mm], amplitude [0..1], phase usually [0..2PI) and rgb [0..255].
	*/
	void _encodeFrame(float x, float y, float z, float amplitude, float phase, unsigned char red, unsigned char green, unsigned char blue, unsigned char *message);


	/*
		Update a frame
		This method assumes xyz (-128.0mm..+128.0mm), amplitude [0..1], phase usually [0..2PI) and rgb [0..255].
	*/
	void updateFrame(glm::vec3 position, float amplitude = 1.0f, float phase = 0.0f, glm::vec3 colour = glm::vec3(0,0,0), bool twinTrap = true);

	/*
		Update multiple frames at once
		This method is needed because 40kHz update cannot be achieved by updating frame-by-frame
		This method assumes xyz (-128.0mm..+128.0mm), amplitude [0..1], phase usually [0..2PI) and rgb [0..255].
		The maximum numFrames capable is 250
	*/
	void updateMultipleFrames(int numFramesPerUpdate, glm::vec3 *positions, float *amplitudes, float *phases, glm::vec3 *colours, bool *twinTraps);
	
	//void updateAnimation(int numFramesPerUpdate, MATDAnimation &mAnimation);

	/*
		Move a twin trap from the previous position to the next one
	*/
	void goToNextPosition(glm::vec3 previousPosition, glm::vec3 nextPosition, int speed = 100000);


	/*
		This method turns the transducers and the LED off (so that the board does not heat up/die misserably)
		The board is still connected, so it can later be used again (e.g. create new traps)
	*/
	void turnOff();




};