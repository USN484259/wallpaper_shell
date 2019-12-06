#include "wallpaper.h"
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include <functional>


#include <Windows.h>
#include <wininet.h>
#include <shlobj.h>

using namespace std;
using Path = USNLIB::filesystem::path;



Wallpaper::Wallpaper(const string& cmd) : timer(CreateWaitableTimer(NULL, TRUE, NULL)), interval(30*1000),paused(false),power_policy(true), maximize_policy(true), explorer_policy(true) {
	CoInitialize(NULL);
	ui.menu_handler(bind(&Wallpaper::menu_event, this, placeholders::_1));
	ui.menu_item(4, "Pause on battery power");
	ui.menu_item(5, "Pause on maximaized");
	ui.menu_item(6, "Pause on folder opened");
	ui.menu_item();
	ui.menu_item(3, "Pause manually");
	ui.menu_item();
	ui.menu_item(2, "Previous wallpaper");
	ui.menu_item(1, "Next wallpaper");
	ui.menu_item();

	parse(cmd);

	ui.menu_checked(3, paused);
	ui.menu_checked(4, power_policy);
	ui.menu_checked(5, maximize_policy);
	ui.menu_checked(6, explorer_policy);

	if (!paused) {
		LARGE_INTEGER due;
		due.QuadPart = -100000000LL;
		if (!SetWaitableTimer(timer, &due, interval, on_timer, this, FALSE))
			throw runtime_error("Wallpaper::SetWaitableTimer");
	}

	ui.show();

}

Wallpaper::~Wallpaper(void) {
	CancelWaitableTimer(timer);
	CoUninitialize();
	CloseHandle(timer);
}

void CALLBACK Wallpaper::on_timer(LPVOID arg, DWORD, DWORD) {
	Wallpaper* wp = (Wallpaper*)arg;
	wp->next();

}

void Wallpaper::parse(const string& cmd) {
	stringstream ss;
	ss.str(cmd);
	string str;
	while (ss.good()) {

		if (!str.empty() && str.front() == '\"') {
			string tmp;
			ss >> tmp;
			str += ' ';
			str += tmp;
			if (ss.good() && tmp.front() != '/' && str.back() != '\"')
				continue;
		}
		else
			ss >> str;

		if (str.empty())
			continue;

		if (str.front() == '/') {
			switch (str.at(1)) {
			case 'p':
				power_policy = false;
				break;
			case 'P':
				power_policy = true;
				break;
			case 'e':
				explorer_policy = false;
				break;
			case 'E':
				explorer_policy = true;
				break;
			case 'm':
				maximize_policy = false;
				break;
			case 'M':
				maximize_policy = true;
				break;
			case 'i':
			case 'I':
				interval = 1000 * strtoul(str.substr(2).c_str(), nullptr, 10);
				if (interval < 5000)
					interval = 30 * 1000;
				break;
			}
		}
		else
			repo.put(str);

		str.clear();

	}
	if (0 == repo.size())
		throw runtime_error("No image selected");

}


void Wallpaper::menu_event(unsigned id) {
	switch (id) {
	case 1:	//Next
		next(true);
		break;
	case 2:	//prev
		prev();
		break;
	case 3:	//Pause
	{
		if (paused) {
			LARGE_INTEGER due;
			due.QuadPart = -100000000LL;
			if (SetWaitableTimer(timer, &due, interval, on_timer, this, FALSE))
				paused = false;
		}
		else {
			if (CancelWaitableTimer(timer))
				paused = true;
		}
		ui.menu_checked(3, paused);
		break;
	}

	case 4:	//power
		ui.menu_checked(4,power_policy = !power_policy);
		break;
	case 5:	//maximize
		ui.menu_checked(5,maximize_policy = !maximize_policy);
		break;
	case 6:	//explorer
		ui.menu_checked(6,explorer_policy = !explorer_policy);
		break;

	}
}

bool Wallpaper::changeable(void) const {

	if (power_policy) {
		SYSTEM_POWER_STATUS ps;
		if (!GetSystemPowerStatus(&ps))
			return false;
		if (ps.ACLineStatus == 0)
			return false;
	}

	if (maximize_policy) {
		HWND foreground = GetForegroundWindow();

		while (foreground) {
			WINDOWPLACEMENT placement;
			if (GetWindowPlacement(foreground, &placement) && placement.showCmd == SW_MAXIMIZE)
				return false;
			foreground = GetWindow(foreground, GW_OWNER);
		}

	}

	if (explorer_policy) {
		if (FindWindow(TEXT("CabinetWClass"), NULL))
			return false;
	}


	return true;
}


bool Wallpaper::set(const string& str) {
	bool suc = false;
	IActiveDesktop *pIAD = nullptr;

	do {
		
		wstring_convert<codecvt_utf8_utf16 <wchar_t> > converter;

		wstring result = converter.from_bytes(str);

		if (result.empty())
			break;

		HRESULT hr;

		hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (void**)&pIAD);
		if (!SUCCEEDED(hr))
			break;
		
		hr = pIAD->SetWallpaper(result.c_str(), 0);

		if (!SUCCEEDED(hr))
			break;
		WALLPAPEROPT wpo;
		wpo.dwSize = sizeof(wpo);
		wpo.dwStyle = WPSTYLE_KEEPASPECT;
		hr = pIAD->SetWallpaperOptions(&wpo, 0);
		if (!SUCCEEDED(hr))
			break;
		hr = pIAD->ApplyChanges(AD_APPLY_ALL);
		if (!SUCCEEDED(hr))
			break;

		//DWORD_PTR p;
		//SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 0, SMTO_NOTIMEOUTIFNOTHUNG, 0, &p);

	} while (suc=true,false);

	pIAD->Release();

	return suc;
}

bool Wallpaper::next(bool force) {
	if (force || changeable())
		;
	else
		return false;
	Path old(previous);
	previous = current;
	current = repo.get();
	if (set(current)) {
		ui.title(current.filename());
		return true;
	}
	previous = old;
	return false;
}

bool Wallpaper::prev(void) {
	if (previous.type() == Path::UNKNOWN)
		return false;

	if (set(previous)) {
		ui.title(previous.filename());
		swap(previous, current);
		return true;
	}
	return false;

}