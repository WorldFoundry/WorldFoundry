//=============================================================================
// memtest.cc:
//=============================================================================

#include <memory/memory.hp>
#include <memory/lmalloc.hp>
#include <memory/dmalloc.hp>
#include <hal/hal.h>

CREATEANDASSIGNOSTREAM(cmem,std::cout);

//=============================================================================

class test
{
public:
	test() { value = 0; }
	test(char bar) { value = bar; }
	int operator==(const test& left) const { return (value == left.value); }
	int value;
};

//=========================================================================

void
TestLMalloc()
{
	std::cout << "testing LMalloc" << std::endl;
	char memory[100000];
	LMalloc testMemory(memory,100000 MEMORY_NAMED( COMMA "TestLMalloc" ) );

	void* one = testMemory.Allocate(100,__FILE__,__LINE__);
	assert(ValidPtr(one));
	void* two = testMemory.Allocate(200,__FILE__,__LINE__);
	assert(ValidPtr(two));

	test* array = (test*)testMemory.Allocate(sizeof(test) * 20,__FILE__,__LINE__);
	assert(ValidPtr(array));

	// make sure ordinary new still works
//	test* foo = new test;
//
//	test* array2 = new test[20];
//
//	delete array2;

	testMemory.Free(array,sizeof(test)*20);
	testMemory.Free(two,200);
	testMemory.Free(one,100);
}

//=============================================================================

void
TestDMalloc()
{
	char memory[100000];
	LMalloc testMemory(memory,100000 MEMORY_NAMED( COMMA "TestLMalloc" ) );

	DMalloc test(testMemory,10000,"Test DMalloc");

	std::cout << "before first:" << test << std::endl;
	void* first = test.Allocate(20,__FILE__,__LINE__);
    std::cout << "first: " << first << std::endl;
	std::cout << "after first:" << test << std::endl;
	void* second = test.Allocate(30,__FILE__,__LINE__);
    std::cout << "second: " << second << std::endl;
	std::cout << "after second:" << test << std::endl;
	void* third = test.Allocate(40,__FILE__,__LINE__);
    std::cout << "third: " << third << std::endl;
	std::cout << "after third:" << test << std::endl;

	assert(second > first);
	assert(third > second);

	test.Free(second);
	std::cout << "after freed second:" << test << std::endl;
	void* fourth = test.Allocate(30,__FILE__,__LINE__);
    std::cout << "fourth: " << fourth << std::endl;
	std::cout << "after fourth:" << test << std::endl;
	assert(second == fourth);
	test.Free(first);
	std::cout << "after freed first:" << test << std::endl;
	void* fifth = test.Allocate(50,__FILE__,__LINE__);
    std::cout << "fifth: " << fifth << std::endl;
	std::cout << "after fifth:" << test << std::endl;
	assert(fifth != first);
	test.Free(fourth);
	std::cout << "after freed fourth:" << test << std::endl;
	void* sixth = test.Allocate(50,__FILE__,__LINE__);
    std::cout << "sixth: " << sixth << std::endl;
	std::cout << "after sixth:" << test << std::endl;
	assert(sixth == first);

	test.Free(fifth);
	test.Free(sixth);
	test.Free(third);


}

//=============================================================================

extern void
CPPMemPoolTest(void);

void
PIGSMain( int argc, char* argv[] )
{
		TestLMalloc();
		TestDMalloc();
		CPPMemPoolTest();

		std::cout << "memtest Done:" << std::endl;
}

//=============================================================================
