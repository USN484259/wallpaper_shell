#pragma once
#include <deque>
#include <string>
#include <random>
#include "filesystem.hpp"


class Repository {
	std::deque<USNLIB::filesystem::path> pool;
	mutable std::default_random_engine rng;
public:
	Repository(void);
	Repository(const Repository&) = delete;
	Repository& operator=(const Repository&) = delete;
	size_t put(const std::string&);
	const USNLIB::filesystem::path& get(void) const;
	size_t size(void) const;




};