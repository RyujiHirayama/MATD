#include "MATDHandle.h"
#include "MATDConfig.h"

bool MATDHandle::connect(int portBottom, int portTop) {
	if (status == MATDState::INIT) {
		//Connect to the boards (baudrate for this board is fixed =12000000).
		ctBottom.connect(portBottom, Baudrate);
		ctTop.connect(portTop, Baudrate);
		//Check for errors
		if (ctBottom.anyErrors() || ctTop.anyErrors()) {
			return false;
		}
		else {
			status = MATDState::CONNECTED;
			return true;
		}
	}
	return false;
}

void MATDHandle::disconnect() {
	if (status == MATDState::INIT)
		return;

	ctBottom.closeConnection();
	ctTop.closeConnection();
	status = MATDState::INIT;
	std::cout << "Closed the connections" << std::endl;
}

unsigned char MATDHandle::_discretizePhase(float phase) {
	static const float TWO_PI = 2 * ((float)M_PI);
	//Transform phase to [0..2PI] range
	float mod_phase = fmodf(phase, TWO_PI);
	if (mod_phase < 0) mod_phase += TWO_PI;
	// ... and then to a [0..1) range
	float normalized_Phase = mod_phase / TWO_PI;
	//Map normal range [0..1) to our discrete range [0..maxPhase)
	int discretePhase = (int)(normalized_Phase * DiscretePhaseMax);
	//Done
	return (unsigned char)(discretePhase);
}

unsigned char MATDHandle::_discretizeAmplitude(float amplitude) {
	//Map normal range [0..1) to our discrete range [0..maxAmplitude]
	int discreteAmplitude = (int)roundf(amplitude * DiscreteAmplitudeMax);
	//Done
	return (unsigned char)(discreteAmplitude);
}

void MATDHandle::sendCalibData() {
	if (status == MATDState::INIT)
		return;

	unsigned char message_bottom[257];
	unsigned char message_top[257];
	// The messages need to have header (252) to tell the FPGAs that sending the calibration data
	message_bottom[0] = CalibCommand;
	message_top[0] = CalibCommand;

	// Every 256 transducer need different calibration
	for (int index = 0; index < 256; index++) {
		// 1. phase correction
		float calib_bottom = (float)(phaseAdjustBottom[index] * M_PI / 180.0f);
		float calib_top = (float)(phaseAdjustTop[index] * M_PI / 180.0f);
		// 2. discretize the corrected phases
		unsigned char discreteCalib_bottom = _discretizePhase(calib_bottom);
		unsigned char discreteCalib_top = _discretizePhase(calib_top);
		// 3. store in the buffer
		message_bottom[index + 1] = discreteCalib_bottom;
		message_top[index + 1] = discreteCalib_top;
	}
	ctBottom.sendString(message_bottom, 257);
	ctTop.sendString(message_top, 257);
}

/*
void MATDHandle::_encodeFrame(float x, float y, float z, float amplitude, float phase, unsigned char red, unsigned char green, unsigned char blue, unsigned char *message) {
	// round the positoin values to make them at 0.25 mm resolution
	int int_x = (int)round(x * 4.0 * 1000.0);
	int_x = (int_x < 0 ? 1024 + int_x : int_x);		// x is range from -128.0 to 128.0mm
	int int_y = (int)round(y * 4.0 * 1000.0);
	int_y = (int_y < 0 ? 1024 + int_y : int_y);		// y is range from -128.0 to 128.0mm
	int int_z = (int)round(z * 4.0 * 1000.0);		// z is range from 0.0 to 256.0mm

	// encode each parameters of the frame
	unsigned char x_7_0 = (unsigned char)(int_x % 256);
	unsigned char x_9_8 = (unsigned char)(int_x / 256);
	unsigned char y_5_0 = (unsigned char)(int_y % 64);
	unsigned char y_9_6 = (unsigned char)(int_y / 64);
	unsigned char z_3_0 = (unsigned char)(int_z % 16);
	unsigned char z_9_4 = (unsigned char)(int_z / 16);
	unsigned char descreteAmplitude = _discretizeAmplitude(amplitude);
	unsigned char descretePhase = _discretizePhase(phase);

	message[0] = x_7_0;					// from 0th to 7th bits of the x coordinate
	message[1] = x_9_8 + y_5_0 * 4;		// from 8th and 9th bits of the x and 0th to 5th bits of the y coordinates
	message[2] = y_9_6 + z_3_0 * 16;	// from 6th and 9th bits of the y and 0th to 3th bits of the z coordinates
	message[3] = z_9_4;					// from 4th and 9th bits of the z coordinate
	message[4] = descreteAmplitude;	
	message[5] = descretePhase;	
	message[6] = red;
	message[7] = green;
	message[8] = blue;
}

void MATDHandle::updateFrame(glm::vec3 position, float amplitude, float phase, glm::vec3 colour, bool twinFlag){
	if (status == MATDState::INIT)
		return;

	static unsigned char message_bottom[10], message_top[10];
	static float zCenter = BoardHeight / 2.0f;

	// number of frames is 1
	message_bottom[0] = 1;
	message_top[0] = 1;
	float signature = twinFlag ? M_PI : 0.0f;	// signature for the top board is PI when create a twin trap, 0 when create a focus point
	_encodeFrame(position.y, position.x, position.z + zCenter, amplitude, phase, colour.r, colour.g, colour.b, &message_bottom[1]);	// swap x and y to match the Optitrack coordinate
	_encodeFrame(position.y, -position.x, -position.z + zCenter, amplitude, phase + signature, colour.r, colour.g, colour.b, &message_top[1]);	// top board is rotated around x-axis

	// send the messeges
	ctBottom.sendString(message_bottom, 10);
	ctTop.sendString(message_top, 10);
	ctBottom.sendByte(SwapCommand);
	ctTop.sendByte(SwapCommand);
}

void MATDHandle::updateMultipleFrames(int numFramesPerUpdate, glm::vec3 *positions, float *amplitudes, float *phases, glm::vec3 *colours, bool *twinFlags) {
	if (status == MATDState::INIT)
		return;

	static unsigned char message_bottom[2251], message_top[2251];
	static float zCenter = BoardHeight / 2.0f;

	// number of frames needs to be sent first
	message_bottom[0] = (unsigned char)numFramesPerUpdate;
	message_top[0] = (unsigned char)numFramesPerUpdate;
	for (int i = 0; i < numFramesPerUpdate; i++) {
		float signature = twinFlags[i] ? M_PI : 0.0f;	// signature for the top board is PI when create a twin trap, 0 when create a focus point
		_encodeFrame(positions[i].y, positions[i].x, positions[i].z + zCenter, amplitudes[i], phases[i], colours[i].r, colours[i].g, colours[i].b, &message_bottom[1 + i * 9]);	// swap x and y to match the Optitrack coordinate
		_encodeFrame(positions[i].y, -positions[i].x, -positions[i].z + zCenter, amplitudes[i], phases[i] + signature, colours[i].r, colours[i].g, colours[i].b, &message_top[1 + i * 9]);	// top board is rotated around x-axis
	}

	// send the messeges
	ctBottom.sendString(message_bottom, 9 * numFramesPerUpdate + 1);
	ctTop.sendString(message_top, 9 * numFramesPerUpdate + 1);
	ctBottom.sendByte(SwapCommand);
	ctTop.sendByte(SwapCommand);
}
*/
void MATDHandle::_encodeFrame(float x, float y, float z, float amplitude, float phase, unsigned char red, unsigned char green, unsigned char blue, unsigned char *message) {
	// round the positoin values to make them at 0.25 mm resolution
	int int_x = (int)round(x * 32.0 * 1000.0);
	int_x = (int_x < 0 ? 8192 + int_x : int_x);		// x is range from -128.0 to 128.0mm
	int int_y = (int)round(y * 32.0 * 1000.0);
	int_y = (int_y < 0 ? 8192 + int_y : int_y);		// y is range from -128.0 to 128.0mm
	int int_z = (int)round(z * 32.0 * 1000.0);		// z is range from 0.0 to 256.0mm

	// encode each parameters of the frame
	unsigned char x_7_0 = (unsigned char)(int_x % 256);
	unsigned char x_12_8 = (unsigned char)(int_x / 256);
	unsigned char y_2_0 = (unsigned char)(int_y % 8);
	unsigned char y_10_3 = (unsigned char)((int)(int_y / 8) % 256);
	unsigned char y_12_11 = (unsigned char)(int_y / 2048);
	unsigned char z_5_0 = (unsigned char)(int_z % 64);
	unsigned char z_12_6 = (unsigned char)(int_z / 64);


	unsigned char descreteAmplitude = _discretizeAmplitude(amplitude);
	unsigned char descretePhase = _discretizePhase(phase);

	message[0] = x_7_0;					// from 0th to 7th bits of the x coordinate
	message[1] = x_12_8 + y_2_0 * 32;	// from 8th and 12th bits of the x and 0th to 2th bits of the y coordinates
	message[2] = y_10_3;				// from 3th and 10th bits of the y
	message[3] = y_12_11 + z_5_0 * 4;	// from 11th and 12th bits of the y and 0th to 5th bits of the z coordinates
	message[4] = z_12_6;				// from 6th and 12th bits of the z coordinate
	message[5] = descreteAmplitude;
	message[6] = descretePhase;
	message[7] = red;
	message[8] = green;
	message[9] = blue;
}

void MATDHandle::updateFrame(glm::vec3 position, float amplitude, float phase, glm::vec3 colour, bool twinFlag) {
	if (status == MATDState::INIT)
		return;

	static unsigned char message_bottom[11], message_top[11];
	static float zCenter = BoardHeight / 2.0f;

	// number of frames is 1
	message_bottom[0] = 1;
	message_top[0] = 1;
	float signature = twinFlag ? M_PI : 0.0f;	// signature for the top board is PI when create a twin trap, 0 when create a focus point
	_encodeFrame(position.y, position.x, position.z + zCenter, amplitude, phase, colour.r, colour.g, colour.b, &message_bottom[1]);	// swap x and y to match the Optitrack coordinate
	_encodeFrame(position.y, -position.x, -position.z + zCenter, amplitude, phase + signature, colour.r, colour.g, colour.b, &message_top[1]);	// top board is rotated around x-axis

	// send the messeges
	ctBottom.sendString(message_bottom, 11);
	ctTop.sendString(message_top, 11);
	ctBottom.sendByte(SwapCommand);
	ctTop.sendByte(SwapCommand);
}

void MATDHandle::updateMultipleFrames(int numFramesPerUpdate, glm::vec3 *positions, float *amplitudes, float *phases, glm::vec3 *colours, bool *twinFlags) {
	if (status == MATDState::INIT)
		return;

	static unsigned char message_bottom[2501], message_top[2501];
	static unsigned char message_swap[1] = { SwapCommand };
	static float zCenter = BoardHeight / 2.0f;

	// number of frames needs to be sent first
	message_bottom[0] = (unsigned char)numFramesPerUpdate;
	message_top[0] = (unsigned char)numFramesPerUpdate;
	for (int i = 0; i < numFramesPerUpdate; i++) {
		float signature = twinFlags[i] ? M_PI : 0.0f;	// signature for the top board is PI when create a twin trap, 0 when create a focus point
		_encodeFrame(positions[i].y, positions[i].x, positions[i].z + zCenter, amplitudes[i], phases[i], colours[i].r, colours[i].g, colours[i].b, &message_bottom[1 + i * 10]);	// swap x and y to match the Optitrack coordinate
		_encodeFrame(positions[i].y, -positions[i].x, -positions[i].z + zCenter, amplitudes[i], phases[i] + signature, colours[i].r, colours[i].g, colours[i].b, &message_top[1 + i * 10]);	// top board is rotated around x-axis
	}

	// send the messeges
	ctBottom.sendString(message_bottom, 10 * numFramesPerUpdate + 1);
	ctTop.sendString(message_top, 10 * numFramesPerUpdate + 1);
	ctBottom.sendByte(SwapCommand);
	ctTop.sendByte(SwapCommand);
}


void MATDHandle::goToNextPosition(glm::vec3 previousPosition, glm::vec3 nextPosition, int speed) {
	// amplitude, phase and colour is fixed to 1, 0 and off, respectively
	float amplitude = 1.0f, phase = 0.0f;
	glm::vec3 black(0, 0, 0);
	int numInitSamples = speed * glm::length(nextPosition - previousPosition);	// number of updates to move the trap depends on the distance and is defined to keep the same speed
	if (numInitSamples != 0) {
		for (int i = 0; i < numInitSamples + 1; i++) {
			float path = i / (float)numInitSamples;
			glm::vec3 currentPosition = previousPosition + (nextPosition - previousPosition) * path;	// the position of the trap moves toward the next position linearly
			updateFrame(currentPosition, 1, 0, black);
		}
	}
}

void MATDHandle::turnOff() {
	glm::vec3 position(0.0f, 0.0f, 0.0f);
	float amplitude = 0.0f, phase = 0.0f;
	glm::vec3 colour(0, 0, 0);
	updateFrame(position, amplitude, phase, colour);
}

