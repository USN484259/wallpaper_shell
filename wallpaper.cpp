#define _CRT_SECURE_NO_WARNINGS
#include "wallpaper.h"
#include <array>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include <functional>
#include <ctime>
#include <iomanip>

#include <Windows.h>
//#include <wininet.h>
//#include <shlobj.h>

#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")

using namespace std;
using Path = USNLIB::filesystem::path;

//#ifdef _DEBUG
//#define verbose_log(s) log(s)
//#else
//#define verbose_log(s)
//#endif

Wallpaper::Wallpaper(const string& cmd) : ui(bind(&Wallpaper::menu_event, this, placeholders::_1)), timer(CreateWaitableTimer(NULL, TRUE, NULL)), interval(30 * 1000), paused(false), power_policy(true), maximize_policy(true), explorer_policy(true) {
	//CoInitialize(NULL);
	ui.menu_item(menu_total, "");
	ui.menu_item();
	ui.menu_item(menu_cur, "");
	ui.menu_item(menu_prev, "");
	ui.menu_item();
	ui.menu_item(menu_power, "Pause on battery power");
	ui.menu_item(menu_maxi, "Pause on maximaized");
	ui.menu_item(menu_exp, "Pause on folder opened");
	ui.menu_item(menu_paused, "Pause manually");
	ui.menu_item();
	ui.menu_item(menu_next, "Next wallpaper");
	ui.menu_item();

	parse(cmd);

	ui.menu_checked(menu_paused, paused);
	ui.menu_checked(menu_power, power_policy);
	ui.menu_checked(menu_maxi, maximize_policy);
	ui.menu_checked(menu_exp, explorer_policy);

	stringstream ss;
	ss << "Total " << repo.size() << " images";
	ui.menu_string(menu_total, ss.str().c_str());
	ui.menu_string(menu_cur, "current: (none)");
	ui.menu_string(menu_prev, "previous: (none)");

	if (log()) {
		log("-------- wallpaper_shell --------");
		ss << '\t' << "Interval " << dec << interval << " ms" << endl;
		log(ss.str());
		ss.str(string());
		ss << "paused " << paused << '\t' << "power_policy " << power_policy << '\t' << "maximize_policy " << maximize_policy << '\t' << "explorer_policy " << explorer_policy << endl;
		log(ss.str());
	}

	if (!paused) {
		set_timer();
	}

	//event_hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_MINIMIZEEND, NULL, on_event, 0, 0, 0);

	ui.show();
}

Wallpaper::~Wallpaper(void) {
	cancel_timer();
	//UnhookWinEvent(event_hook);
	//CoUninitialize();
	CloseHandle(timer);
	log("-------- ~wallpaper_shell --------");
}

bool Wallpaper::log(void) const {
	return log_file.is_open();
}

void Wallpaper::log(const string& str) const{
	if (!log())
		return;
	time_t t = time(nullptr);
	log_file << put_time(localtime(&t), "%F %T%t") << str;
	if (str.back() != '\n')
		log_file << endl;
}


void CALLBACK Wallpaper::on_timer(LPVOID arg, DWORD, DWORD) {
	Wallpaper* wp = (Wallpaper*)arg;
	wp->next();

}

/*
void CALLBACK Wallpaper::on_event(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG, LONG, DWORD, DWORD) {
	static ofstream event_log(".\\events.log",ios::app);
	stringstream ss;
	if (!hwnd)
		return;
	//HWND desktop = GetDesktopWindow();
	//if (GetWindow(hwnd, GW_OWNER) != desktop)
	//	return;

	switch (event) {
	case EVENT_SYSTEM_FOREGROUND:
		ss << "EVENT_SYSTEM_FOREGROUND";
		break;
	case EVENT_SYSTEM_MINIMIZEEND:
		ss << "EVENT_SYSTEM_MINIMIZEEND";
		break;
	case EVENT_SYSTEM_MINIMIZESTART:
		ss << "EVENT_SYSTEM_MINIMIZESTART";
		break;
	case EVENT_SYSTEM_MOVESIZEEND:
		ss << "EVENT_SYSTEM_MOVESIZEEND";
		break;
	case EVENT_SYSTEM_MOVESIZESTART:
		ss << "EVENT_SYSTEM_MOVESIZESTART";
		break;
	case EVENT_SYSTEM_SWITCHEND:
		ss << "EVENT_SYSTEM_SWITCHEND";
		break;
	case EVENT_SYSTEM_CAPTUREEND:
		ss << "EVENT_SYSTEM_CAPTUREEND";
		break;
	case EVENT_SYSTEM_CAPTURESTART:
		ss << "EVENT_SYSTEM_CAPTURESTART";
		break;

	//case EVENT_OBJECT_CREATE:
	//	ss << "EVENT_OBJECT_CREATE";
	//	break;
	//case EVENT_OBJECT_DESTROY:
	//	ss << "EVENT_OBJECT_DESTROY";
	//	break;
	//case EVENT_OBJECT_FOCUS:
	//	ss << "EVENT_OBJECT_FOCUS";
	//	break;
	//case EVENT_OBJECT_HIDE:
	//	ss << "EVENT_OBJECT_HIDE";
	//	break;
	//case EVENT_OBJECT_LOCATIONCHANGE:
	//	ss << "EVENT_OBJECT_LOCATIONCHANGE";
	//	break;
	//case EVENT_OBJECT_SHOW:
	//	ss << "EVENT_OBJECT_SHOW";
	//	break;
	//case EVENT_OBJECT_STATECHANGE:
	//	ss << "EVENT_OBJECT_STATECHANGE";
	//	break;
	//case EVENT_OBJECT_VALUECHANGE:
	//	ss << "EVENT_OBJECT_VALUECHANGE";
	//	break;
	//case EVENT_OBJECT_REORDER:
	//	ss << "EVENT_OBJECT_REORDER";
	//	break;
	//case EVENT_OBJECT_PARENTCHANGE:
	//	ss << "EVENT_OBJECT_PARENTCHANGE";
	//	break;

	//default:
	//	return;
	}
	ss << '\t' << hex << event << '\t' << hwnd << ' ';
	char buf[0x100] = { 0 };
	RECT area = { 0 };
	GetClassName(hwnd, buf, 0x100);
	GetWindowRect(hwnd, &area);
	ss << buf << dec << " [" << area.left << ',' << area.top << "] [" << area.right << ',' << area.bottom << ']' << endl;
	event_log << ss.str();
}
*/

void Wallpaper::parse(const string& cmd) {
	stringstream ss;
	ss.str(cmd);
	string str;
	bool recursive = false;
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
			case 'R':
				recursive = true;
				break;
			case 'r':
				recursive = false;
				break;
			case 'i':
			case 'I':
				interval = 1000 * strtoul(str.substr(2).c_str(), nullptr, 10);
				if (interval < 5000)
					interval = 30 * 1000;
				break;
			case 'l':
			case 'L':
				log_file.close();
				log_file.open(str.substr(str.at(2) == ':' ? 3 : 2), ios::app);
				break;
			}
		}
		else
			repo.put(str, recursive);

		str.clear();

	}
	if (0 == repo.size())
		throw runtime_error("No image selected");

}

bool Wallpaper::set_timer(void) {
	log("set_timer");
	LARGE_INTEGER due;
	due.QuadPart = -100000000LL;
	if (!SetWaitableTimer(timer, &due, interval, on_timer, this, FALSE)) {
		log("set_timer failed");
		return false;
	}
	return true;

}

bool Wallpaper::cancel_timer(void) {
	log("cancel_timer");
	return CancelWaitableTimer(timer) ? true : false;
}

void Wallpaper::menu_event(unsigned id) {
	switch (id) {
	case menu_next:
		log("manual change");
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
		log(string("paused ").append(paused ? "true":"false"));
		break;
	}

	case menu_power:
		ui.menu_checked(menu_power,power_policy = !power_policy);
		log(string("power_policy ").append(power_policy ? "true" : "false"));
		break;
	case menu_maxi:
		ui.menu_checked(menu_maxi,maximize_policy = !maximize_policy);
		log(string("maximize_policy ").append(maximize_policy ? "true" : "false"));
		break;
	case menu_exp:	//explorer
		ui.menu_checked(menu_exp,explorer_policy = !explorer_policy);
		log(string("explorer_policy ").append(explorer_policy ? "true" : "false"));
		break;

	case shell_ui::session_locked:
		log("session_locked");
		cancel_timer();
		break;
	case shell_ui::session_unlocked:
		log("session_unlocked");
		if (!paused)
			set_timer();
		break;
	}
}

bool operator==(const RECT& a, const RECT& b) {
	return abs(a.left - b.left) + abs(a.top - b.top) + abs(a.right - b.right) + abs(a.bottom - b.bottom) <= 32;
}


struct enum_window_info {
	bool explorer_policy;
	stringstream& reason;
	RECT desktop;

};

static const string folder_class_name("CabinetWClass");


bool Wallpaper::changeable(void) const {
	stringstream reason("Not changeable :");
	do {
		if (power_policy) {
			SYSTEM_POWER_STATUS ps;
			if (!GetSystemPowerStatus(&ps)) {
				reason << " GetSystemPowerStatus failed";
				break;
			}
			if (ps.ACLineStatus == 0) {
				reason << " Not AC powered";
				break;
			}
		}

		if (maximize_policy) {
			enum_window_info info{ explorer_policy ,reason ,0 };
			GetWindowRect(GetDesktopWindow(), &info.desktop);
			if (!EnumWindows(on_enum, (LPARAM)&info))
				break;

			/*

			HWND window = GetDesktopWindow();
			RECT screen = { 0 };
			//HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY);
			//MONITORINFO screen = { sizeof(MONITORINFO),0 };
			//GetMonitorInfo(monitor, &screen);

			GetWindowRect(window, &screen);
			window = GetWindow(window, GW_CHILD);
			//DWORD pid_exp = 0;
			static const array<string, 3> passed_names = { "Windows.UI.Core.CoreWindow","WorkerW","Progman" };
			static const string taskbar_class_name("Shell_TrayWnd");
			char buf[0x100] = { 0 };
			for (; window; window = GetWindow(window, GW_HWNDNEXT)) {


				
				//if (taskbar_class_name == buf) {
				//	GetWindowThreadProcessId(window, &pid_exp);
				//}

				LONG style = GetWindowLong(window, GWL_STYLE);
				if (!(style & WS_VISIBLE) || (style & WS_DISABLED)) {
					continue;
				}

				if (GetWindowLong(window, GWL_EXSTYLE) & (WS_EX_TOPMOST | WS_EX_NOACTIVATE)) {
					continue;
				}

				DWORD clocked = 0;
				if (S_OK == DwmGetWindowAttribute(window, DWMWA_CLOAKED, &clocked, 4) && clocked) {
					continue;
				}


				if (style & WS_MINIMIZE) {
					continue;
				}


				GetClassName(window, buf, 0x100);

				if (style & WS_MAXIMIZE) {
					reason << " Window maximized";
					break;
				}

				if (passed_names.cend() != find(passed_names.cbegin(), passed_names.cend(), string(buf)))
					continue;

				if (explorer_policy && folder_class_name == buf) {
					reason << " Found opened folder";
					break;
				}

				//DWORD pid;
				//GetWindowThreadProcessId(window, &pid);
				//if (pid == pid_exp) {
				//	continue;
				//}




				RECT area = { 0 };
				if (GetWindowRect(window, &area) && screen == area) {
					reason << " Window fullscreen";
					break;
				}


			}
			if (window) {
				reason << ':' << buf << ' ' << hex << window;
				break;
			}
			*/
		}

		else if (explorer_policy) {
			if (FindWindowA(folder_class_name.c_str(), NULL)) {
				reason << " Found opened folder";
				break;
			}
		}


		return true;
	} while (false);

	log(reason.str());
	return false;
}

BOOL CALLBACK Wallpaper::on_enum(HWND hwnd, LPARAM lparam) {
	//const Wallpaper* This = static_cast<const Wallpaper*>((const void*)lparam);
	enum_window_info& info = *(enum_window_info*)lparam;

	static const array<string, 3> passed_names = { "Windows.UI.Core.CoreWindow","WorkerW","Progman" };
	static const string taskbar_class_name("Shell_TrayWnd");



	LONG style = GetWindowLong(hwnd, GWL_STYLE);
	if (!(style & WS_VISIBLE) || (style & WS_DISABLED)) {
		return TRUE;
	}

	if (GetWindowLong(hwnd, GWL_EXSTYLE) & (WS_EX_TOPMOST | WS_EX_NOACTIVATE)) {
		return TRUE;
	}

	DWORD clocked = 0;
	if (S_OK == DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &clocked, 4) && clocked) {
		return TRUE;
	}

	if (style & WS_MINIMIZE) {
		return TRUE;
	}

	char buf[0x100] = { 0 };
	GetClassName(hwnd, buf, 0x100);

	do {

		if (style & WS_MAXIMIZE) {
			info.reason << " Window maximized";
			break;
		}

		if (passed_names.cend() != find(passed_names.cbegin(), passed_names.cend(), string(buf)))
			return TRUE;

		if (info.explorer_policy && folder_class_name == buf) {
			info.reason << " Found opened folder";
			break;
		}

		//DWORD pid;
		//GetWindowThreadProcessId(window, &pid);
		//if (pid == pid_exp) {
		//	continue;
		//}




		RECT area = { 0 };
		if (GetWindowRect(hwnd, &area) && info.desktop == area) {
			info.reason << " Window fullscreen";
			break;
		}



		return TRUE;
	} while (false);


	info.reason << ':' << buf << ' ' << hex << hwnd;

	return FALSE;
}


bool Wallpaper::set(const string& str) {
	if (!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (PVOID)str.c_str(), 0)) {
		log("set wallpaper failed");
		return false;
	}
	return true;

}

/*
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
	if (!suc) {
		log("set wallpaper failed");
	}
	return suc;
}
*/

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
	log(p);
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