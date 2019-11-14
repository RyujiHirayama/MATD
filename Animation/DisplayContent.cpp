#pragma once
#include "DisplayContent.h"

void DisplayContent::fillBuffers(int numFramesPerUpdate, glm::vec3 *posBuffer, float *ampBuffer, float *phaseBuffer, glm::vec3 *colourBuffer, bool *twinBuffer) {
	for (int i = 0; i < numFramesPerUpdate; i++) {
		posBuffer[i] = positions[posAddress];
		ampBuffer[i] = amplitudes[ampAddress];
		phaseBuffer[i] = phases[phaseAddress];
		colourBuffer[i] = colours[colourAddress];
		twinBuffer[i] = twinFlags[twinAddress];

		if (posAddress == positions.size() - 1) {
			posAddress = 0; posFin = true;
			changeTiming = true;
		}
		else {
			posAddress++;
			changeTiming = false;
		}

		if (ampAddress == amplitudes.size() - 1) {
			ampAddress = 0; ampFin = true;
		}
		else ampAddress++;

		if (phaseAddress == phases.size() - 1) {
			phaseAddress = 0; phaseFin = true;
		}
		else phaseAddress++;

		if (colourAddress == colours.size() - 1) {
			colourAddress = 0; colourFin = true;
		}
		else colourAddress++;

		if (twinAddress == twinFlags.size() - 1) {
			twinAddress = 0; twinFin = true;
		}
		else twinAddress++;
	}
};

void DisplayContent::resetAddressAndFlag() {
	posAddress = 0;
	ampAddress = 0; phaseAddress = 0;
	colourAddress = 0; twinAddress = 0;
	changeTiming = false; posFin = false;
	ampFin = false; phaseFin = false;
	colourFin = false; twinFin = false;
};

bool DisplayContent::updateFinished() {
	return posFin && ampFin && phaseFin && colourFin && twinFin;
}


bool DisplayContent::isChangeTiming() {
	return changeTiming;
}

void DisplayContent::getMinMaxPositions() {
	minPosition = glm::vec3(std::numeric_limits<float>::max());
	maxPosition = glm::vec3(std::numeric_limits<float>::min());

	for (int s = 0; s < positions.size(); s++) {
		minPosition.x = std::min(minPosition.x, positions[s].x);
		minPosition.y = std::min(minPosition.y, positions[s].y);
		minPosition.z = std::min(minPosition.z, positions[s].z);

		maxPosition.x = std::max(maxPosition.x, positions[s].x);
		maxPosition.y = std::max(maxPosition.y, positions[s].y);
		maxPosition.z = std::max(maxPosition.z, positions[s].z);
	}
}

void DisplayContent::normalisePositions(float drawingSize) {
	getMinMaxPositions();

	float scale = drawingSize / std::max(maxPosition.x - minPosition.x, std::max(maxPosition.y - minPosition.y, maxPosition.z - minPosition.z));
	glm::vec3 offset = -0.5f * scale * (maxPosition + minPosition);

	for (int i = 0; i < positions.size(); i++) {
		positions[i] = scale * positions[i] + offset;
	}
}

void DisplayContent::shiftPositions(glm::vec3 shift) {
	for (int i = 0; i < positions.size(); i++) {
		positions[i] = positions[i] + shift;
	}
}

void DisplayContent::rotatePositions(glm::mat3 rotate) {
	for (int i = 0; i < positions.size(); i++) {
		positions[i] = rotate * positions[i];
	}
}

void DisplayContent::interporatePositions(float constantVelocity) {

	std::vector<glm::vec3> tmpPositions, tmpColours;
	glm::vec3 previousPosition, nextPosition, nextColour;
	int cnt = 0;
	for (int j = 0; j < positions.size(); j++) {
		if (j == positions.size() - 1) {
			tmpPositions.push_back(positions[j]);
			tmpColours.push_back(colours[j]);
		}
		else {
			previousPosition = positions[j];
			nextPosition = positions[j + 1];
			nextColour = colours[j + 1];

			float distance = length(nextPosition - previousPosition);
			int numInterporation = round(1000000.0f * distance / (25.f * constantVelocity));

			//std::cout << cnt++ << ", " << distance << ", " << numInterporation << std::endl;

			for (int i = 0; i < numInterporation; i++) {
				float path = numInterporation > 0 ? i / (float)numInterporation : 1.f;
				glm::vec3 currentPosition = previousPosition + (nextPosition - previousPosition) * path;
				tmpPositions.push_back(currentPosition);
				tmpColours.push_back(nextColour);
			}
		}
	}

	positions = tmpPositions;
	colours = tmpColours;
}