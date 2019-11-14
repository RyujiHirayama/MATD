#pragma once
#include <iostream>
#include <thread>
#include <conio.h> 
#include <windows.h>

#define KEY_SPACE 32
#define KEY_ENTER 13
#define KEY_ESCAPE 27
#define KEY_TAB 9
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_RIGHT 77
#define KEY_LEFT 75

class StateControl
{
	enum CurrentState { INIT = 0, MOVING = 1, STOP = 2 };
	int state;
	bool changeFlag = false;
	DWORD updatePeriod;
	DWORD baseTime, currentTime;
	bool sendReady;
	DWORD timeCount, timeCountMax;

	bool success, inputFlag;

public:
	StateControl(DWORD updatePeriod) : state(INIT), updatePeriod(updatePeriod), sendReady(true), timeCount(0), timeCountMax(MAXDWORD), inputFlag(false) {
		baseTime = uGetTime();
		ThreadStart();
	}
	void ThreadStart() {
		std::thread th(&StateControl::KeyListener, this);
		th.detach();
	}
	void KeyListener() {
		int ch;
		std::cout << "Press key to control the state: " << std::endl;
		std::cout << "'Enter' to start/stop running amplitude modulation" << std::endl;
		std::cout << "'Space' to start/stop the correction of the bead position using OptiTrack" << std::endl;
		std::cout << "'Escape' to quit the loop and to end the program" << std::endl;
		while ((ch = _getch()) != EOF) {
			if (ch == KEY_ENTER) {
				if (state == INIT) {
					std::cout << "start moving!" << std::endl;
					state = MOVING;
				}
				else if (state == MOVING) {
					std::cout << "stop moving..." << std::endl;
					state = INIT;
				}
			}
			if (ch == KEY_ESCAPE) {
				std::cout << "exit from the loop..." << std::endl;
				state = STOP;
				break;
			}
			if (ch == KEY_TAB) {
				changeFlag = true;
			}
			if (ch == 't') {
				success = true;
				inputFlag = true;
			}
			if (ch == 'f') {
				success = false;
				inputFlag = true;
			}
		}
	}
	bool isWorking() {
		if (state != STOP) return true;
		else return false;
	}

	void countTimer() {
		if (timeCount == timeCountMax - 1) timeCount = 0;
		else timeCount++;
	}

	bool isReady() {
		if (state == MOVING) {
			if (sendReady) {
				sendReady = false;
				return true;
			}
			else {
				if ((currentTime = uGetTime(baseTime)) > (DWORD)(updatePeriod * (timeCount + 1))) {
					sendReady = true;
				}
				return false;
			}
		}
		else {
			baseTime = uGetTime();
			timeCount = 0;
			return false;
		}
	}

	bool getChangeFlag() {
		return changeFlag;
	}
	void disableChangeFlag() {
		changeFlag = false;
	}

	bool anyInput() {
		return inputFlag;
	}
	bool checkSuccess() {
		inputFlag = false;
		return success;
	}

	void uSleep(DWORD waitTime) {
		LARGE_INTEGER nFreq, nBefore, nAfter;
		DWORD dwTime;

		QueryPerformanceFrequency(&nFreq);
		QueryPerformanceCounter(&nBefore);

		do {
			QueryPerformanceCounter(&nAfter);
			dwTime = (DWORD)((nAfter.QuadPart - nBefore.QuadPart) * 1000000 / nFreq.QuadPart);
		} while (dwTime < waitTime);
	}

	DWORD uGetTime(DWORD baseTime = 0) {
		LARGE_INTEGER nFreq, currentTime;
		DWORD dwTime;

		QueryPerformanceFrequency(&nFreq);
		QueryPerformanceCounter(&currentTime);
		dwTime = (DWORD)(currentTime.QuadPart * 1000000 / nFreq.QuadPart);

		return dwTime - baseTime;
	}
};