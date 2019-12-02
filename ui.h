#pragma once
#include "gui.hpp"

class shell_ui :public USNLIB::gui {
	HMENU menu;
	const UINT TaskbarCreated;
	static const GUID guid;

	bool place_icon(void);

protected:
	LRESULT msg_proc(UINT, WPARAM, LPARAM) override;
	bool msg_pump(MSG&) override;
public:
	shell_ui(void);
	~shell_ui(void);

	void title(const std::string&) override;

};