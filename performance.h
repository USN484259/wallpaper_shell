#pragma once
#include <unordered_map>
#include <string>
#include <ctime>
typedef void* HANDLE;

class performance {
	HANDLE query;
	std::unordered_map<std::string, HANDLE> counters;
	time_t timestamp;

	bool initialize(void);

public:
	performance(void);
	~performance(void);

	bool update(size_t interval = 100);

	long operator[](const std::string&);

};