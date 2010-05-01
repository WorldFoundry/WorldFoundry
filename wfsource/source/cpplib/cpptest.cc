//=============================================================================
// cpptest.cc:
//=============================================================================

//#include <cpplib/array.hp>
#include <cpplib/stdstrm.hp>
#include <streams/dbstrm.hp>

//-----------------------------------------------------------------------------

//#if defined(_MSC_VER) && SW_DBSTREAM
//#include <cpplib/strmnull.hp>
//CREATENULLSTREAM(cppteststrm_null);				// Create a global instance of the output
//DBSTREAM1( CREATEANDASSIGNOSTREAM(ccpp,cppteststrm_null); )
//#endif

//=============================================================================

class test
{
public:
	test() { value = 0; }
	test(char bar) { value = bar; }
	int operator==(const test& left) const { return (value == left.value); }
	int value;
};


#if 0
void
TestArray()
{
	cout << "testing array<test>" << endl;
	test foo(10);
	test bar(20);

	test baz;

	baz = bar;
	cout << "foo = " << foo.value << endl;
	cout << "bar = " << bar.value << endl;
	cout << "baz = " << baz.value << endl;


	Array<test> _array;
	assert(!_array.Valid());
	assert(_array.Size() == 0);

	_array.SetMax(10);
	assert(_array.Valid());
	assert(_array.Size() == 0);

	_array.Add(foo);
	assert(_array.Valid());
	assert(_array.Size() == 1);
	_array.Add(bar);
	assert(_array.Valid());
	assert(_array.Size() == 2);

	ArrayIterConst<test> iter(_array);

	cout << "_array dump:" << endl;
	int index = 0;
	while(!iter.Empty())
	{
		test temp;

		temp = *iter;
		cout << "index = " << index << ", value = " << temp.value << endl;
		++iter;
		++index;
	}

}
#endif
//=============================================================================

void
main( int argc, char* argv[] )
{
//		TestArray();

		DBSTREAM1( std::cout << "ostream test:" << std::endl; )

		long foo = 10;


//		char carray[20];
//		ostrstream test(carray,20);
//		test << "this is a test" << std::endl;

#if SW_DBSTREAMS > 0
		cout.width(8);
		cout.fill('!');

		std::cout << "10 in hex: " << hex << foo << std::endl;
		std::cout << "10 in dec: " << dec << foo << std::endl;

		std::cout << "12345678 in hex: " << hex << long(0x12345678) << std::endl;
		std::cout << "9abcdef in hex: " << hex << long(0x9abcdef) << std::endl;
		std::cout << "ffecc1fc in hex: " << hex << long(0xffecc1fc) << std::endl;

		std::cout << "0xaa55 in hex:" << long(0xaa55) << std::endl;


		std::cnull << "this went to null" << std::endl;


		std::cout << "CPPTest Done:" << std::endl;
#endif

    cprogress << "cprogress!" << std::endl;


	sys_exit(1);

//	)
	//return 0;
}
