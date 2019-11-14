#pragma once

#include <windows.h>	//Sorry.. it is windows dependant...

/**
	This class hides all the details related to the COM connection. 
*/
class COMToolkit2{
	HANDLE hSerial;
	DCB dcbSerialParams; 
	COMMTIMEOUTS timeouts;
	int baudRate;
	enum COM_States{INIT=0, READY=1, ERRORS=2};
	int status;

public:
	COMToolkit2():status(COM_States::INIT) {}
	void connect(int portNumber, int baudRate=9600);
	bool anyErrors();
	void closeConnection();
	//For sending data
	bool checkCTS();
	void sendByte(unsigned char b);
	void sendString(unsigned char* buffer, int numBytes);
	//For receiving data
	int available();
	int readByte();
	void readString(unsigned char *buffer, int numBytes);

	/**
		ACTIVE WAITING up to micro-second accuracy.
		Only use for seriously time-critical communication!!
		For longer waits, simply use Sleep(miliseconds);
	*/
	void uSleep(LONGLONG microSeconds);

private: 
	//Auxiliary methods to support the deffinition of the methods COMToolkit declares.
	void createFile(int portNumber);
	void setCommMask();
	void defineControlSettings();
	void setCommState();
	void setTimeouts();
	void setNoTimeouts();
};
