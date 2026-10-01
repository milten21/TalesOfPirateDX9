#include <cstddef>
#include <cstdio>
#include <cstring>

#include "ChaAttr.h"
#include "CompCommand.h"
#include "InfoNet.h"
#include "Point.h"

namespace
{
struct Layout
{
	const char* name;
	std::size_t actual;
	std::size_t expected;
};

const Layout layouts[] = {
	{"sizeof(SItemGrid)", sizeof(SItemGrid), 180},
	{"sizeof(LOOK)", sizeof(LOOK), 6128},
	{"offsetof(LOOK, sHairID)", offsetof(LOOK, sHairID), 6124},
	{"sizeof(CChaAttr)", sizeof(CChaAttr), 872},
	{"sizeof(Point)", sizeof(Point), 8},
	{"sizeof(NetMessageHead)", sizeof(NetMessageHead), 40},
	{"offsetof(NetMessageHead, msgOrder)", offsetof(NetMessageHead, msgOrder), 16},
	{"offsetof(NetMessageHead, msgEncSize)", offsetof(NetMessageHead, msgEncSize), 36},
};
}

int main(int argc, char* argv[])
{
	const bool print = argc > 1 && std::strcmp(argv[1], "--print") == 0;
	int failures = 0;
	for (const Layout& layout : layouts)
	{
		if (print)
		{
			std::printf("%s = %u\n", layout.name, static_cast<unsigned>(layout.actual));
			continue;
		}
		if (layout.actual != layout.expected)
		{
			std::printf("%s is %u, the Windows build has %u\n", layout.name, static_cast<unsigned>(layout.actual), static_cast<unsigned>(layout.expected));
			++failures;
		}
	}
	return failures == 0 ? 0 : 1;
}
