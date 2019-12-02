#include "repository.hpp"
#include <ctime>

using namespace std;
using Path = USNLIB::filesystem::path;


Repository::Repository(void) : rng(time(nullptr)) {}

size_t Repository::put(const string& str) {
	Path p(str);
	//switch (p.type()) {
	//case Path::FILE:
	//	pool.push_back(p);
	//	return 1;
	//case Path::DIRECTORY:
	//	break;
	//default:
	//	return 0;
	//}

	////iterate through DIR

	size_t count = 0;
	for (auto it = p.begin(); it != p.end(); ++it) {
		Path file = *it;
		if (file.type() == Path::FILE) {
			string ext = file.extension();
			if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "bmp") {
				pool.push_back(file);
				count++;
			}
		}
	}
	return count;
}

const Path& Repository::get(void) const {
	return pool.at(rng() % pool.size());
}


size_t Repository::size(void) const {
	return pool.size();
}