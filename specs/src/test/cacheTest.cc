#include <iostream>
#include <string>
#include "utils/lruCache.h"

typedef std::shared_ptr<std::string> PString;

int main(int argc, char** argv)
{
	lruCache<int, std::string> cache(20,25);

	std::cout << "First, let's add 25 entries to the cache\n";

	for (int i=0; i<25; i++) {
		auto ps = std::make_shared<std::string>(std::to_string(i));
		cache.set(i, ps);
	}

	cache.Debug();

	std::cout << "Let's get all except 10 so that it has the lowest lru value\n";

	for (int i=0; i<25; i++) {
		if (i != 10) cache.get(i);
	}

	cache.Debug();

	std::cout << "Now let's set a new value. It should replace 10\n";
	int i=30;
	cache.set(i, std::make_shared<std::string>("30"));

	cache.Debug();

	std::cout << "So let's try to get 10. It should be missing...";
	i = 10;
	auto ps = cache.get(i);
	if (ps) {
		std::cout << "it's not!: " << *ps << std::endl;
		return -4;
	} else {
		std::cout << "it is\n";
	}

	std::cout << "Add a whole bunch of numbers. No other should remain\n";
	for (int i=100; i<=200; i++) {
		auto ps = std::make_shared<std::string>(std::to_string(i));
		cache.set(i, ps);
	}

	cache.Debug();

	return 0;
}
