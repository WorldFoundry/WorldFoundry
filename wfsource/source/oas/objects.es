@*============================================================================*/
@* objects.es: creates the enumerated list (objects.e) of all objects in veolcity, suitable for inclusion from inside of an enumeration */
@*============================================================================*/
/*============================================================================*/
/* object.e: enumeration of all velocity game objects,  */
/*   created from  object.es,DO NOT MODIFY */
/*============================================================================*/

#ifndef _OBJECTS_HS
#define _OBJECTS_HS

@define OBJECTSHEADER 	NULL_KIND,
@define OBJECTSFOOTER
@define OBJECTENTRY(name) name@+_KIND,
@define OBJECTNOACTORENTRY(name) name@+_KIND,
@define OBJECTONLYTEMPLATEENTRY(name) name@+_KIND,
@define OBJECTSUBENTRY(name,parent) name@+_KIND,
@define COLTABLEHEADER
@define COLTABLEFOOTER
@define COLTABLEENTRY(obj1,obj2,msg1,msg2)

/*============================================================================*/

@include objects.mac

/*============================================================================*/
#endif
/*============================================================================*/
