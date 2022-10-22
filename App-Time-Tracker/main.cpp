#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <psapi.h>
#include <string>
#include <cmath>
#include <thread>
#include <chrono>
#include <fstream>

std::fstream testfile; // test

std::string timeToString(long long seconds) {
	int h = seconds / 3600;
	int m = (seconds - h * 3600) / 60;
	int s = seconds - h * 3600 - m * 60;
	if (h == 0) {
		return std::string("000000:") + (m < 10 ? "0" : "")
			+ std::to_string(m) + ":" + (s < 10 ? "0" : "")
			+ std::to_string(s);
	}
	return std::string(5 - (int)(log(h) / log(10)), '0')
		+ std::to_string(h) + ":" + (m < 10 ? "0" : "")
		+ std::to_string(m) + ":" + (s < 10 ? "0" : "")
		+ std::to_string(s);
}

long long timeToLLong(std::string sTime) {
	std::string h = sTime.substr(0, sTime.find(":"));
	sTime.erase(0, sTime.find(":") + 1);
	std::string m = sTime.substr(0, sTime.find(":"));
	sTime.erase(0, sTime.find(":") + 1);
	return std::stoll(h, nullptr) * 3600 + std::stoll(m, nullptr) * 60 + std::stoll(sTime, nullptr);
}

void addToFile(std::fstream& fileOverwrite, std::ofstream& fileAppend, char currentTitle[]) {
	std::string title, time, newLine;
	std::string previousTitle = "", previousTime = "999999:99:99";
	bool match = false;

	// read through file and check if current active name already has an entry
	while (getline(fileOverwrite, title)) {
		getline(fileOverwrite, time);

		// if yes, update time
		if (title == std::string(currentTitle)) {
			fileOverwrite.seekp((int)fileOverwrite.tellp() - 13); // move cursor to start of time line
			time = timeToString(timeToLLong(time) + 1);
			fileOverwrite << time;
			match = true;
		}

		// sort: if this entry has a greater time value than the previous one, swap them
		if (timeToLLong(time) > timeToLLong(previousTime)) {
			fileOverwrite.seekp((int)fileOverwrite.tellp() - 13 - 150 - 13 - 150 - 2); // move cursor to start of previous title line; 150 instead of the line length 128 since GetModuleFileNameExA() adds a 22-character sequence at the end
			fileOverwrite << title << '\n'; // move cursor to start of previous title line; 150 instead of the line length 128 since GetModuleFileNameExA() adds a 22-character sequence at the end
			//fileOverwrite.seekp((int)fileOverwrite.tellp() + 1); // move cursor to start of previous time line; these numbers were worked out through trial & error after an annoying bug involving them happened
			fileOverwrite << time << "\n\n"; // move cursor to start of previous time line
			//fileOverwrite.seekp((int)fileOverwrite.tellp() + 2); // move cursor to start of current title line
			fileOverwrite << previousTitle << '\n'; // move cursor to start of current title line
			//fileOverwrite.seekp((int)fileOverwrite.tellp() + 1); // move cursor to start of current time line
			fileOverwrite << previousTime << "\n\n"; // move cursor to start of current time line
		} else {
			previousTitle = title;
			previousTime = time;
			fileOverwrite << "\n";
			//previousTime.substr(previousTime.length() - 1);
			//testfile << "TEst";
		}

		if (match) {
			return;
		}

		//getline(fileOverwrite, newLine); // move cursor past extra newline at the end of each entry
		//fileOverwrite.seekp((int)fileOverwrite.tellp() + 2); // doesn't work
	}

	// else add entry if it doesn't exist
	fileAppend << currentTitle << "\n" << "000000:00:01" << "\n\n";
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow) {
	std::ofstream fileAppendD_exe, fileAppendD_title, fileAppendA_exe, fileAppendA_title;
	std::fstream fileOverwriteD_exe, fileOverwriteD_title, fileOverwriteA_exe, fileOverwriteA_title;
	testfile.open("test.txt", std::ios::app); // test

	const std::string ENV = std::string(std::getenv("USERPROFILE"));
	char currentTitle[128];
	DWORD identifier;
	int len;

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		fileAppendD_exe.open(ENV + "\\Documents\\daily_exe.txt", std::ios::app | std::ios::binary); // opening in binary mode avoids automatic '\r' insertions before '\n' which caused a LOT of pain
		fileOverwriteD_exe.open(ENV + "\\Documents\\daily_exe.txt", std::ios::in | std::ios::out | std::ios::binary);
		fileAppendD_title.open(ENV + "\\Documents\\daily_title.txt", std::ios::app | std::ios::binary);
		fileOverwriteD_title.open(ENV + "\\Documents\\daily_title.txt", std::ios::in | std::ios::out | std::ios::binary);
		fileAppendA_exe.open(ENV + "\\Documents\\aggregate_exe.txt", std::ios::app | std::ios::binary);
		fileOverwriteA_exe.open(ENV + "\\Documents\\aggregate_exe.txt", std::ios::in | std::ios::out | std::ios::binary);
		fileAppendA_title.open(ENV + "\\Documents\\aggregate_title.txt", std::ios::app | std::ios::binary);
		fileOverwriteA_title.open(ENV + "\\Documents\\aggregate_title.txt", std::ios::in | std::ios::out | std::ios::binary);
		
		// get window exe name
		GetWindowThreadProcessId(GetForegroundWindow(), &identifier);
		len = GetModuleFileNameExA(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, identifier), NULL, currentTitle, 128);
		if (len == 0 || std::strstr(currentTitle, "LockApp")) {
			continue;
		}
		for (int i = len; i < sizeof(currentTitle) + 22; i++) { // apparently currentTitle is given an additional 22-character sequence at the end
			currentTitle[i] = ' ';
		}
		addToFile(fileOverwriteA_exe, fileAppendA_exe, currentTitle);

		// get window title name
		len = GetWindowTextA(GetForegroundWindow(), currentTitle, 128);
		for (int i = len; i < sizeof(currentTitle) + 22; i++) {
			currentTitle[i] = ' ';
		}
		addToFile(fileOverwriteA_title, fileAppendA_title, currentTitle);

		fileAppendD_exe.close();
		fileOverwriteD_exe.close();
		fileAppendD_title.close();
		fileOverwriteD_title.close();
		fileAppendA_exe.close();
		fileOverwriteA_exe.close();
		fileAppendA_title.close();
		fileOverwriteA_title.close();
		testfile.close();
	}

	// todo: use functions to implement daily
	// todo: remove lockapp.exe? (if string contains LockApp)
	// todo: test accuracy of not using system time
	// todo: get exe to run in system tray
}
