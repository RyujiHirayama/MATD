#pragma once
#include "../MATDAnimation.h"

class RasterImage : public MATDAnimation {
public:
	RasterImage(MATDHandle& mHandle) : MATDAnimation(mHandle) { ; }

	void createAnimation() {
		DisplayContent dContent;

		float straightLength = 0.032f;
		float pixelSize = 0.001f;
		int numStraight = 4000;
		int numCurve = 1000;
		int numCurveHalf = numCurve;
		int numHorizontal = 32;
		int numHorizontalHalf = numHorizontal / 2;

		//cv::Mat img = cv::imread("media/US_logo.png", 1);
		cv::Mat img = cv::imread("media/RubikCube.jpg", 1);
		for (int j = 0; j < numHorizontal; j++) {
			for (int i = 0; i < numStraight; i++) {
				float imgX = straightLength / 2.0 - i * straightLength / (float)numStraight;
				if (j % 2 == 1) imgX *= -1.0;
				float imgY = -pixelSize * ((numHorizontal - 1) / 2.0 - 2.0 * (j % numHorizontalHalf));
				if (j >= numHorizontal / 2) imgY *= -1.0;

				dContent.positions.push_back(glm::vec3(0.0f, imgX, imgY));
				dContent.colours.push_back(getImageColor(img, imgX, imgY, pixelSize * numHorizontal, straightLength));
			}
			if (j % numHorizontalHalf != numHorizontal / 2 - 1) {
				for (int i = 0; i < numCurve; i++) {
					float imgX = -straightLength / 2.0 - pixelSize * sinf(i * M_PI / (float)numCurve);
					if (j % 2 == 1) imgX *= -1.0;
					float imgY = -pixelSize * ((numHorizontal - 3) / 2.0 + cosf(i * M_PI / (float)numCurve) - 2.0 * (j % numHorizontalHalf));
					if (j >= numHorizontalHalf) imgY *= -1.0;

					dContent.positions.push_back(glm::vec3(0.0f, imgX, imgY));
					dContent.colours.push_back(getImageColor(img, imgX, imgY, pixelSize * numHorizontal, straightLength));
				}
			}
			else {
				for (int i = 0; i < numCurveHalf; i++) {
					float imgX = -straightLength / 2.0 - pixelSize * sinf(i * M_PI / (float)numCurveHalf);
					if (j % 2 == 1) imgX *= -1.0;
					float imgY = -pixelSize * ((numHorizontal - 3) / 2.0 + 0.5 * cosf(i * M_PI / (float)numCurveHalf) - 2.0 * (j % numHorizontalHalf) + 0.5);
					if (j >= numHorizontalHalf) imgY *= -1.0;

					dContent.positions.push_back(glm::vec3(0.0f, imgX, imgY));
					dContent.colours.push_back(getImageColor(img, imgX, imgY, pixelSize * numHorizontal, straightLength));
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