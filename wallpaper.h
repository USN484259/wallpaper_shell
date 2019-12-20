#pragma once
#include <string>
#include <fstream>
#include "repository.hpp"
#include "ui.h"

class Wallpaper {
	Repository repo;
	shell_ui ui;

	HANDLE timer;
	LONG interval;
	mutable std::ofstream log_file;

	//HWINEVENTHOOK event_hook;

	bool paused;
	bool power_policy;
	bool maximize_policy;
	bool explorer_policy;

	enum : unsigned { menu_total = 0x10,  menu_cur = 0x12, menu_prev = 0x13, menu_next = 1, menu_paused = 4, menu_power = 5, menu_maxi = 6, menu_exp = 7 };

private:
	bool log(void) const;
	void log(const std::string&) const;

	void parse(const std::string&);
	bool changeable(void) const;
	bool set_timer(void);
	bool cancel_timer(void);
	bool set(const std::string&);
	bool next(bool force = false);
	void menu_event(unsigned);

	//GetMonitorInfoA

	static void CALLBACK on_timer(LPVOID arg, DWORD, DWORD);

	static BOOL CALLBACK on_enum(HWND, LPARAM);

	//static void CALLBACK on_event(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD);
public:
	Wallpaper(const std::string&);
	~Wallpaper(void);



};