#define _CRTDBG_MAP_ALLOC
#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	testing::InitGoogleTest(&argc, argv);

	auto res = RUN_ALL_TESTS();
	_CrtDumpMemoryLeaks();
	return res;
}
