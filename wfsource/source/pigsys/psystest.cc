//=============================================================================
// psystest.c:
// Copyright ( c ) 1994,95,96,97,98,99 World Foundry Group  
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org

//===========================================================================
// $RCSfile$
// Description: A quick test of the implementation of pigsys.hp.
// Original Author: Andre Burgoyne
// $Header$
//===========================================================================

#define	_PSYSTEST_C
#include <pigsys/pigsys.hp>

//=============================================================================

int
main( int argc, char* argv[] )
{
//	void*	vbp1;
	void*	vbp2;
	char*	b1;
	char*	b2;

//#if defined(__PSX__)|| defined(__SAT__)
//	int		argc = 0;
//	char**	argv = NULL;
//#endif

#if defined( DOS )
	// Example code to show how to increase size of tree (in case of an
	// internal MemCheck error) because it couldn't add more entries...
	printf( "MC_Settings.MaxMem = %d\n", MC_Settings.MaxMem );
	MC_Settings.MaxMem = 4096;
#endif

	sys_init(&argc, &argv);
	printf("sys_init finished\n");

//	vbp1 = calloc(1,1);
//	printf("expect a memory leak at in %s, line %d\n", __FILE__,__LINE__-1);
//	printf("sys_init finished1, vbp1 = %lx\n",vbp1);
//        assert( ValidPtr(vbp1) );
//	printf("sys_init finished2\n");
//	assert( ((char*)vbp1)[0] == 0 );
//	printf("sys_init finished3\n");

	printf("Boolean tests: true = %p, false = %p\n", (1==1), (1==0) );

#if defined( MEMCHECK )
	{
	char* a = (char*)malloc( 10 );
	memset( a, 0, 11 );
	}
#endif

//	b2 = b1 = (char*)malloc(0x80000);	// attempt big-block (500K) allocation
//        assert( ValidPtr(b1) );
//		free(b2);

	assert( ValidPtr( (void*)1000 ) );

	printf("Expect a null pointer assertion\n");
	AssertMsg( ValidPtr( 0 ), "Assertion with a message" );

	//NOT_REACHED
	return 0;
}
