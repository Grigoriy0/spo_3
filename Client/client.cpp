#include<iostream>
#include<Windows.h>
#include<string>

void errorExit(const char *str) {
	fprintf(stderr, "[Client] %30s|GLE=%d.\n", str, GetLastError());
	system("pause");
	exit(EXIT_FAILURE);
}

HANDLE OpenPipe(const std::string &pipeName) {
	HANDLE hPipe;
	while (true) {
		hPipe = CreateFile(
			pipeName.c_str(),   // pipe name 
			GENERIC_READ,  // read and write access 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

		if (hPipe != INVALID_HANDLE_VALUE)
			break;
		// Exit if an error other than ERROR_PIPE_BUSY occurs

		if (GetLastError() != ERROR_PIPE_BUSY) {
			errorExit("CreateFile failed");
		}

		if (!WaitNamedPipe(pipeName.c_str(), 20000)) {
			errorExit("WaitNamedPipe: 20 sec timed out");
		}
	}
	return hPipe;
}


std::string ReadFromServer(HANDLE hPipe) {
	static const int bufSize = 256;
	char buf[bufSize] = {};
	if (!ReadFile(hPipe, buf, bufSize, nullptr, nullptr)) {
		return "";
	}
	
	return buf;
}


HANDLE OpenServerEvent() {
	HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, false, "PipeServerStarted");
	if (!hEvent) {
		errorExit("OpenEvent failed");
	}
	return hEvent;
}


void NotifyServer(HANDLE hEvent) {
	if (!SetEvent(hEvent)) {
		errorExit("SetEvent failed");
	}
}


void WaitServerEvent(HANDLE hEvent) {
	DWORD result = WaitForSingleObject(hEvent, INFINITE);
	if (result == WAIT_FAILED) {
		errorExit("WaitForSingleObject failed");
	}
}

int main(int argc, char *argv[]) {
	// wait for signal from server
	// start loop
	//		read string
	//		print string
	//		notify server
	// repeat loop if string != "\quit"
	HANDLE hPipe;
	HANDLE hEvent;
	
	DWORD  cbRead, cbToWrite, cbWritten;
	const char* pipeName = "\\\\.\\pipe\\MyPipe";
	
	hEvent = OpenServerEvent();
	WaitServerEvent(hEvent);

	hPipe = OpenPipe(pipeName);

	std::string message;
	do
	{
		message = ReadFromServer(hPipe);
		std::cout << message << std::endl;
		NotifyServer(hEvent);

	} while (message != "quit" && message != "exit");
	
	CloseHandle(hPipe);
	CloseHandle(hEvent);
	return 0;
}