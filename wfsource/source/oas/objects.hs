@*============================================================================*/
@* objects.hs: creates the enumerated list (objects.h) of all objects in veolcity */
@*============================================================================*/
/*============================================================================*/
/* object.h: enumeration of all velocity game objects,  */
/*   created from  object.hs,DO NOT MODIFY */
/*============================================================================*/

#ifndef _OBJECTS_HS
#define _OBJECTS_HS

@define OBJECTSHEADER enum EActorKind { @n	NULL_KIND,
@define OBJECTSFOOTER 	};
@define OBJECTENTRY(name) name@+_KIND,
@define OBJECTONLYTEMPLATEENTRY(name) name@+_KIND,
@define OBJECTNOACTORENTRY(name) name@+_KIND,
@define OBJECTSUBENTRY(name,parent) name@+_KIND,

/*============================================================================*/

@include objects.mac

/*============================================================================*/
#endif
/*============================================================================*/
