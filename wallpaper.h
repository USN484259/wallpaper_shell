#pragma once
#include <string>


class Wallpaper {

public:
	Wallpaper(void);
	~Wallpaper(void);

	bool changeable(void) const;

	bool set(const std::string&);


};