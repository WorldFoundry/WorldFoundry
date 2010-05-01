@*============================================================================
@* objects.s: creates the enumerated list (objects.h) of all objects in veolcity
@*============================================================================
/*============================================================================*/
/* objects.c: created from objects.s, DO NOT MODIFY */
/*============================================================================*/

@define OBJECTSHEADER
@define OBJECTSFOOTER
@define OBJECTENTRY(name)	case Actor::@+name@+_KIND:@n			if(!(startData->objectData->oadFlags & (1<<OADFLAG_TEMPLATE_OBJECT))) @n				object = Oad@+name(startData); @n			break;
@define OBJECTONLYTEMPLATEENTRY(name)	case Actor::@+name@+_KIND:@n			object = NULL; @n			break;
@define OBJECTSUBENTRY(name,parent)	case Actor::@+name@+_KIND:@n			object = Oad@+parent(startData); @n			break;
@define OBJECTNOACTORENTRY(name)	case Actor::@+name@+_KIND:@n			object = NULL; @n			break;
@define COLTABLEHEADER
@define COLTABLEFOOTER
@define COLTABLEENTRY(obj1,obj2,msg1,msg2)

extern "C" {
#include "oas/oad.h"
}

/*============================================================================*/
/* note this routine may return NULL, if it does, it means there is no object to add (probably a template object) */

Actor*
ConstructOadObject(int32 type, const SObjectStartupData* startData)
{
	Actor* object = NULL;
	switch(type)
	 {
@include objects.mac
		default:

			DBSTREAM1( cerror << "object " << type << " doesn't exist" << std::endl; )
			assert(0);					// attempted to construct object not in list
			break;
	 };
	return(object);
}

@undef OBJECTONLYTEMPLATEENTRY
@define OBJECTONLYTEMPLATEENTRY(name)	case Actor::@+name@+_KIND:@n			object = Oad@+name(startData); @n			break;

@undef OBJECTENTRY
@define OBJECTENTRY(name)	case Actor::@+name@+_KIND:@n			object = Oad@+name(startData); @n			break;

/*============================================================================*/
/* note this routine will construct template objects (or even non-template objects) */

Actor*
ConstructTemplateObject(int32 type, const SObjectStartupData* startData)
{
	Actor* object = NULL;
	assert( ValidPtr( startData ) );
	startData->objectData->oadFlags |= 1<<OADFLAG_TEMPLATE_OBJECT;
	((SObjectStartupData *)startData)->roomNum = -1;

	switch(type)
	 {
@include objects.mac
		default:
			DBSTREAM1( cerror << "object " << type << " doesn't exist"; )
			assert(0);					// attempted to construct object not in list
			break;
	 };
	return(object);
}

/*============================================================================*/
