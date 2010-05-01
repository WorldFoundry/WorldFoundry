///////////////////////////////////////////////////////////////////////////////
//	vrmlconv.hpp	IffConversion class from 3ds2vrml						 //
//																			 //
//	Created 01/03/97 by Phil Torre											 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _VRMLCONV_HPP_
#define _VRMLCONV_HPP_

#include "global.hpp"

#define szWorldFoundryGDK szRegWorldFoundryGDK	//"Software\\World Foundry\\GDK"
#define szRegMax2IFF szRegWorldFoundryGDK "\\max2iff"

//#include "textvert.hpp"
class SceneEntry;
class SceneEnumProc;
class _IffWriter;

#if MAX_RELEASE < 2000
typedef Point3 VertColor;
#endif

// This is a pair of <MAX vertex index><MAX UV index>, used to build our face list
struct XYZUV
{
	int vertexIdx;
	int uvIdx;
	VertColor _vc;

	bool operator < (const XYZUV& him) const { return false; }
	bool operator == (const XYZUV& him) const
	{
		return ( (vertexIdx == him.vertexIdx) && (uvIdx == him.uvIdx) && (_vc == him._vc) );
	}
};

// The array of vertex indices index into OUR list, not the one MAX keeps!
struct RemappedFace
{
	int v[3];
	int matIdx;

	bool operator < (const RemappedFace& him) const { return false; }
	bool operator == (const RemappedFace& him) const { return ((v[0]==him.v[0]) && (v[1]==him.v[1]) && (v[2]==him.v[2])); }
};


class IffConversion
{
public:
	IffConversion( INode* pNode, _IffWriter* iff, const max2iffOptions* options );
	~IffConversion();
	void OpenInputFile( const char* iname );

protected:
	void writeMesh();
	void writeMeshFrame( int, _IffWriter* );
	void writeGeometry();
	void writeMaterials();
	void writeAnimation();

	bool objectIsClass( const char* szClass ) const;
	void writeHandles();
	Mesh* CreateGroupMesh();

	Mesh* mesh;
	vector<XYZUV>			_pointList;
	vector<RemappedFace>	_faceList;

	HWND _hwndMax;

	INode* _pNode;
	TimeValue _theTime;
//	ofstream* ofile;
	_IffWriter* _iff;
	_IffWriter* _iffGeometry;
	_IffWriter* _iffAnimation;
	_IffWriter* _iffHandle;
	_IffWriter* _iffMaterial;
	_IffWriter* _iffEvent;

	max2iffOptions _options;
};

// This is a dummy View subclass for when calling Interface methods that require a View pointer
class NullView: public View
{
public:
	Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
	NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
};


#endif	// _VRMLCONV_HPP_
