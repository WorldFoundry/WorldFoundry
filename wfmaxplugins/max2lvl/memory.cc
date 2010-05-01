//============================================================================
// memory.cc: kts quick memory tracking system
//============================================================================

#include <iostream.h>
//#include <i86.h>


//============================================================================
// kts temp code to print how much memory is available

struct meminfo {
    unsigned LargestBlockAvail;
    unsigned MaxUnlockedPage;
    unsigned LargestLockablePage;
    unsigned LinAddrSpace;
    unsigned NumFreePagesAvail;
    unsigned NumPhysicalPagesFree;
    unsigned TotalPhysicalPages;
    unsigned FreeLinAddrSpace;
    unsigned SizeOfPageFile;
    unsigned Reserved[3];
} MemInfo;

#define DPMI_INT	0x31

//============================================================================

void
DumpFreeMemory(ostream& out)
{
    union REGS regs;
    struct SREGS sregs;

//	out << "Free memory dump" << endl;

    regs.x.eax = 0x00000500;
    memset( &sregs, 0, sizeof(sregs) );
    sregs.es = FP_SEG( &MemInfo );
    regs.x.edi = FP_OFF( &MemInfo );

    int386x( DPMI_INT, &regs, &regs, &sregs );
    out << "  Largest available block (in bytes): " << MemInfo.LargestBlockAvail << endl;
//    out << "Maximum unlocked page allocation: " << MemInfo.MaxUnlockedPage << endl;
//    out << "Pages that can be allocated and locked: " << MemInfo.LargestLockablePage << endl;
//    out << "Total linear address space including allocated pages: " << MemInfo.LinAddrSpace << endl;
//    out << "Number of free pages available: " << MemInfo.NumFreePagesAvail << endl;
//    out << "Number of physical pages not in use: " <<MemInfo.NumPhysicalPagesFree << endl;
//    out << "Total physical pages managed by host: " << MemInfo.TotalPhysicalPages << endl;
//    out << "Free linear address space (pages): " << MemInfo.FreeLinAddrSpace << endl;
//    out << "Size of paging/file partition (pages): " << MemInfo.SizeOfPageFile << endl;
}
