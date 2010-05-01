//=============================================================================
// Item.c: Implementation of "items"
//=============================================================================
// dependencies

#define __iTEM_C
#include <hal/halbase.h>
#include <hal/item.h>

#include <cstdio>

//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//			Debug-time-only memory management
//			"Items" are managed in debug version,
//			but just pointers in ship version.
//	History:
//			Created	01-16-95 11:32pm Joseph Boyle
//			(in progress)

//	Class Hierarchy:
//			none

//	Dependancies:
//		item.h

//	Restrictions:

//	Example:

//=============================================================================

#if DO_VALIDATION

#define truncate(num) num&(MAXITEMS-1)
//#define truncate(num) num%MAXITEMS

static long itemCounter=1;
typedef struct _itemEntry {
	int	inuse;
	long	ID;
	void*   data;
	const char* type;
} itemEntry;

static itemEntry itemArray[MAXITEMS];

Item
GetEmptyItem(void)
{
	int tries;
	// search for empty entry
   	tries = 0;
	while(itemArray[truncate(itemCounter)].inuse)  {
		assert(tries++<MAXITEMS);
		itemCounter++;
		assert(itemCounter>0);	  // overflow after 2 billion items
	}
	return(itemCounter);
}

//=============================================================================

Item
ItemAlloc(void* ptr, const char* type)
{
	Item i; itemEntry *entry;

	i = GetEmptyItem(); entry = &itemArray[truncate(i)];

	entry->inuse = true;
	entry->ID 	= itemCounter;
	entry->data	= ptr;
	entry->type	= type;
#if ITEM_PRINT
	printf("Allocating Item"); ItemPrint(i);
#endif
	return(i);
}

//=============================================================================

void
ItemUseCheck(Item itemID)
{	itemEntry *entry;
#if ITEM_PRINT
	ItemPrint(itemID);
#endif
	assert(itemID>0);
	entry = &itemArray[truncate(itemID)];
	assert(entry->inuse == true);
	assert(entry->ID == itemID);
	VALIDATEPTR(entry->data);
}

//=============================================================================

void
ItemCheck(Item itemID, const char* type)
{
	ItemUseCheck(itemID);
#if ITEM_PRINT
	ItemPrint(itemID);
#endif
	assert(strcmp(itemArray[truncate(itemID)].type,type) == 0);
	//		   runtime type checking
}

//=============================================================================

void
ItemFree(Item itemID, const char* type)
{	itemEntry *entry;
	ItemCheck(itemID,type);

	entry = &itemArray[truncate(itemID)];

	entry->inuse 	= false;
	entry->ID 		= NULLITEM;
	entry->data	= NULL;
	entry->type	= "invalid";
}

//=============================================================================

void*
ItemGet(Item itemID, const char* type)
{
	ItemCheck(itemID,type);
	return(itemArray[truncate(itemID)].data);
}

//=============================================================================

void
ItemInit(void)
{	int i;
	for (i=0; i<MAXITEMS; i++) {
		itemArray[i].inuse 	= false;
		itemArray[i].ID 	= NULLITEM;
		itemArray[i].data 	= NULL;
		itemArray[i].type 	= "Never used";
	}
}

//=============================================================================

Item
ItemLookup(void* ptr)
{	int i;
	for (i=0; i<MAXITEMS; i++)
		if (ptr == itemArray[i].data && itemArray[i].inuse == true)
			return(i);
	assert(i<MAXITEMS);
	return(-1);	// should not be reached
}

//=============================================================================

void
ItemPrint(Item itemID)
{
	itemEntry *entry;

	entry = &itemArray[truncate(itemID)];

    printf("item: %ld	",itemID);
    printf("inuse: %d	",entry->inuse);
	printf("ID: %ld	",entry->ID);
 	printf("ptr: %p	",entry->data);
	printf("type: %s\n",entry->type);
}

//=============================================================================

#if TEST_ITEM

void
ItemTest(void)
{
	int i; char c; long l; Item d, e, f;

	ITEMTYPECREATE(intitem,int);
	ITEMTYPECREATE(charitem,char);
	ITEMTYPECREATE(longitem,long);

	d = ITEMCREATE(&i,int);
	e = ITEMCREATE(&c,char);
	f = ITEMCREATE(&l,long);

//	ITEMRETRIEVE(e,int);			wrong type

	ITEMDESTROY(d,int);
	ITEMDESTROY(e,char);
	ITEMDESTROY(f,long);

//	ITEMRETRIEVE(d,int);		deallocated item

}

#endif

//=============================================================================

void
ItemTerm(void)
{	int i;
	for (i=0; i<MAXITEMS; i++) {
		if (itemArray[i].ID > 0 && itemArray[i].inuse == true) {
			ItemPrint(itemArray[i].ID);
		}
	}
}

//=============================================================================
#endif // DO_VALIDATION
//=============================================================================
