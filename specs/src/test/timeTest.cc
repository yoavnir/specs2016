#include "utils/TimeUtils.h"

int main(int argc, char** argv)
{
	uint64_t now = specTimeGetTOD();

	std::cout << "Time: " << specTimeConvertToPrintable(now, argv[1]) << "\n";

	return 0;
}
