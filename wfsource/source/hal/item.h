//=============================================================================
// Item.h: public item interface
//=============================================================================
// use only once insurance

#ifndef _iTEM_H
#define _iTEM_H

//=============================================================================
// Documentation:

//	Abstract:
//			Shell of item interface
//	History:
//			Created	01-03-95 09:38am Kevin T. Seghetti
//			Modified 1-17 Joseph Boyle (in progress)

//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:  Type checking not working yet!

//	Example:
//=============================================================================

#define DYNAMICITEMS	  1	// type stored as string; checking is at runtime

#if DO_VALIDATION
typedef long Item;
#define NULLITEM 0
#define MAXITEMS 4096
// must be power of 2
#else
typedef void* Item;
#define NULLITEM NULL
#endif

//=============================================================================

#if DO_VALIDATION

Item 	GetEmptyItem(void);
Item 	ItemAlloc(void* ptr, const char* type);
void 	ItemPrint(Item itemID);
void 	ItemUseCheck(Item itemID);
void 	ItemCheck(Item itemID, const char* type);
void 	ItemFree(Item itemID, const char* type);
void* 	ItemGet(Item itemID, const char* type);
void	ItemInit(void);
Item	ItemLookup(void* ptr);
void 	ItemTest(void);
void 	ItemTerm(void);

#endif

//=============================================================================
// items are no longer just pointers (at debug time)

#if DO_VALIDATION

#if	DYNAMICITEMS
#define VALIDATEITEM(item) ItemUseCheck(item)
#else
#define VALIDATEITEM(item) ItemCheck(item)	// supply type arg?
#endif	// DYNAMICITEMS

#else
#define VALIDATEITEM(item)
#endif

//=============================================================================
// used to create items at compile time, must be invoked in global scope,
// probably in a header file

#if DO_VALIDATION
#if	DYNAMICITEMS
#define ITEMTYPECREATE(itemtype,datatype) typedef Item itemtype
#else
#define ITEMTYPECREATE(itemtype,datatype) typedef itemtype datatype ## _item_name
#endif	// DYNAMICITEMS

#else
#define ITEMTYPECREATE(itemtype,datatype) typedef datatype* itemtype
#endif	// DO_VALIDATION

//=============================================================================
// given an pointer to type "type", create an item to refer to it

#if DO_VALIDATION

#if	DYNAMICITEMS
#define ITEMCREATE(ptr,type) ItemAlloc((void*)ptr,# type)
#else
#define	ITEMCREATE(ptr,type) (type ## Item) ItemAlloc(ptr,# type)
#endif	// DYNAMICITEMS

#else
#define ITEMCREATE(ptr,type) (type*)(ptr)
#endif	// DO_VALIDATION

//============================================================================
// given an item of type "type", destroy it

#if DO_VALIDATION

#if	DYNAMICITEMS
#define ITEMDESTROY(item,type) ItemFree(item,# type)
#else
#define	ITEMDESTROY(item,type) ItemFree((Item)item,# type)
#endif	// DYNAMICITEMS

#else
#define ITEMDESTROY(item,type)
#endif	// DO_VALIDATION

//=============================================================================
// given an item of type "type", convert to pointer to type "type"

#if DO_VALIDATION

#if	DYNAMICITEMS
#define ITEMRETRIEVE(item,type) (type*)ItemGet(item,# type)
#else
#define	ITEMRETRIEVE(item,type) ItemGet((Item)item,# type)
#endif	// DYNAMICITEMS

#else
#define ITEMRETRIEVE(item,type) (type*)(item)
#endif	// DO_VALIDATION


//=============================================================================
// given a pointer to type "type", lookup and return its item
// kts this is a kludge, only used by parts of HAL until I get a better idea

#if DO_VALIDATION
#define ITEMLOOKUP(ptr,type) ItemLookup(ptr)
#else
#define ITEMLOOKUP(ptr,type) (type*)(ptr)
#endif

//=============================================================================
#endif
//=============================================================================
