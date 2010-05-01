///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//	scene.cpp                                                                //
//                                                                           //
//	Classes related to scene enumeration for 3DS MAX plugins                 //
//                                                                           //
//	Written 1/9/97 by Phil Torre                                             //
//	(Borrowed largely from the 3ds exporter sample code)					 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include "scene.hpp"

//--------------------------------------------------------------


SceneEntry::SceneEntry(INode *n, Object *o, int t)
{
	node = n; obj = o; type = t; next = NULL;
	tnode = n->GetTarget();
}

//--------------------------------------------------------------

SceneEnumProc::SceneEnumProc(IScene *scene, TimeValue t, Interface *i)
{
	time = t;
	theScene = scene;
	count = 0;
	head = tail = NULL;
	this->i = i;
	theScene->EnumTree(this);
}

SceneEnumProc::~SceneEnumProc()
{
	while(head)
	{
		SceneEntry *next = head->next;
		delete head;
		head = next;
	}
	head = tail = NULL;
	count = 0;
}

int
SceneEnumProc::callback(INode *node)
{
	Object *obj = node->EvalWorldState(time).obj;

//	// List objects which can convert themselves to TriObjects, and which are
//	// currently selected
//	if ( (obj->CanConvertToType(triObjectClassID)) &&
//		 (node->Selected()) )
//	{
//		Append(node, obj, OBTYPE_MESH);
//	}

	// for now, use all objects in the scene
	Append(node, obj, OBTYPE_MESH);

#ifdef DISABLED

	if (node->IsTarget())
	{
		INode* ln = node->GetLookatNode();
		if (ln)
		{
			Object *lobj = ln->EvalWorldState(time).obj;
			switch(lobj->SuperClassID())
			{
				case LIGHT_CLASS_ID:  Append(node, obj, OBTYPE_LTARGET); break;
				case CAMERA_CLASS_ID: Append(node, obj, OBTYPE_CTARGET); break;
			}
		}
		return TREE_CONTINUE;
	}

	switch (obj->SuperClassID())
	{
		case HELPER_CLASS_ID:
			if ( obj->ClassID()==Class_ID(DUMMY_CLASS_ID,0))
				Append(node, obj, OBTYPE_DUMMY);
			break;
		case LIGHT_CLASS_ID:
			if (obj->ClassID()==Class_ID(OMNI_LIGHT_CLASS_ID,0))
				Append(node, obj, OBTYPE_OMNILIGHT);
			else
			if (obj->ClassID()==Class_ID(DIR_LIGHT_CLASS_ID,0))
				Append(node, obj, OBTYPE_DIRLIGHT);
		//export DIR_LIGHT and FSPOT_LIGHT????
			break;
		case CAMERA_CLASS_ID:
			if (obj->ClassID()==Class_ID(LOOKAT_CAM_CLASS_ID,0))
				Append(node, obj, OBTYPE_CAMERA);
			break;
	}

#endif // DISABLED

	return TREE_CONTINUE;	// Keep on enumeratin'!
}


void
SceneEnumProc::Append(INode *node, Object *obj, int type)
{
	SceneEntry *entry = new SceneEntry(node, obj, type);

	if(tail)
		tail->next = entry;
	tail = entry;
	if(!head)
		head = entry;
	count++;
}

Box3
SceneEnumProc::Bound()
{
	Box3 bound;
	bound.Init();
	SceneEntry *e = head;
	ViewExp *vpt = i->GetViewport(NULL);
	while(e)
	{
		Box3 bb;
		e->obj->GetWorldBoundBox(time, e->node, vpt, bb);
		bound += bb;
		e = e->next;
	}
	return bound;
}

SceneEntry*
SceneEnumProc::Find(INode *node)
{
	SceneEntry *e = head;
	while(e)
	{
		if(e->node == node)
			return e;
		e = e->next;
	}
	return NULL;
}

SceneEntry*
SceneEnumProc::operator[](int index)
{
	if (index > (count - 1))
		return NULL;

	SceneEntry* ptr = head;
	while (index > 0)
	{
		ptr = ptr->next;
		assert(ptr);
		index--;
	}

	return ptr;
}
