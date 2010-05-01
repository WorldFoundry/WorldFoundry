// object.h
#ifndef OBJECT_H
#define OBJECT_H

#include <iostream.h>
class Item;

class Object
{
public:
	Object( const char* );
	~Object();

	const char* GetName() const;

	Item* _itemObject;
	Item* _itemGeneral;
	Item* _itemOad;
	Item* _itemMailboxes;

	friend ostream& operator << ( ostream&, const Object& );

protected:
	const char* _szObjectName;

private:
	Object();
};


#endif	// OBJECT_H
