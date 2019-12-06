#pragma once
#include <string>
#include "repository.hpp"
#include "ui.h"

class Wallpaper {
	Repository repo;
	shell_ui ui;

	HANDLE timer;
	LONG interval;

	bool paused;
	bool power_policy;
	bool maximize_policy;
	bool explorer_policy;

	enum : unsigned { menu_next = 1, menu_cur = 2, menu_prev = 3, menu_paused = 4, menu_power = 5, menu_maxi = 6, menu_exp = 7 };

private:
	void parse(const std::string&);
	bool changeable(void) const;
	bool set_timer(void);
	bool cancel_timer(void);
	bool set(const std::string&);
	bool next(bool force = false);
	void menu_event(unsigned);


	static void CALLBACK on_timer(LPVOID arg, DWORD, DWORD);
public:
	Wallpaper(const std::string&);
	~Wallpaper(void);



};