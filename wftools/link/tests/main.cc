
extern void a();

int
main( int argc, char* argv[] )
{
	a();
	return 0;
}

extern "C" void __main() { }
