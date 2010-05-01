//==============================================================================
// cyterrain.hpp:
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

#include "resource.h"
#include "max.h"
#include "iparamm.h"
#include "simpobj.h"
#include "surf_api.h"

#include "standard.hpp"

//=============================================================================
// kts not used yet
#define OBJECT_NAME CycloneTerrain

//=============================================================================
// Unique Class ID.  It is specified as two 32-bit quantities.
// These must be unique.  Use the Class_ID generator program to create
// these values.  This program is available from the main menu of the
// help file.

#define CYCLONE_TERRAIN_C_CLASS_ID Class_ID(0x67bf0fd4, 0x5f473045)

//=============================================================================
// The terrain object class definition.  It is derived from SimpleObject and
// IParamArray.  SimpleObject is the class to derive objects from which have
// geometry are renderable, and represent themselves using a mesh.
// IParamArray is used as part of the Parameter Map scheme used to manage
// the user interface parameters.

class CycloneTerrainObject : public SimpleObject, public IParamArray
{
	public:
		// Class variables
		// There is only one set of these variables shared by all instances
		// of this class.  This is OK because there is only one terrain
		// being edited at a time.
		static IParamMap* pmapTypeIn;
		static IParamMap* pmapParam;
		static int dlgWidthSegments;
		static int dlgLengthSegments;
		static int dlgSmooth;
		static int dlgRestrictToPowerOf2;
		static int dlgFixedSegmentSize;
		static Point3 crtPos;
		static float crtLength;
		static float crtWidth;
		// This is the Interface pointer into MAX.  It is used to call
		// functions implemented in MAX itself.
		static IObjParam *ip;

		// --- Inherited virtual methods of Animatable ---
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return CYCLONE_TERRAIN_C_CLASS_ID; }
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		// --- Inherited virtual methods of ReferenceMaker ---
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		// --- Inherited virtual methods of ReferenceTarget ---
		RefTargetHandle Clone(RemapDir& remap = NoRemap());

		// --- Inherited virtual methods of BaseObject ---
		CreateMouseCallBack* GetCreateMouseCallBack();
		TCHAR *GetObjectName() { return GetString(IDS_RB_OBJECT_NAME); }

		// --- Inherited virtual methods of Object ---
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		BOOL IsParamSurface() {return FALSE;}
//		Point3 GetSurfacePoint(TimeValue t, float u, float v,Interval &iv);
//		int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);

		// --- Inherited virtual methods of SimpleObject ---
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

		// --- Inherited virtual methods of GenTerrain ---
//		void SetParams(float rad, int segs, BOOL smooth=TRUE, BOOL genUV=TRUE,
//			 float hemi=0.0f, BOOL squash=FALSE, BOOL recenter=FALSE);

		// --- Inherited virtual methods of IParamArray ---
		BOOL SetValue(int i, TimeValue t, int v);
		BOOL SetValue(int i, TimeValue t, float v);
		BOOL SetValue(int i, TimeValue t, Point3 &v);
		BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

		// --- Methods of CycloneTerrainObject ---
		CycloneTerrainObject();
};

//=============================================================================
// Misc stuff
#define MAX_SEGMENTS	50
#define MIN_SEGMENTS	1

#define MIN_SIZE		float(0)
#define MAX_SIZE		float(1.0E30)

#define MIN_SMOOTH		0
#define MAX_SMOOTH		1

#define DEF_SEGMENTS	4
#define DEF_SIZE		float(0.0)

#define SMOOTH_ON	1
#define SMOOTH_OFF	0

//=============================================================================
