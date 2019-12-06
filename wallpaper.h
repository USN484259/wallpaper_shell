#pragma once
#include <string>
#include "repository.hpp"
#include "ui.h"

class Wallpaper {
	Repository repo;
	shell_ui ui;

	HANDLE timer;
	LONG interval;

	USNLIB::filesystem::path current, previous;

	bool paused;
	bool power_policy;
	bool maximize_policy;
	bool explorer_policy;

private:
	void parse(const std::string&);
	bool changeable(void) const;
	bool set(const std::string&);
	bool next(bool force = false);
	bool prev(void);
	void menu_event(unsigned);

	static void CALLBACK on_timer(LPVOID arg, DWORD, DWORD);
public:
	Wallpaper(const std::string&);
	~Wallpaper(void);



};