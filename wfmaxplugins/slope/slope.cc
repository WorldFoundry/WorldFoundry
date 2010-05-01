// slope.cpp

#include "prim.h"
#include "iparamm.h"
#include "Simpobj.h"
#include "../lib/wf_id.hp"
#define WEDGE

class BoxObject : public GenBoxObject, public IParamArray
{
public:
	// Class vars
	static IParamMap* pmapTypeIn;
	static IParamMap* pmapParam;
	static IObjParam* ip;
	static Point3 crtPos;
	static float crtWidth, crtHeight, crtLength;

	BoxObject();
	
	// From BaseObject
	CreateMouseCallBack* GetCreateMouseCallBack();
	void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	TCHAR *GetObjectName() { return Slope_ClassName; }

	// Animatable methods
	void DeleteThis() { delete this; }
	Class_ID ClassID() { return Slope_ClassID; }
	
	// From ref
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	IOResult Load(ILoad *iload);

	// From IParamArray
	BOOL SetValue(int i, TimeValue t, int v);
	BOOL SetValue(int i, TimeValue t, float v);
	BOOL SetValue(int i, TimeValue t, Point3 &v);
	BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
	BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
	BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

	// From SimpleObject
	void BuildMesh( TimeValue t );
	BOOL OKtoDisplay( TimeValue t );
	void InvalidateUI();
	ParamDimension *GetParameterDim( int pbIndex );
	TSTR GetParameterName( int pbIndex );

	// From GenBoxObject
	void SetParams(float width, float height, float length, int wsegs,int lsegs, 
		int hsegs, BOOL genUV); 
};				



#define BMIN_LENGTH		float(0)
#define BMAX_LENGTH		float(1.0E30)
#define BMIN_WIDTH		float(0)
#define BMAX_WIDTH		float(1.0E30)
#define BMIN_HEIGHT		float(-1.0E30)
#define BMAX_HEIGHT		float(1.0E30)

//--- ClassDescriptor and class vars ---------------------------------

class BoxObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) {return new BoxObject;}
	const TCHAR *	ClassName() { return Slope_ClassName; }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Slope_ClassID; }
	const TCHAR* 	Category() { return _T( "Standard Primitives" ); }
	void			ResetClassParams(BOOL fileReset);
	};

static BoxObjClassDesc boxObjDesc;

ClassDesc*
GetBoxobjDesc()
{ 
	return &boxObjDesc;
}


// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for sphere class.
IObjParam *BoxObject::ip         = NULL;
IParamMap *BoxObject::pmapTypeIn = NULL;
IParamMap *BoxObject::pmapParam  = NULL;	
Point3 BoxObject::crtPos         = Point3(0,0,0);		
float BoxObject::crtWidth        = 0.0f; 
float BoxObject::crtHeight       = 0.0f;
float BoxObject::crtLength       = 0.0f;


void 
BoxObjClassDesc::ResetClassParams( BOOL fileReset )
{
	BoxObject::crtWidth   = 0.0f; 
	BoxObject::crtHeight  = 0.0f;
	BoxObject::crtLength  = 0.0f;
	BoxObject::crtPos     = Point3(0,0,0);
}

//--- Parameter map/block descriptors -------------------------------

// Parameter block indices ("Parameters")
enum {
	PB_LENGTH,
	PB_WIDTH,
	PB_HEIGHT,
};

// Non-parameter block indices (keyboard entry)
enum {
	PB_TI_POS,
	PB_TI_LENGTH,
	PB_TI_WIDTH,
	PB_TI_HEIGHT,
};


// Type in
static ParamUIDesc descTypeIn[] = {
	
	// Position
	ParamUIDesc(
		PB_TI_POS,
		EDITTYPE_UNIVERSE,
		IDC_TI_POSX,IDC_TI_POSXSPIN,
		IDC_TI_POSY,IDC_TI_POSYSPIN,
		IDC_TI_POSZ,IDC_TI_POSZSPIN,
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),
	
	// Length
	ParamUIDesc(
		PB_TI_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_LENGTHEDIT,IDC_LENSPINNER,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),
	
	// Width
	ParamUIDesc(
		PB_TI_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_WIDTHEDIT,IDC_WIDTHSPINNER,
		BMIN_WIDTH,BMAX_WIDTH,
		SPIN_AUTOSCALE),	

	// Height
	ParamUIDesc(
		PB_TI_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_HEIGHTEDIT,IDC_HEIGHTSPINNER,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	
	};
#define TYPEINDESC_LENGH 4


//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Length
	ParamUIDesc(
		PB_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_LENGTHEDIT,IDC_LENSPINNER,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),
	
	// Width
	ParamUIDesc(
		PB_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_WIDTHEDIT,IDC_WIDTHSPINNER,
		BMIN_WIDTH,BMAX_WIDTH,
		SPIN_AUTOSCALE),	

	// Height
	ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_HEIGHTEDIT,IDC_HEIGHTSPINNER,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	
	};
#define PARAMDESC_LENGH (3)


ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 }, 
	};

#define PBLOCK_LENGTH	(3)

#define CURRENT_VERSION	0
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);


//--- TypeInDlgProc --------------------------------

class BoxTypeInDlgProc : public ParamMapUserDlgProc 
{
public:
	BoxObject* ob;

	BoxTypeInDlgProc( BoxObject* o ) {ob=o;}
	BOOL DlgProc( TimeValue t, IParamMap* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {delete this;}
};


BOOL BoxTypeInDlgProc::DlgProc( TimeValue t, IParamMap* map,
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{
				case IDC_TI_CREATE: 
				{
					// We only want to set the value if the object is 
					// not in the scene.
					if ( ob->TestAFlag( A_OBJ_CREATING ) )
					{
						ob->pblock->SetValue( PB_LENGTH, 0, ob->crtLength );
						ob->pblock->SetValue( PB_WIDTH, 0, ob->crtWidth );
						ob->pblock->SetValue( PB_HEIGHT, 0, ob->crtHeight );
					}

					Matrix3 tm( 1 );
					tm.SetTrans( ob->crtPos );
					ob->suspendSnap = FALSE;
					ob->ip->NonMouseCreate( tm );
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;	
				}
			}
			break;	
		}

	return FALSE;
}


//--- Box methods -------------------------------


BoxObject::BoxObject()
{
	MakeRefByID( FOREVER, 0, CreateParameterBlock( descVer1, PBLOCK_LENGTH, CURRENT_VERSION ) );

	pblock->SetValue( PB_LENGTH, 0, crtLength );
	pblock->SetValue( PB_WIDTH, 0, crtWidth );
	pblock->SetValue( PB_HEIGHT, 0, crtHeight );
}


IOResult
BoxObject::Load( ILoad* iload )
{
	iload->RegisterPostLoadCallback( new ParamBlockPLCB(NULL,0,&curVersion,this,0));

	return IO_OK;
}


void 
BoxObject::BeginEditParams( IObjParam* ip, ULONG flags,Animatable* prev )
{
	SimpleObject::BeginEditParams( ip, flags, prev );
	this->ip = ip;

	if ( pmapParam )
	{
		// Left over from last Box ceated
		pmapTypeIn->SetParamBlock( this );
		pmapParam->SetParamBlock( pblock );
	}
	else
	{
		// Gotta make a new one
		if ( flags & BEGIN_EDIT_CREATE )
		{
			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_BOXPARAM3),
				"Keyboard Entry",
				APPENDROLL_CLOSED);
		}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_BOXPARAM2),
			"Parameters",
			0);
	}

	if ( pmapTypeIn )
	{	// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new BoxTypeInDlgProc(this));
	}
}
		

void
BoxObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	SimpleObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if ( flags & END_EDIT_REMOVEUI )
	{
		if (pmapTypeIn) DestroyCPParamMap(pmapTypeIn);
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		pmapTypeIn = NULL;
	}
}


void 
BoxObject::SetParams(float width, float height, float length, int wsegs,int lsegs, 
			int hsegs, BOOL genUV) 
{
	pblock->SetValue( PB_WIDTH, 0, width );
	pblock->SetValue( PB_HEIGHT, 0, height );
	pblock->SetValue( PB_LENGTH, 0, length );
} 


// vertices ( a b c d ) are in counter clockwise order when viewd from 
// outside the surface unless bias!=0 in which case they are clockwise
static void 
MakeQuad(int nverts, Face *f, int a, int b , int c , int d, int sg, int bias) 
{
	int sm = 1<<sg;
	assert(a<nverts);
	assert(b<nverts);
	assert(c<nverts);
	assert(d<nverts);
	if ( bias )
	{
		f[0].setVerts( b, a, c);
		f[0].setSmGroup(sm);
		f[0].setEdgeVisFlags(1,0,1);
		f[1].setVerts( d, c, a);
		f[1].setSmGroup(sm);
		f[1].setEdgeVisFlags(1,0,1);
	} 
	else
	{
		f[0].setVerts( a, b, c );
		f[0].setSmGroup( sm );
		f[0].setEdgeVisFlags( 1, 1, 0 );
		f[1].setVerts( c, d, a );
		f[1].setSmGroup( sm );
		f[1].setEdgeVisFlags( 1, 1, 0 );
	}
}


static void 
MakeTri( int nverts, Face *f, int a, int b, int c, int sg, int bias )
{
	int sm = 1<<sg;
	assert(a<nverts);
	assert(b<nverts);
	assert(c<nverts);
	if ( bias )
	{
		f[0].setVerts( b, a, c );
		f[0].setSmGroup( sm );
		f[0].setEdgeVisFlags( 1, 0, 1 );
	} 
	else
	{
		f[0].setVerts( a, b, c );
		f[0].setSmGroup( sm );
		f[0].setEdgeVisFlags( 1, 1, 0 );
	}
}


#define MAKE_QUAD(na,nb,nc,nd,sm,b)	\
do \
{ \
	MakeQuad( nverts, &(mesh.faces[nf]), na, nb, nc, nd, sm, b ); \
	nf += 2; \
} \
while ( 0 )

#define MAKE_TRI(na,nb,nc,sm,b) \
do \
{ \
	MakeTri( nverts, &(mesh.faces[nf]), na, nb, nc, sm, b ); \
	++nf; \
} \
while ( 0 )


void 
BoxObject::BuildMesh( TimeValue t )
{
	int nverts;
	int nv;
	int nfaces;
	Point3 va,vb,p;
	float l, w, h;
	BOOL bias = 0;

	// Start the validity interval at forever and widdle it down
	ivalid = FOREVER;	
	pblock->GetValue(PB_LENGTH,t,l,ivalid);
	pblock->GetValue(PB_WIDTH,t,w,ivalid);
	pblock->GetValue(PB_HEIGHT,t,h,ivalid);
	if (h<0.0f) bias = 1;

	nverts = 6;
#if defined( WEDGE )
	nfaces = 8;
#else
	nfaces = 2;
#endif

	mesh.setNumVerts( nverts );
	mesh.setNumFaces( nfaces );

	nv = 0;
	
	vb =  Point3(w,l,h)/float(2);   
	va = -vb;

	va.z = float(0);
	vb.z = h;

	int nf = 0;

	{	// do bottom vertices
	p.z = va.z;

	p.y = va.y;
	p.x = va.x;		mesh.setVert( nv++, p );
	p.x += w;		mesh.setVert( nv++, p );

	p.y += l;
	p.x = va.x;		mesh.setVert( nv++, p );
	p.x += w;		mesh.setVert( nv++, p );

	// do top vertices
	p.z = vb.z;
	p.y = va.y;
	p.x = va.x;		mesh.setVert( nv++, p );
	p.x += w;		mesh.setVert( nv++, p );
	}

#if defined( WEDGE )
	MAKE_QUAD( 0, 2, 3, 1, 1, bias );		// bottom face
#endif
	MAKE_QUAD( 4, 5, 3, 2, 2, bias );		// top face
#if defined( WEDGE )
	MAKE_QUAD( 5, 4, 0, 1, 3, bias );		// back face
	MAKE_TRI( 2, 0, 4, 4, bias );			// side
	MAKE_TRI( 1, 3, 5, 5, bias );			// side
#endif

	mesh.setNumTVerts( 0 );
	mesh.setNumTVFaces( 0 );

	mesh.buildNormals();

	mesh.InvalidateGeomCache();
}


class BoxObjCreateCallBack: public CreateMouseCallBack 
{
	BoxObject* ob;
	Point3 p0,p1;
	IPoint2 sp0, sp1;
	BOOL square;
public:
	int proc( ViewExp* vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
	void SetObj(BoxObject *obj) { ob = obj; }
};


int 
BoxObjCreateCallBack::proc( ViewExp* vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) 
{
	Point3 d;
	if ( msg == MOUSE_POINT || msg == MOUSE_MOVE ) 
	{
		switch ( point )
		{
			case 0:
			{
				sp0 = m;
				ob->pblock->SetValue(PB_WIDTH,0,0.0f);
				ob->pblock->SetValue(PB_LENGTH,0,0.0f);
				ob->pblock->SetValue(PB_HEIGHT,0,0.0f);
				ob->suspendSnap = TRUE;								
				p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				p1 = p0 + Point3(.01,.01,.01);
				mat.SetTrans(float(.5)*(p0+p1));				
				Point3 xyz = mat.GetTrans();
				xyz.z = p0.z;
				mat.SetTrans(xyz);
				break;
			}

			case 1:
			{
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				p1.z = p0.z +(float).01; 
				if ( flags & MOUSE_CTRL ) 
				{
					mat.SetTrans(p0);
				} 
				else 
				{
					mat.SetTrans(float(.5)*(p0+p1));
					Point3 xyz = mat.GetTrans();
					xyz.z = p0.z;
					mat.SetTrans(xyz);					
				}
				d = p1-p0;
				
				square = FALSE;
				if (flags&MOUSE_CTRL) 
				{	// Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;
					square = TRUE;
				}

				ob->pblock->SetValue(PB_WIDTH,0,float(fabs(d.x)));
				ob->pblock->SetValue(PB_LENGTH,0,float(fabs(d.y)));
				ob->pblock->SetValue(PB_HEIGHT,0,float(fabs(d.z)));
				ob->pmapParam->Invalidate();										

				if ( msg==MOUSE_POINT && (Length(sp1-sp0)<3 || Length(d)<0.1f) )
				{
					return CREATE_ABORT;
				}
				break;
			}

			case 2:
				p1.z = p0.z + vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,0,1),sp1,m));
				if ( !square ) 
				{
					mat.SetTrans(float(.5)*(p0+p1));
					mat.SetTrans(2,p0.z); // set the Z component of translation
				}

				d = p1-p0;
				if (square) {
					// Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;					
					}

				ob->pblock->SetValue(PB_WIDTH,0,float(fabs(d.x)));
				ob->pblock->SetValue(PB_LENGTH,0,float(fabs(d.y)));
				ob->pblock->SetValue(PB_HEIGHT,0,float(d.z));
				ob->pmapParam->Invalidate();				
					
				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;					
					return CREATE_STOP;
					}
				break;
			}
		}
	else
	if ( msg == MOUSE_ABORT ) 
	{
		return CREATE_ABORT;
	}

	return TRUE;
}


static BoxObjCreateCallBack boxCreateCB;

CreateMouseCallBack* 
BoxObject::GetCreateMouseCallBack() 
{
	boxCreateCB.SetObj( this );

	return &boxCreateCB;
}


BOOL 
BoxObject::OKtoDisplay( TimeValue t )
{
	return TRUE;
}


// From ParamArray
BOOL 
BoxObject::SetValue( int i, TimeValue t, int v )
{
	return TRUE;
}


BOOL 
BoxObject::SetValue(int i, TimeValue t, float v)
{
	switch ( i ) 
	{
		case PB_TI_LENGTH: crtLength = v; break;
		case PB_TI_WIDTH:  crtWidth = v; break;
		case PB_TI_HEIGHT: crtHeight = v; break;
	}	

	return TRUE;
}


BOOL 
BoxObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch ( i ) 
	{
		case PB_TI_POS: crtPos = v; break;
	}		

	return TRUE;
	}


BOOL 
BoxObject::GetValue( int i, TimeValue t, int& v, Interval& ivalid )
{
	return TRUE;
}


BOOL 
BoxObject::GetValue( int i, TimeValue t, float& v, Interval& ivalid ) 
{	
	switch ( i ) 
	{
		case PB_TI_LENGTH: v = crtLength; break;
		case PB_TI_WIDTH:  v = crtWidth; break;
		case PB_TI_HEIGHT: v = crtHeight; break;
	}

	return TRUE;
}


BOOL 
BoxObject::GetValue( int i, TimeValue t, Point3& v, Interval& ivalid )
{	
	switch ( i ) 
	{
		case PB_TI_POS: v = crtPos; break;		
	}
	
	return TRUE;
}


void 
BoxObject::InvalidateUI() 
{
	if ( pmapParam ) 
		pmapParam->Invalidate();
}


ParamDimension*
BoxObject::GetParameterDim( int pbIndex )
{
	switch ( pbIndex )
	{
		case PB_LENGTH:return stdWorldDim;
		case PB_WIDTH: return stdWorldDim;
		case PB_HEIGHT:return stdWorldDim;
		default: return defaultDim;
	}
}


TSTR 
BoxObject::GetParameterName( int pbIndex )
{
	switch ( pbIndex )
	{
		case PB_LENGTH: return TSTR( "Length" );
		case PB_WIDTH:  return TSTR( "Width" );
		case PB_HEIGHT: return TSTR( "Height" );
		default: return TSTR(_T(""));
	}
}


RefTargetHandle 
BoxObject::Clone( RemapDir& remap )
{
	BoxObject* newob = new BoxObject();
	if ( newob )
	{
		newob->ReplaceReference( 0, pblock->Clone( remap ) );
		newob->ivalid.SetEmpty();
	}
	
	return newob;
}
