#include<iostream>
#include<Windows.h>
#include<string>


void errorExit(const char *str) {
	fprintf(stderr, "[Server] %30s|GLE=%d.\n", str, GetLastError());
	system("pause");
	exit(EXIT_FAILURE);
}

HANDLE CreatePipe() {
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO startInfo;
	const char * pipeName = "\\\\.\\pipe\\MyPipe";
	HANDLE hPipe;
	memset(&startInfo, 0, sizeof(startInfo));
	startInfo.cb = sizeof(startInfo);
	memset(&piProcInfo, 0, sizeof(piProcInfo));

	hPipe = CreateNamedPipe(
		pipeName,
		PIPE_ACCESS_OUTBOUND,
		PIPE_TYPE_MESSAGE |
		PIPE_READMODE_MESSAGE |
		PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		4096,
		4096,
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		errorExit("CreateNamedPipe failed");
	}
	return hPipe;
}

void WriteToPipe(HANDLE handle, const char* message, int len) {
	if (!WriteFile(handle, message, len, nullptr, nullptr)) {
		errorExit("WriteFile failed");
	}
}

HANDLE CreateServerEvent() {
	HANDLE event = CreateEvent(nullptr, true, false, "PipeServerStarted");
	if (event == nullptr) {
		errorExit("CreateEvent failed");
	}
	return event;
}


void NotifyClients(HANDLE hEvent) {
	if (!SetEvent(hEvent)) {
		errorExit("SetEvent failed");
	}
}


void WaitClient(HANDLE hEvent) {
	DWORD result = WaitForSingleObject(hEvent, INFINITE);
	if (result == WAIT_FAILED) {
		errorExit("WaitForSingleObject failed");
	}
}


void ConnectToPipe(HANDLE hPipe) {
	if (!ConnectNamedPipe(hPipe, nullptr)) {
		errorExit("ConnectNamedPipe failed");
	}
}


int main() {
	// notify client 
	// start loop
	//		input string
	//		send string to client
	//		wait for signal from client
	// repeat loop if string != "\quit"
	
	const char * clientName = "client.exe";
	char buf[256];
	DWORD iNumBytesToRead = 256, i;
	HANDLE hPipe = CreatePipe();
	HANDLE hEvent = CreateServerEvent();
	std::cout << "Press enter to start PipeServer";
	system("pause");
	system("cls");
	NotifyClients(hEvent);
	ConnectToPipe(hPipe);

	const int bufSize = 256;
	char message[bufSize] = "";
	do {
		std::cout << "\nInput a string you want to send to client(s)";
		std::cin.getline(message, bufSize);
		uint8_t len = strlen(message);
		WriteToPipe(hPipe, message, len);
		WaitClient(hEvent);
	} while (strcmp(message, "quit") && strcmp(message, "exit"));

	CloseHandle(hPipe);
	CloseHandle(hEvent);
	return 0;
}