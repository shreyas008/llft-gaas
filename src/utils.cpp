#include "utils.hpp"

using namespace std;

Window get_window(xdo_t* xwin, const char* winname)
{
	xdo_search_t search; // Window search paramater
	Window* list;
	unsigned int nwindows;

	memset(&search, 0, sizeof(xdo_search_t));
	search.max_depth = -1;
	search.require = xdo_search::SEARCH_ANY;
	search.searchmask |= SEARCH_NAME;
	search.winname = winname;

	xdo_search_windows(xwin, &search, &list, &nwindows);
	return list[0];
}

// Helper function to generate a random ID
string randomId(size_t length) {
	// static const string characters(
	//     "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	static const string characters("0123456789");
	string id(length, '0');
	default_random_engine rng(random_device{}());
	uniform_int_distribution<int> dist(0, int(characters.size() - 1));
	generate(id.begin(), id.end(), [&]() { return characters.at(dist(rng)); });
	return id;
}

void* create_shared_memory(char* mem_name, int size)
{
	int memFd = shm_open(mem_name, O_CREAT | O_RDWR, S_IRWXU);
	int res = ftruncate(memFd, size);
	void *buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);
	return buffer;
}

void* open_shared_memory(char* mem_name, int size) {
	int memFd = shm_open(mem_name, O_RDWR, 0);
	void *buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);
	return buffer;
}

void destroy_shared_memory(vector<char*> mem_names) {
    return;
}
