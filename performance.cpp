#include "performance.h"
#include <Windows.h>
#include <Pdh.h>
#include <PdhMsg.h>

#pragma comment(lib,"pdh.lib")

using namespace std;

performance::performance(void) : query(NULL),timestamp(0){
	/*
	if (ERROR_SUCCESS != PdhOpenQuery(NULL, NULL, &query))
		throw runtime_error("PDH open");
	if (ERROR_SUCCESS != PdhAddEnglishCounter(query, "\\Processor(_Total)\\% Processor Time", NULL, &processor_counter))
		throw runtime_error("PDH processor add");
		*/

	//"\\PhysicalDisk(0)\\% Disk Time"

	//char buffer[0x100];
	//PDH_BROWSE_DLG_CONFIG cfg = { 0 };
	//cfg.szReturnPathBuffer = buffer;
	//cfg.cchReturnPathLength = 0x100;
	//PdhBrowseCounters(&cfg);
}

performance::~performance(void) {
	if (query)
		PdhCloseQuery(query);
}

bool performance::initialize(void) {
	if (query)
		return true;
	if (ERROR_SUCCESS != PdhOpenQuery(NULL, NULL, &query))
		return false;

	HANDLE new_counter;

	if (ERROR_SUCCESS != PdhAddEnglishCounter(query, "\\Processor(_Total)\\% Processor Time", NULL, &new_counter))
		return false;
	counters.insert(make_pair(string("processor"), new_counter));

	if (ERROR_SUCCESS != PdhAddEnglishCounter(query, "\\PhysicalDisk(_Total)\\% Disk Time", NULL, &new_counter))
		return false;
	counters.insert(make_pair(string("disk"), new_counter));

	return true;
}


bool performance::update(size_t interval) {
	if (!initialize())
		return false;

	time_t current_time = time(NULL);

	if (current_time - timestamp < 2)
		return true;

	timestamp = current_time;

	if (ERROR_SUCCESS != PdhCollectQueryData(query))
		return false;
	Sleep(interval);
	if (ERROR_SUCCESS != PdhCollectQueryData(query))
		return false;
	return true;
}


long performance::operator[](const string& name) {
	auto it = counters.find(name);
	if (it == counters.cend())
		return -1;
	HANDLE current_counter = it->second;

	PDH_FMT_COUNTERVALUE value;

	if (ERROR_SUCCESS != PdhGetFormattedCounterValue(current_counter, PDH_FMT_LONG | PDH_FMT_NOCAP100, NULL, &value))
		return -1;
	if (value.CStatus == PDH_CSTATUS_VALID_DATA || value.CStatus == PDH_CSTATUS_NEW_DATA)
		;
	else
		return -1;

	return value.longValue;

}