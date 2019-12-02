#include "ui.h"
#include "repository.hpp"
#include "wallpaper.h"
#include <exception>

using namespace std;
using Path = USNLIB::filesystem::path;

struct context_holder {
	shell_ui& ui;
	Repository& repo;
	Wallpaper& wp;
};



void CALLBACK OnTimer(LPVOID arg, DWORD, DWORD) {
	context_holder* context = (context_holder*)arg;
	if (!context->wp.changeable())
		return;
	const Path& path = context->repo.get();
	context->ui.title(path.filename());
	//MessageBox(NULL, ((const string&)path).c_str(), NULL, MB_OK);
	context->wp.set(path);

}




int WinMain(
	HINSTANCE hInstance,
	HINSTANCE,
	LPSTR     lpCmdLine,
	int       nShowCmd
) {
	HANDLE timer = NULL;
	try {
		shell_ui ui;
		Repository repo;
		Wallpaper wp;
		repo.put("E:\\LifeRoutine\\BURST\\");

		LONG interval = 1000 * 30;
		timer = CreateWaitableTimer(NULL, TRUE, NULL);
		if (!timer)
			throw runtime_error("CreateWaitableTimer");
		LARGE_INTEGER due;
		due.QuadPart = -100000000LL;
		context_holder context = { ui, repo , wp };
		SetWaitableTimer(timer, &due, interval, OnTimer, &context, FALSE);


		ui.show();
	}
	catch (exception& e) {
		e;
	}
	if (timer) {
		CancelWaitableTimer(timer);
		CloseHandle(timer);
	}

	return 0;
}