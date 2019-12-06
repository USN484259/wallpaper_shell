#include "wallpaper.h"
#include <exception>
#include <string>

using namespace std;


const char* mutex_name = "io.github.USN484259@wallpaper_shell";



int WinMain(
	HINSTANCE hInstance,
	HINSTANCE,
	LPSTR     lpCmdLine,
	int       nShowCmd
) {
	HANDLE hMutex = NULL;
	try {
		hMutex = CreateMutex(NULL, TRUE, mutex_name);
		if (!hMutex)
			throw runtime_error("CreateMutex failed");
		if (ERROR_ALREADY_EXISTS == GetLastError())
			throw runtime_error("Instance already exists");
		Wallpaper wp(string((const char*)lpCmdLine));
	}
	catch (exception& e) {
		MessageBox(NULL, e.what(), NULL, MB_OK);
	}
	CloseHandle(hMutex);
	return 0;
}