#include "ui.h"
#include <windowsx.h>
#include <WtsApi32.h>
#pragma comment(lib,"Wtsapi32.lib")
#include <stdexcept>
#include "resource.h"
using namespace std;
using namespace USNLIB;

const GUID shell_ui::guid =
#ifdef _DEBUG
{ 0xdd02320c, 0xfa56, 0x4369,{ 0x89, 0xef, 0x43, 0xea, 0x90, 0x3a, 0x88, 0x77 } };
#else
{ 0x69ec26b3, 0xf719, 0x460e,{ 0x89, 0x7d, 0xd7, 0x44, 0x7b, 0xf1, 0x3e, 0x82 } };
#endif


bool shell_ui::place_icon(void) {
	NOTIFYICONDATA icon = { 0 };

	icon.cbSize = sizeof(NOTIFYICONDATA);
	icon.hWnd = hwnd;
	icon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_GUID | NIF_SHOWTIP;
	icon.uCallbackMessage = WM_USER + 259;
	icon.hIcon = LoadIcon(instance, MAKEINTRESOURCE(MAIN_ICON));
	strcpy_s(icon.szTip, tit.c_str());
	icon.guidItem = guid;

	Shell_NotifyIcon(NIM_DELETE, &icon);
	if (!Shell_NotifyIcon(NIM_ADD, &icon))
		return false;

	icon.cbSize = sizeof(NOTIFYICONDATA);
	icon.uVersion = NOTIFYICON_VERSION_4;

	if (!Shell_NotifyIcon(NIM_SETVERSION, &icon))
		return false;

	return true;

}

LRESULT shell_ui::msg_proc(UINT msg, WPARAM wParam, LPARAM lParam) {

	// {DD02320C-FA56-4369-89EF-43EA903A8877}

	if (msg == TaskbarCreated) {
		place_icon();
		return 0;
	}

	switch (msg) {
	case WM_CREATE:
	{
		if (place_icon() && WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION))
			return 0;
		return -1;

	}
	case WM_CLOSE:
	{
		NOTIFYICONDATA icon = { 0 };
		icon.cbSize = sizeof(NOTIFYICONDATA);
		icon.hWnd = hwnd;
		icon.uFlags = NIF_GUID;
		icon.guidItem = guid;

		Shell_NotifyIcon(NIM_DELETE, &icon);

		WTSUnRegisterSessionNotification(hwnd);

		break;
	}
	case WM_WTSSESSION_CHANGE:
		switch (wParam) {
		case WTS_SESSION_LOCK:
		case WTS_SESSION_UNLOCK:
			handler(wParam | 0x3FF0);
		}
		break;
	case WM_USER + 259:
	{
		switch (LOWORD(lParam)) {
		case WM_LBUTTONUP:
		case WM_CONTEXTMENU:

			SetForegroundWindow(hwnd);


			int selection = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_NOANIMATION, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam), 0, hwnd, NULL);

			switch (selection) {
			case menu_exit_id:
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			case 0:
				break;
			default:
				handler(selection);
			}
			return 0;

		}

	}

	}

	return gui::msg_proc(msg, wParam, lParam);
}

bool shell_ui::msg_pump(MSG& msg) {
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD))
			return msg.message != WM_QUIT;

		MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);

	}
}


shell_ui::shell_ui(handler_type h) : gui(typeid(shell_ui).name()), menu(CreatePopupMenu()), TaskbarCreated(RegisterWindowMessage(TEXT("TaskbarCreated"))), handler(h) {
	style = 0;
	if (!menu)
		throw runtime_error("shell_ui CreateMenu");
	//if (AppendMenu(menu, 0, 2, "Next")
	//	&& AppendMenu(menu, MF_SEPARATOR, 0, NULL)
	//	&& AppendMenu(menu, 0, 1, "Exit")
	//	
	//	);
	//else
	if (!AppendMenu(menu, 0, menu_exit_id, "Exit"))
		throw runtime_error("shell_ui AppendMenu");

	tit = "wallpaper_engine";

}

shell_ui::~shell_ui(void) {
	DestroyMenu(menu);
}

void shell_ui::menu_item(unsigned id, const char* str) {
	//if (id == menu_exit_id)
	//	throw runtime_error("shell_ui::menu_item id conflict");
	InsertMenu(menu, menu_exit_id, id ? 0 : MF_SEPARATOR, id, str);
}

void shell_ui::menu_checked(unsigned id, bool state) {
	CheckMenuItem(menu, id, state ? MF_CHECKED : 0);
}

void shell_ui::menu_string(unsigned id, const char* str) {
	ModifyMenu(menu, id, MF_DISABLED, id, str);
}

void shell_ui::title(const string& str) {
	gui::title(str);

	NOTIFYICONDATA icon = { 0 };

	icon.cbSize = sizeof(NOTIFYICONDATA);
	icon.hWnd = hwnd;

	icon.uFlags = NIF_GUID | NIF_TIP | NIF_SHOWTIP;
	strcpy_s(icon.szTip, str.c_str());
	icon.guidItem = guid;
	Shell_NotifyIcon(NIM_MODIFY, &icon);

}
