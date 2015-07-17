#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <tchar.h>

using namespace std;

HWND buttonHandle = 0;
char * labelName;

//Holds the configuration settings (folder location, executable and .nto settings file location
struct settings {
	string ntoExe;
	string dir;
	string ntoFileLocation;
};

void loadSettings(settings &config);

//Used to find the handle
BOOL CALLBACK GetButtonHandle(HWND handle, LPARAM) {
	char label[100];
	int size = GetWindowTextA(handle, label, sizeof(label));
	if (strcmp(label, labelName) == 0) {
		buttonHandle = handle;
		return false;
	}
	return true;
}

int main() {
	settings cfg;
	loadSettings(cfg);

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	char * ntoExe = new char[cfg.ntoExe.size() + 1];
	strcpy_s(ntoExe, cfg.ntoExe.size() + 1, cfg.ntoExe.c_str());
	const char * cwd = cfg.dir.c_str();
	char * szNto = new char[cfg.ntoFileLocation.size() + 1];
	strcpy_s(szNto, cfg.ntoFileLocation.size() + 1, cfg.ntoFileLocation.c_str());

	//Creates process of NTO
	if (CreateProcess(0, ntoExe, 0, 0, FALSE, 0, 0, cwd, &si, &pi)) {
		// wait a sec for process to open
		WaitForSingleObject(pi.hProcess, 2500);
		HWND windowHandle = FindWindow(0, "NTO 6H V1.09b");
		HWND LFSHandle = FindWindow(0, "Live for Speed");
		labelName = "Load Settings";
		if (windowHandle != 0) {
			BOOL ret = EnumChildWindows(windowHandle, GetButtonHandle, 0);

			if (buttonHandle != 0)
				PostMessage(buttonHandle, BM_CLICK, 0, 0);
			else
				cout << "error: couldn't find buttonhandle";

			Sleep(1000);
			HWND handleOpen = 0;
			HWND editHwnd = 0;
			HWND comboHwnd = 0;
			HWND comboHwnd2 = 0;

			va_list args;
			TCHAR szText[4096];
			//Have to work its way to the edit field of NTO.
			handleOpen = FindWindow(0, "Open");
			if (handleOpen != 0) {
				comboHwnd = FindWindowEx(handleOpen, 0, "ComboBoxEx32", NULL);
				if (comboHwnd != 0) {
					comboHwnd2 = FindWindowEx(comboHwnd, 0, "ComboBox", NULL);
					if (comboHwnd2 != 0) {
						editHwnd = FindWindowEx(comboHwnd2, 0, "Edit", NULL);
						if (editHwnd != 0) {
							va_start(args, szNto);
							_vsntprintf_s(szText, sizeof(szText) / sizeof(szText[0]), szNto, args);
							szText[(sizeof(szText) / sizeof(szText[0])) - 1] = TEXT('\0');
							va_end(args);
							SendMessage(editHwnd, EM_REPLACESEL, FALSE, (LPARAM)szText);
							buttonHandle = 0;

							labelName = "&Open";
							ret = EnumChildWindows(handleOpen, GetButtonHandle, 0);
							if (buttonHandle != 0)
								PostMessage(buttonHandle, BM_CLICK, 0, 0);

							Sleep(500);

							buttonHandle = 0;
							labelName = "Connect";
							ret = EnumChildWindows(windowHandle, GetButtonHandle, 0);
							if (buttonHandle != 0) {
								Sleep(300);
								SendMessage(buttonHandle, BM_CLICK, 0, 0);
							}

							if (LFSHandle != 0) {
								SetForegroundWindow(LFSHandle);
								ShowWindow(LFSHandle, SW_RESTORE);
							}
						}
					}
				}
			}

		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
		cout << "error: " << GetLastError() << '\n';

	return 0;
}

void loadSettings(settings &config) {
	ifstream indata("config.ini");
	if (!indata) {
		cerr << "File could not be opened!" << endl;
		exit(1);
	}
	string line;
	while (!indata.eof()) {
		getline(indata, line);
		if (line[0] == '#') {
			continue;
		}
		else if (line.find("nto-exe") != string::npos) {
			line.replace(0, line.find("=") + 1, "");
			line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
			for (unsigned int i = 0; i < line.length(); i++) {
				if (line[i] == '\\') {
					config.dir += '\\';
				}
				config.ntoExe += line[i];
			}
		}
		else if (line.find("nto-settings") != string::npos) {
			line.replace(0, line.find("=") + 1, "");
			line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
			config.ntoFileLocation = line;
		}
		else
			continue;
	}
	if (config.ntoExe.length() <= 1 || config.ntoFileLocation.length() <= 1) {
		cerr << "Configuration Failed!" << endl;
		exit(1);
	}
	else {
		size_t splitLoc = config.ntoExe.find_last_of("/\\");
		config.dir = config.ntoExe.substr(0, splitLoc) + "/";
		cout << "Configuration successfully loaded!" << endl;
	}

	/*if (config.dir.length() <= 1 || config.ntoFileLocation.length() <= 1) {
	cerr << "Configuration Failed!" << endl;
	exit(1);
	}
	else {
	config.ntoExe = config.dir + "newtweakorder.exe";
	cout << "Config successfully loaded!" << endl;
	}*/
}