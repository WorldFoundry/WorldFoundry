///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//	scene.hpp                                                                //
//                                                                           //
//	Classes related to scene enumeration for 3DS MAX plugins                 //
//                                                                           //
//	Written 1/9/97 by Phil Torre                                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#ifndef _SCENE_HPP_
#define _SCENE_HPP_

#include "global.hpp"


//--------------------------------------------------------------

#define OBTYPE_MESH 0
#define OBTYPE_CAMERA 1
#define OBTYPE_OMNILIGHT 2
#define OBTYPE_DIRLIGHT 3
#define OBTYPE_DUMMY 5
#define OBTYPE_CTARGET 6
#define OBTYPE_LTARGET 7

class SceneEntry
{
public:
	TSTR name;
	INode *node,*tnode;
	Object *obj;
	int type;		// See above
	int id;
	SceneEntry *next;
	SceneEntry(INode *n, Object *o, int t);
	void SetID(int id) { this->id = id; }
};

//- Material Export -------------------------------------------------------------

//struct MEntry { SMtl *sm; Mtl *m; };
//
//class MeshMtlList: public Tab<MEntry>
//{
//public:
//	void AddMtl(Mtl *m);
//	void ReallyAddMtl(Mtl *m);
//	int FindMtl(Mtl *m);
//	int FindSName(char *nam);
//	~MeshMtlList()
//	{
//		for (int i=0; i<Count(); i++)
//		{
//			FreeMatRefs((*this)[i].sm);
//			delete (*this)[i].sm;
//		}
//	}
//};

//--------------------------------------------------------------

class SceneEnumProc : public ITreeEnumProc
{
public:
	Interface	*i;
	SceneEntry *head;
	SceneEntry *tail;
	IScene		*theScene;
	int			count;
//	MeshMtlList *mtlList;
	TimeValue	time;
//				SceneEnumProc(IScene *scene, TimeValue t, Interface *i, MeshMtlList *ml);
				SceneEnumProc(IScene *scene, TimeValue t, Interface *i);
				~SceneEnumProc();
	int			Count() { return count; }
	void		Append(INode *node, Object *obj, int type);
	int			callback( INode *node );
	Box3		Bound();
	SceneEntry *Find(INode *node);
	SceneEntry *operator[](int index);
	void BuildNames();
};

//--------------------------------------------------------------

// This is a dummy View subclass for when calling Interface methods that require a View pointer
class NullView: public View
{
public:
	Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
	NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
};

//--------------------------------------------------------------

#endif  // _SCENE_HPP_
