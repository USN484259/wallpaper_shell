#pragma once
#include "gui.hpp"
#include <functional>

class shell_ui :public USNLIB::gui {
public:
	typedef std::function<void(unsigned)> handler_type;
private:
	enum : unsigned { menu_exit_id = 0x7FFF };

	HMENU menu;
	const UINT TaskbarCreated;
	handler_type handler;

	static const GUID guid;
	bool place_icon(void);


protected:
	LRESULT msg_proc(UINT, WPARAM, LPARAM) override;
	bool msg_pump(MSG&) override;
public:
	shell_ui(void);
	~shell_ui(void);
	void menu_handler(handler_type);
	void menu_item(unsigned = 0, const char* = nullptr);
	void menu_checked(unsigned, bool);
	void title(const std::string&) override;

};