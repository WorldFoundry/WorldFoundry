@*============================================================================*/
@* objects.cts: creates objects.ctb, collision array table
@*============================================================================*/
//============================================================================*/
// objects.ctb: collision array table, included by room.cc
// created from  object.cts,DO NOT MODIFY */
//============================================================================*/

@define OBJECTSHEADER
@define OBJECTSFOOTER
@define OBJECTENTRY(name,collide)
@define OBJECTONLYTEMPLATEENTRY(name,collide)
@define OBJECTNOACTORENTRY(name,collide)
@define OBJECTSUBENTRY(name,parent,collide)
@define COLTABLEHEADER
@define COLTABLEFOOTER
@define COLTABLEENTRY(obj1,obj2,msg1,msg2) obj1,obj2,msg1,msg2

//============================================================================

@include objects.mac

//============================================================================
