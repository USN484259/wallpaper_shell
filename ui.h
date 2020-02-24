#pragma once
#include "gui.hpp"
#include <functional>

class shell_ui : public USNLIB::gui {
public:
	typedef std::function<void(unsigned)> handler_type;
	enum :unsigned { taskbar_created = 0x3FFF, session_locked = 0x3FF7, session_unlocked = 0x3FF8 };
private:
	enum : unsigned { menu_exit_id = 0x7FFF };

	HMENU menu;
	const UINT TaskbarCreated;
	handler_type const handler;

	static const GUID guid;
	bool place_icon(void);


protected:
	LRESULT msg_proc(UINT, WPARAM, LPARAM) override;
	bool msg_pump(MSG&) override;


public:
	shell_ui(handler_type);
	~shell_ui(void);

	void menu_item(unsigned = 0, const char* = nullptr);
	void menu_checked(unsigned, bool);
	void menu_string(unsigned, const char*);
	void title(const std::string&) override;
	using gui::title;
};