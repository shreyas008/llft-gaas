#include "utils.hpp"

using namespace std;

Window get_window(xdo_t* xwin)
{
	xdo_search_t search; // Window search paramater
	Window* list;
	unsigned int nwindows;

	memset(&search, 0, sizeof(xdo_search_t));
	search.max_depth = -1;
	search.require = xdo_search::SEARCH_ANY;
	search.searchmask |= SEARCH_NAME;
	search.winname = "nanoarch";

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
