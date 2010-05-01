//==============================================================================
// cyterrain.cc:
// Copyright (c) 1998 Recombinant Limited  All Rights Reserved.
//
// This is UNPUBLISHED PROPRIETARY SOURCE CODE of Recombinant Limited
// The contents of this file may not be disclosed to third parties, copied
// or duplicated in any form, in whole or in part, without the prior written
// permission of Recombinant Limited
//==============================================================================
// Description: 3ds MAX Terrain primitive for Cyclone Studios
// Original Author: Kevin T. Seghetti
//	Based on 3dsmax sample code by Rolf Berteig
// most of the work was does in BuildMesh
//==============================================================================

#include "terrain.hpp"
#include "standard.hpp"
#include "terraindata.inc"

//=============================================================================
// always rounds down

long
RestrictToPowerOf2(long value)
{
	if(value < 0)
	{
		static recurseCount=0;
		assert(recurseCount == 0);
		recurseCount++;
		int retVal = -RestrictToPowerOf2(-value);
		recurseCount--;
		return retVal;
	}
	else
	{
		assert (value >=0);
		int count=0;
		// loop through all values of powers of 2 (assume can never be larger than MAX_SIZE
		for(long power=1;power < MAX_SIZE; )
		{
			if(value <= power)
				return power;
			assert(count++ < 33);

			power = power << 1;
		}
	}
	assert(0);
	return(0);
}

//=============================================================================
// --- Methods of CycloneTerrainObject ---

CycloneTerrainObject::CycloneTerrainObject()
{
	// Create the parameter block and make a reference to it.
	SetAFlag(A_PLUGIN1);
	MakeRefByID(FOREVER, 0, CreateParameterBlock(paramBlockDescID, PARAMDESC_ENTRIES, CURRENT_VERSION));
	assert(pblock);

	for(int index=0;index<PARAMETER_ENTRIES;index++)
	{
		cycloneTerrainParameterEntryArray[index]._entry.SetValue(pblock,index,0);
	}
}

//=============================================================================
// Class vars
// Initialize the class vars

int CycloneTerrainObject::dlgWidthSegments;
int CycloneTerrainObject::dlgLengthSegments;
int CycloneTerrainObject::dlgSmooth;
int CycloneTerrainObject::dlgFixedSegmentSize;
int CycloneTerrainObject::dlgRestrictToPowerOf2;
IParamMap *CycloneTerrainObject::pmapParam  = NULL;
IParamMap *CycloneTerrainObject::pmapTypeIn = NULL;
IObjParam *CycloneTerrainObject::ip         = NULL;
Point3 CycloneTerrainObject::crtPos;
float CycloneTerrainObject::crtLength;	// creation width
float CycloneTerrainObject::crtWidth;

//=============================================================================
// TypeInDlgProc
// This is the method called when the user clicks on the Create button
// in the Keyboard Entry rollup.  It was registered as the dialog proc
// for this button by the SetUserDlgProc() method called from
// BeginEditParams().

class CycloneTerrainTypeInDlgProc : public ParamMapUserDlgProc
{
	public:
		CycloneTerrainObject *so;
		CycloneTerrainTypeInDlgProc(CycloneTerrainObject *s) {so=s;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
};

//-----------------------------------------------------------------------------

BOOL
CycloneTerrainTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_TI_CREATE:
				{
					// verify valid size
					if (so->crtLength==0.0 || so->crtWidth==0.0)
						return TRUE;

					// We only want to set the value if the object is
					// not in the scene.
					if (so->TestAFlag(A_OBJ_CREATING))
					{
						int restrictToPowerOf2;
						so->pblock->GetValue(PB_RESTRICTTOPOWEROF2, 0, restrictToPowerOf2, FOREVER);
						if(restrictToPowerOf2)
						{
							so->crtWidth = float(RestrictToPowerOf2(int(so->crtWidth*65536.0)))/65536.0;
							so->crtLength = float(RestrictToPowerOf2(int(so->crtLength*65536.0)))/65536.0;
						}

						so->pblock->SetValue(PB_WIDTH,0,so->crtWidth);
						so->pblock->SetValue(PB_LENGTH,0,so->crtLength);

						int fixedSegmentSize;
						so->pblock->GetValue(PB_FIXEDSEGMENTSIZE, 0, fixedSegmentSize, FOREVER);
						if(fixedSegmentSize)
						{
							so->dlgWidthSegments = int(so->crtWidth);
							so->dlgLengthSegments = int(so->crtLength);

							so->pblock->SetValue(PB_WIDTHSEGS,0,int(so->crtWidth));
							so->pblock->SetValue(PB_LENGTHSEGS,0,int(so->crtLength));
						}
					}

					Matrix3 tm(1);
					tm.SetTrans(so->crtPos);
					so->suspendSnap = FALSE;
					so->ip->NonMouseCreate(tm);
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;
				}
			}
			break;
	}
	return FALSE;
}

//=============================================================================
// Class CycloneTerrainObjectCreateCallBack
// Declare a class derived from CreateMouseCallBack to handle
// the user input during the creation phase of the terrain object.

class CycloneTerrainObjectCreateCallBack : public CreateMouseCallBack
{
	IPoint2 sp0;
	CycloneTerrainObject *ob;
	Point3 p0;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		void SetObj(CycloneTerrainObject *obj) {ob = obj;}
};

//=============================================================================
// --- Inherited virtual methods of CreateMouseCallBack ---
// This is the method that actually handles the user input
// during the object creation.

int
CycloneTerrainObjectCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat )
{
	Point3 p1,center;

	#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
		#ifdef _3D_CREATE
			vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		#else
			vpt->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
		#endif
	}
	#endif

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE)
	{
		switch(point)
		{
			case 0:  // only happens with MOUSE_POINT msg
				assert(msg==MOUSE_POINT);
				ob->pblock->SetValue(PB_LENGTH,0,0.0f);
				ob->pblock->SetValue(PB_WIDTH,0,0.0f);
				ob->suspendSnap = TRUE;
				sp0 = m;
				#ifdef _3D_CREATE
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				mat.SetTrans(p0);
				break;
			case 1:
				mat.IdentityMatrix();
				//mat.PreRotateZ(HALFPI);
				#ifdef _3D_CREATE
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				Point3 d = p1-p0;
				d.y = -d.y;

				int restrictToPowerOf2;
				ob->pblock->GetValue(PB_RESTRICTTOPOWEROF2, 0, restrictToPowerOf2, FOREVER);
				if(restrictToPowerOf2)
				{
					d.x = float(RestrictToPowerOf2(int(d.x*65536.0)))/65536.0;
					d.y = float(RestrictToPowerOf2(int(d.y*65536.0)))/65536.0;
				}

				Point3 upperLeft = p0;
				if(d.x < 0)
					upperLeft.x = p0.x+d.x;
				if(d.y < 0)
					upperLeft.y = p0.y-d.y;

				mat.SetTrans(upperLeft);
				float width = fabs(d.x);
				float length = fabs(d.y);

				ob->pblock->SetValue(PB_LENGTH,0,length);
				ob->pblock->SetValue(PB_WIDTH,0,width);

				int fixedSegmentSize;
				ob->pblock->GetValue(PB_FIXEDSEGMENTSIZE, 0, fixedSegmentSize, FOREVER);
				if(fixedSegmentSize)
				{
					ob->dlgWidthSegments = int(width);
					ob->dlgLengthSegments = int(length);

					ob->pblock->SetValue(PB_WIDTHSEGS,0,int(width));
					ob->pblock->SetValue(PB_LENGTHSEGS,0,int(length));
				}

				ob->pmapParam->Invalidate();

				if (flags&MOUSE_CTRL)
				{
					float ang = (float)atan2(p1.y-p0.y,p1.x-p0.x);
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
				}

				if (msg==MOUSE_POINT)
				{
					ob->suspendSnap = FALSE;
					return (Length(m-sp0)<3 || Length(p1-p0)<0.1f)?CREATE_ABORT:CREATE_STOP;
				}
				break;
		}
	}
	else
	if (msg == MOUSE_ABORT)
	{
		return CREATE_ABORT;
	}
	return TRUE;
}

//=============================================================================

// A single instance of the callback object.
static CycloneTerrainObjectCreateCallBack cycloneTerrainCreateCB;

//=============================================================================
// --- Inherited virtual methods of Animatable ---
// This method is called by the system when the user needs
// to edit the objects parameters in the command panel.
// it creates each roll-up the first time through, and re-uses them after that

void
CycloneTerrainObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	// We subclass off SimpleObject so we must call its
	// BeginEditParams() method first.
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapParam)
	{
		// Left over from last terrain ceated
		pmapTypeIn->SetParamBlock(this);
		pmapParam->SetParamBlock(pblock);
	}
	else
	{
		// Gotta make a new one.
		if (flags&BEGIN_EDIT_CREATE)
		{

			// Here we create each new rollup page in the command panel
			// using our descriptors.
			pmapTypeIn = CreateCPParamMap(
				descTypeIn,PARAMDESC_KEYBOARD_ENTRIES,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_KEYBOARDENTRY),
				"Keyboard Entry",
				APPENDROLL_CLOSED);
		}

		pmapParam = CreateCPParamMap(
			paramUIDesc,PARAMDESC_ENTRIES,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_PARAMETERS),
			"Parameters",
			0);
		}

	if(pmapTypeIn)
	{
		// A callback for the type in.
		// This handles processing the Create button in the
		// Keyboard Entry rollup page.
		pmapTypeIn->SetUserDlgProc(new CycloneTerrainTypeInDlgProc(this));
	}
}

//=============================================================================
// This is called by the system to terminate the editing of the
// parameters in the command panel.

void
CycloneTerrainObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	SimpleObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if (flags & END_EDIT_REMOVEUI )
	{
		// Remove the rollup pages from the command panel.
		if (pmapTypeIn)
			DestroyCPParamMap(pmapTypeIn);
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		pmapTypeIn = NULL;
	}

	// Save these values in class variables so the next object created
	// will inherit them.
	pblock->GetValue(PB_WIDTHSEGS,ip->GetTime(),dlgWidthSegments,FOREVER);
	pblock->GetValue(PB_LENGTHSEGS,ip->GetTime(),dlgLengthSegments,FOREVER);
	pblock->GetValue(PB_SMOOTH,ip->GetTime(),dlgSmooth,FOREVER);
	pblock->GetValue(PB_RESTRICTTOPOWEROF2,ip->GetTime(),dlgRestrictToPowerOf2,FOREVER);
	pblock->GetValue(PB_FIXEDSEGMENTSIZE,ip->GetTime(),dlgFixedSegmentSize,FOREVER);
}

//=============================================================================
// --- Inherited virtual methods of ReferenceMaker ---
#define NEWMAP_CHUNKID	0x0100

// Called by MAX when the terrain object is loaded from disk.

IOResult
CycloneTerrainObject::Load(ILoad *iload)
{
	ClearAFlag(A_PLUGIN1);

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch (iload->CurChunkID())
		{
			case NEWMAP_CHUNKID:
				SetAFlag(A_PLUGIN1);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK)
			return res;
	}

	// This is the callback that corrects for any older versions
	// of the parameter block structure found in the MAX file
	// being loaded.
#if 0
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
#endif
	return IO_OK;
}

//=============================================================================

IOResult
CycloneTerrainObject::Save(ISave *isave)
{
	if (TestAFlag(A_PLUGIN1))
	{
		isave->BeginChunk(NEWMAP_CHUNKID);
		isave->EndChunk();
	}
 	return IO_OK;
}

//=============================================================================
// --- Inherited virtual methods of ReferenceTarget ---

RefTargetHandle
CycloneTerrainObject::Clone(RemapDir& remap)
{
	CycloneTerrainObject* newob = new CycloneTerrainObject();
	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->ivalid.SetEmpty();
	return(newob);
}

//=============================================================================
// --- Inherited virtual methods of BaseObject ---

CreateMouseCallBack*
CycloneTerrainObject::GetCreateMouseCallBack()
{
	cycloneTerrainCreateCB.SetObj(this);
	return(&cycloneTerrainCreateCB);
}

//=============================================================================
// --- Inherited virtual methods of Object ---

Object* CycloneTerrainObject::ConvertToType(TimeValue t, Class_ID obtype)
{
#if 0
	if (obtype == patchObjectClassID)
	{
		Interval valid = FOREVER;
//		float radius;
		int smooth;
		int genUVs;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_SMOOTH,t,smooth,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		PatchObject *ob = new PatchObject();
		BuildSpherePatch(ob->patch,radius,smooth,genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	}
	else  if (obtype == EDITABLE_SURF_CLASS_ID)
	{
		Interval valid = FOREVER;
		float radius, hemi;
		int recenter, genUVs;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_HEMI,t,hemi,valid);
		pblock->GetValue(PB_RECENTER,t,recenter,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		Object *ob = BuildNURBSSphere(radius, hemi, recenter,genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;

	}
	else
#endif
	{
		return SimpleObject::ConvertToType(t,obtype);
	}
}

//=============================================================================

int CycloneTerrainObject::CanConvertToType(Class_ID obtype)
{
#if 0
	if (obtype==patchObjectClassID || obtype==defObjectClassID ||
		obtype==triObjectClassID || obtype==EDITABLE_SURF_CLASS_ID)
	{
		return 1;
	}
	else
#endif
	{
		return SimpleObject::CanConvertToType(obtype);
	}
}

//=============================================================================

void
CycloneTerrainObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
#if 0
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1,&id);
    nlist.Append(1,&name);
#endif
}

//=============================================================================

BOOL CycloneTerrainObject::HasUVW()
{
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs;
}

//=============================================================================

void CycloneTerrainObject::SetGenUVW(BOOL sw)
{
	if (sw==HasUVW())
		return;
	pblock->SetValue(PB_GENUVS,0, sw);
}

//=============================================================================
// --- Inherited virtual methods of SimpleObject ---
// Return TRUE if it is OK to display the mesh at the time requested,
// return FALSE otherwise.

BOOL
CycloneTerrainObject::OKtoDisplay(TimeValue t)
{
	float length,width;
	pblock->GetValue(PB_LENGTH,t,length,FOREVER);
	pblock->GetValue(PB_WIDTH,t,width,FOREVER);
	if (length==0.0f || width==0.0f)
		return FALSE;
	else
		return TRUE;
}

//=============================================================================
// This method is called when the user interface controls need to be
// updated to reflect new values because of the user moving the time
// slider.  Here we simply call a method of the parameter map to
// handle this for us.

void
CycloneTerrainObject::InvalidateUI()
{
	if (pmapParam)
		pmapParam->Invalidate();
}

//=============================================================================
// Builds the mesh representation for the terrain based on the
// state of it's parameters at the time requested.
//=============================================================================

void
CycloneTerrainObject::BuildMesh(TimeValue t)
{
	Point3 p;
	int nf=0,nv=0;
	int widthSegs;
	int lengthSegs;
	int smooth;
	int restrictToPowerOf2=0;
	float length;
	float width;
	int multiMaterial;
	BOOL genUVs = TRUE;

//	if (TestAFlag(A_PLUGIN1))
//		startAng = HALFPI;

	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	pblock->GetValue(PB_LENGTH, t, length, ivalid);
	pblock->GetValue(PB_WIDTH, t, width, ivalid);
	pblock->GetValue(PB_WIDTHSEGS, t, widthSegs, ivalid);
	pblock->GetValue(PB_LENGTHSEGS, t, lengthSegs, ivalid);
	pblock->GetValue(PB_SMOOTH, t, smooth, ivalid);
	pblock->GetValue(PB_RESTRICTTOPOWEROF2, t, restrictToPowerOf2, ivalid);
	pblock->GetValue(PB_MULTI_MATERIAL, t, multiMaterial, ivalid);
	pblock->GetValue(PB_GENUVS, t, genUVs, ivalid);
	LimitValue(widthSegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(lengthSegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(smooth, MIN_SMOOTH, MAX_SMOOTH);
	LimitValue(length, MIN_SIZE, MAX_SIZE);
	LimitValue(width, MIN_SIZE, MAX_SIZE);

	length = -length;

	if(restrictToPowerOf2)
	{
		length = float(RestrictToPowerOf2(int(length*65536.0)))/65536.0;
		width = float(RestrictToPowerOf2(int(width*65536.0)))/65536.0;
	}

	widthSegs *= 2;					        // convert from patch count to segment count
	lengthSegs *= 2;

	int nverts = (widthSegs+1)*(lengthSegs+1);
	int nfaces = widthSegs * lengthSegs * 2;

	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.setSmoothFlags(smooth != 0);

	// create vertices
	for(int y=0;y<lengthSegs+1;y++)
	{
		float yPos = ((1.0/lengthSegs)*y)*length;
		for(int x=0;x<widthSegs+1;x++)
		{
			int vertNum = (y*(widthSegs+1))+x;
			assert(vertNum < nverts);
			float xPos = ((1.0/widthSegs)*x)*width;
			mesh.setVert(vertNum, xPos, yPos, 0.0f);
		}
	}

	// create faces
	{
	bool phase;		            // oscilates between odd & even
	for(int y=0;y<lengthSegs;y++)
	{
		phase = y&1;			        // odd or even line
		for(int x=0;x<widthSegs;x++)
		{
			int faceNum = ((y*widthSegs)+x)*2;

			int vert0 = (y*(widthSegs+1))+x;
			int vert1 = (y*(widthSegs+1))+x+1;
			int vert2 = ((y+1)*(widthSegs+1))+x;
			int vert3 = ((y+1)*(widthSegs+1))+x+1;

			int matID = 0;
			if(multiMaterial)
				matID = ((y/2)*(widthSegs/2)) + (x/2);

			if(phase)
			{							// odd (/)
				mesh.faces[faceNum].setSmGroup(smooth?1:0);
				mesh.faces[faceNum].setMatID(matID);
   				mesh.faces[faceNum].setEdgeVisFlags(1,1,1);
				mesh.faces[faceNum].setVerts(vert0, vert2, vert1);

				mesh.faces[faceNum+1].setSmGroup(smooth?1:0);
				mesh.faces[faceNum+1].setMatID(matID);
				mesh.faces[faceNum+1].setEdgeVisFlags(1,1,1);
				mesh.faces[faceNum+1].setVerts(vert3, vert1, vert2);
			}
			else
			{	                        // even  (\)
				mesh.faces[faceNum+0].setSmGroup(smooth?1:0);
				mesh.faces[faceNum+0].setMatID(matID);
				mesh.faces[faceNum+0].setEdgeVisFlags(1,1,1);
				mesh.faces[faceNum+0].setVerts(vert2, vert3,vert0);

				mesh.faces[faceNum+1].setSmGroup(smooth?1:0);
				mesh.faces[faceNum+1].setMatID(matID);
   				mesh.faces[faceNum+1].setEdgeVisFlags(1,1,1);
				mesh.faces[faceNum+1].setVerts(vert1, vert0, vert3);

			}
			phase = !phase;
		}
	}
	}

	if (genUVs)
	{
		if(multiMaterial)
		{
			mesh.setNumTVerts( 9 );
			mesh.setNumTVFaces(nfaces);

			mesh.setTVert(0, 0.0f, -0.0f, 0.0f);
			mesh.setTVert(1, 0.5f, -0.0f, 0.0f);
			mesh.setTVert(2, 1.0f, -0.0f, 0.0f);
			mesh.setTVert(3, 0.0f, -0.5f, 0.0f);
			mesh.setTVert(4, 0.5f, -0.5f, 0.0f);
			mesh.setTVert(5, 1.0f, -0.5f, 0.0f);
			mesh.setTVert(6, 0.0f, -1.0f, 0.0f);
			mesh.setTVert(7, 0.5f, -1.0f, 0.0f);
			mesh.setTVert(8, 1.0f, -1.0f, 0.0f);

			for(int faceIndex=0;faceIndex<nfaces;faceIndex+=4)
			{
				bool phase = ((faceIndex/2) /widthSegs) & 1;
				if(phase)
				{        			    // odd rows
					mesh.tvFace[faceIndex+0].setTVerts(3,6,4);
					mesh.tvFace[faceIndex+1].setTVerts(7,4,6);
					mesh.tvFace[faceIndex+2].setTVerts(7,8,4);
					mesh.tvFace[faceIndex+3].setTVerts(5,4,8);
				}
				else
				{						// even rows
					mesh.tvFace[faceIndex+0].setTVerts(3,4,0);
					mesh.tvFace[faceIndex+1].setTVerts(1,0,4);
					mesh.tvFace[faceIndex+2].setTVerts(1,4,2);
					mesh.tvFace[faceIndex+3].setTVerts(5,2,4);
				}
			}
		}
		else
		{
			int xTCount = widthSegs+1;
			int yTCount = lengthSegs+1;
			mesh.setNumTVerts( xTCount * yTCount ) ;
			mesh.setNumTVFaces(nfaces);

			// do texture vertices
			for (int yIndex =0; yIndex<yTCount; yIndex++)
			{
				float v = yIndex*(1.0f/float(lengthSegs));
				for (int xIndex =0; xIndex<xTCount; xIndex++)
				{
					mesh.setTVert((yIndex*xTCount)+xIndex, xIndex*(1.0f/float(widthSegs)), 1.0f-v, 0.0f);
				}
			}

			// do texture faces
			for(int y=0;y<lengthSegs;y++)
			{
				bool phase = y&1;			        // odd or even line
				for(int x=0;x<widthSegs;x++)
				{
					int faceNum = ((y*widthSegs)+x)*2;

					int vert0 = (y*(widthSegs+1))+x;
					int vert1 = (y*(widthSegs+1))+x+1;
					int vert2 = ((y+1)*(widthSegs+1))+x;
					int vert3 = ((y+1)*(widthSegs+1))+x+1;

					if(phase)
					{				    // odd rows
						mesh.tvFace[faceNum].setTVerts(vert0,vert2,vert1);
						mesh.tvFace[faceNum+1].setTVerts(vert3,vert1,vert2);
					}
					else
					{					// even rows
						mesh.tvFace[faceNum].setTVerts(vert2,vert3,vert0);
						mesh.tvFace[faceNum+1].setTVerts(vert1,vert0,vert3);
					}
					phase = !phase;
				}
			}
		}
	}
	else
	{
		mesh.setNumTVerts(0);
		mesh.setNumTVFaces(0);
	}
	mesh.InvalidateGeomCache();
	mesh.BuildStripsAndEdges();
}


//=============================================================================
// --- Inherited virtual methods of IParamArray ---
// These methods allow the parameter map to access our class variables.
// Based on the virtual array index passed in we set or get the
// variable requested.

BOOL
CycloneTerrainObject::SetValue(int i, TimeValue t, int v)
{
	switch (i)
	{
		default:
			assert(0);
			break;
	}
	return TRUE;
}

//=============================================================================

BOOL
CycloneTerrainObject::SetValue(int i, TimeValue t, float v)
{
	switch (i)
	{
		case PB_TI_WIDTH :
//			if(dlgRestrictToPowerOf2)
//				v = float(RestrictToPowerOf2(int(v*65536.0)))/65536.0;
			crtWidth = v;
			break;
		case PB_TI_LENGTH :
//			if(dlgRestrictToPowerOf2)
//				v = float(RestrictToPowerOf2(int(v*65536.0)))/65536.0;
			crtLength = v;
			break;
		default:
			assert(0);
			break;
	}
	return TRUE;
}

//=============================================================================

BOOL
CycloneTerrainObject::SetValue(int i, TimeValue t, Point3 &v)
{
	switch (i)
	{
		case PB_TI_POS:
			crtPos = v;
			break;
		default:
			assert(0);
			break;
	}
	return TRUE;
}

//=============================================================================

BOOL
CycloneTerrainObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid)
{
	switch (i)
	{
//		case PB_CREATEMETHOD:
//			v = 0;
//			break;
		default:
			assert(0);
			break;
	}
	return TRUE;
}

//=============================================================================

BOOL
CycloneTerrainObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid)
{
	switch (i)
	{
		case PB_TI_WIDTH :
			v = crtWidth;
			break;
		case PB_TI_LENGTH :
			v = crtLength;
			break;
		default:
			assert(0);
			break;
	}
	return TRUE;
}

//=============================================================================

BOOL
CycloneTerrainObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid)
{
	switch (i)
	{
		case PB_TI_POS:
			v = crtPos;
			break;
		default:
			assert(0);
			break;
	}
	return TRUE;
}


// See the Advanced Topics section on DLL Functions and Class Descriptors
// for more information.
//=============================================================================
// | The Class Descriptor
//=============================================================================

class CycloneTerrainClassDesc: public ClassDesc
{
public:
	// kts added: call reset upon creation
	CycloneTerrainClassDesc()  { ResetClassParams(TRUE); }

	// This method should return TRUE if the plug-in can be picked
	// and assigned by the user. Some plug-ins may be used privately by other
	// plug-ins implemented in the same DLL and should not appear in lists for
	// user to choose from (such plug-ins would return FALSE).
	int 			IsPublic() { return 1; }
	// This is the method that actually creates a new instance of
	// the plug-in class.  The system calls the correspoding
	// Animatable::DeleteThis() method of the plug-in to free the memory.
	// This implementations use 'new' and 'delete'.
	void			*Create(BOOL loading = FALSE) { return new CycloneTerrainObject; }
	// This is the name that appears on the button in the MAX user interface.
	const TCHAR		*ClassName() { return GetString(IDS_RB_OBJECT_BUTTON_NAME); }
	// The system calls this method at startup to determine the type of plug-in
	// this is. The possible options are defined in PLUGAPI.H
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	// The system calls this method to retrieve the unique Class ID
	// for this object.
	Class_ID		ClassID() { return CYCLONE_TERRAIN_C_CLASS_ID; }
	// The category is selected in the bottom most drop down list in the
	// reate branch. If this is set to be an exiting category for example
	// "Standard Primitives" then the plug-in will appear in that category.
	// If the category doesn't exists then it is created.
	const TCHAR		*Category() { return GetString(IDS_RB_PRIM_TYPE); }
	// When the user executes File / Reset this method is called.  The plug-in
	// can respond by resetting itself to use its default values.
	void			ResetClassParams(BOOL fileReset);
};

//=============================================================================
// Declare a single static instance of the class descriptor.
static CycloneTerrainClassDesc cycloneTerrainDesc;

//=============================================================================
// This function returns the address of the descriptor.  We call it from
// the LibClassDesc() function, which is called by the system when loading
// the DLLs at startup.

ClassDesc*
GetCycloneTerrainDesc()
{
	return &cycloneTerrainDesc;
}

//=============================================================================

void CycloneTerrainClassDesc::ResetClassParams(BOOL fileReset)
{
	CycloneTerrainObject::dlgWidthSegments    = DEF_SEGMENTS;
	CycloneTerrainObject::dlgLengthSegments    = DEF_SEGMENTS;
	CycloneTerrainObject::dlgSmooth      = SMOOTH_ON;
	CycloneTerrainObject::dlgFixedSegmentSize = 0;
	CycloneTerrainObject::dlgRestrictToPowerOf2  = 1;
	CycloneTerrainObject::crtPos         = Point3(0,0,0);
	CycloneTerrainObject::crtLength      = 0.0f;
	CycloneTerrainObject::crtWidth      = 0.0f;
}

//=============================================================================

#include "standard.cpp"

//=============================================================================
