#pragma once
#include "../MATDAnimation.h"

class Hemisphere : public MATDAnimation {
public:
	Hemisphere(MATDHandle& mHandle) : MATDAnimation(mHandle) { ; }

	void createAnimation() {
		DisplayContent dContent;

		int numLines = 64;
		float pixelSize = 0.001f;
		float sphereRadius = numLines * pixelSize / 2.0f;
		int numCircleMaximum = 13220;
		int numCircleMinimum = 8000;
		int numTurn = 500;
		cv::Mat img = cv::imread("media/EarthTexture2.jpg", 1);

		glm::vec3 currentPosition, previousPosition, nextPosition;
		for (int k = 0; k < 2; k++) {
			for (int j = 0; j < numLines / 2; j++) {
				if (k == 0)
					currentPosition.z = pixelSize * (j * 2.0 - (numLines - 1) / 2.0);
				else
					currentPosition.z = -pixelSize * (j * 2.0 - (numLines - 1) / 2.0);

				float circleRadius = sqrt(sphereRadius * sphereRadius - currentPosition.z * currentPosition.z);
				int numCircle = numCircleMinimum + (numCircleMaximum - numCircleMinimum) * circleRadius / sphereRadius;
				numCircle = round(numCircle / 200) * 200;
				for (int i = 0; i < numCircle; i++) {
					float radian = i * M_PI / (float)numCircle;
					if (j % 2 == 1) radian = M_PI - radian;
					currentPosition.x = -circleRadius * sin(radian);
					currentPosition.y = circleRadius * cos(radian);
					dContent.positions.push_back(currentPosition);


					float imgX = radian - M_PI / 2.0;
					float imgY = asin(currentPosition.z / sphereRadius);
					dContent.colours.push_back(getImageColor(img, imgX, imgY, M_PI, M_PI));
				}

				previousPosition = currentPosition;
				nextPosition.x = 0.0f;
				if (j == numLines / 2 - 1) {
					if (k == 0)
						nextPosition.z = pixelSize * (numLines - 1) / 2.0;
					else
						nextPosition.z = -pixelSize * (numLines - 1) / 2.0;
					nextPosition.y = sqrt(sphereRadius * sphereRadius - nextPosition.z * nextPosition.z);
				}
				else {
					if (k == 0)
						nextPosition.z = currentPosition.z + 2.0 * pixelSize;
					else
						nextPosition.z = currentPosition.z - 2.0 * pixelSize;
					if (j % 2 == 0)
						nextPosition.y = -sqrt(sphereRadius * sphereRadius - nextPosition.z * nextPosition.z);
					else
						nextPosition.y = sqrt(sphereRadius * sphereRadius - nextPosition.z * nextPosition.z);
				}

				for (int i = 0; i < numTurn; i++) {
					float radian = i * M_PI / (float)numTurn;
					currentPosition.x = 0.5 * (nextPosition.z - previousPosition.z) * sin(radian);
					currentPosition.y = 0.5 * ((previousPosition.y + nextPosition.y) - (nextPosition.y - previousPosition.y) * cos(radian));
					currentPosition.z = 0.5 * ((previousPosition.z + nextPosition.z) - (nextPosition.z - previousPosition.z) * cos(radian));
					dContent.positions.push_back(currentPosition);


					float imgX = radian - M_PI / 2.0;
					float imgY = asin(currentPosition.z / sphereRadius);
					dContent.colours.push_back(getImageColor(img, imgX, imgY, M_PI, M_PI));
				}
			}
		}

		std::cout << "Required exposure time is: ";
		std::cout << 25.0 * dContent.positions.size() * 0.000001 << " s" << std::endl;

		dContent.amplitudes.push_back(1.0f);
		dContent.phases.push_back(0.0f);
		dContent.twinFlags.push_back(true);

		dContents.push_back(dContent);
	};
};