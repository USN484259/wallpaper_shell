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



Wallpaper::Wallpaper(const string& cmd) : ui(bind(&Wallpaper::menu_event, this, placeholders::_1)), timer(CreateWaitableTimer(NULL, TRUE, NULL)), interval(30 * 1000), paused(false), power_policy(true), maximize_policy(true), explorer_policy(true) {
	CoInitialize(NULL);
	ui.menu_item(menu_cur, "");
	ui.menu_item(menu_prev, "");
	ui.menu_item();
	ui.menu_item(menu_power, "Pause on battery power");
	ui.menu_item(menu_maxi, "Pause on maximaized");
	ui.menu_item(menu_exp, "Pause on folder opened");
	ui.menu_item();
	ui.menu_item(menu_paused, "Pause manually");
	ui.menu_item();
	ui.menu_item(menu_next, "Next wallpaper");
	ui.menu_item();

	parse(cmd);

	ui.menu_checked(menu_paused, paused);
	ui.menu_checked(menu_power, power_policy);
	ui.menu_checked(menu_maxi, maximize_policy);
	ui.menu_checked(menu_exp, explorer_policy);

	ui.menu_string(menu_cur, "current: (none)");
	ui.menu_string(menu_prev, "previous: (none)");

	if (!paused) {
		set_timer();
	}

	ui.show();

}

Wallpaper::~Wallpaper(void) {
	cancel_timer();
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

bool Wallpaper::set_timer(void) {
	LARGE_INTEGER due;
	due.QuadPart = -100000000LL;
	if (!SetWaitableTimer(timer, &due, interval, on_timer, this, FALSE))
		return false;
	return true;

}

bool Wallpaper::cancel_timer(void) {
	return CancelWaitableTimer(timer) ? true : false;
}

void Wallpaper::menu_event(unsigned id) {
	switch (id) {
	case menu_next:
		next(true);
		break;
	case menu_prev:

		break;
	case menu_paused:
	{
		if (paused) {
			if (set_timer())
				paused = false;
		}
		else {
			if (cancel_timer())
				paused = true;
		}
		ui.menu_checked(menu_paused, paused);
		break;
	}

	case menu_power:
		ui.menu_checked(menu_power,power_policy = !power_policy);
		break;
	case menu_maxi:
		ui.menu_checked(menu_maxi,maximize_policy = !maximize_policy);
		break;
	case menu_exp:	//explorer
		ui.menu_checked(menu_exp,explorer_policy = !explorer_policy);
		break;

	case shell_ui::session_locked:
		cancel_timer();
		break;
	case shell_ui::session_unlocked:
		if (!paused)
			set_timer();
		break;
	}
}

bool Wallpaper::changeable(void) const {
	/*
	HDESK desk = OpenInputDesktop(0, FALSE, GENERIC_READ);
	if (!desk)
		return false;

	DWORD len;
	USEROBJECTFLAGS info;
	GetUserObjectInformation(desk, UOI_FLAGS, &info, sizeof(USEROBJECTFLAGS), &len);
	CloseDesktop(desk);

	if (info.dwFlags & WSF_VISIBLE)
		;
	else
		return false;
		*/

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
	//Path old(previous);
	//previous = current;
	string old_title("previous: ");
	old_title.append(ui.title());
	const Path& p = repo.get();
	if (set(p)) {
		ui.title(p.filename());
		ui.menu_string(menu_cur, string("current: ").append(p.filename()).c_str());
		ui.menu_string(menu_prev, old_title.c_str());

		return true;
	}
	//previous = old;
	return false;
}

//bool Wallpaper::prev(void) {
//	if (previous.type() == Path::UNKNOWN)
//		return false;
//
//	if (set(previous)) {
//		ui.title(previous.filename());
//		swap(previous, current);
//		return true;
//	}
//	return false;
//
//}