//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
// unused
// --- Global Functions ---
// Triangular patch layout:
//
//   A---> ac ----- ca <---C
//   |                    /
//   |                  /
//   v    i1    i3    /
//   ab            cb
//
//   |           /
//   |    i2   /
//
//   ba     bc
//   ^     /
//   |   /
//   | /
//   B
//
// vertices ( a b c d ) are in counter clockwise order when viewed from
// outside the surface

// Vector length for unit circle
#define CIRCLE_VECTOR_LENGTH 0.5517861843f

#if 0
// Build a Patch rep of the sphere for use with ConvertToType().
static void BuildSpherePatch(PatchMesh& amesh, float radius, int smooth, BOOL textured)
{
	Point3 p;
	int np=0,nv=0;

	int nverts = 6;
	int nvecs = 48;
	int npatches = 8;
	amesh.setNumVerts(nverts);
	amesh.setNumTVerts(textured ? 13 : 0);
	amesh.setNumVecs(nvecs);
	amesh.setNumPatches(npatches);
	amesh.setNumTVPatches(textured ? npatches : 0);

	Point3 v0(0.0f, 0.0f, radius);		// Top
	Point3 v1(0.0f, 0.0f, -radius);		// Bottom
	Point3 v2(0.0f, -radius, 0.0f);		// Front
	Point3 v3(radius, 0.0f, 0.0f);		// Right
	Point3 v4(0.0f, radius, 0.0f);		// Back
	Point3 v5(-radius, 0.0f, 0.0f);		// Left

	// Create the vertices.
	amesh.verts[0].flags = PVERT_COPLANAR;
	amesh.verts[1].flags = PVERT_COPLANAR;
	amesh.verts[2].flags = PVERT_COPLANAR;
	amesh.verts[3].flags = PVERT_COPLANAR;
	amesh.verts[4].flags = PVERT_COPLANAR;
	amesh.verts[5].flags = PVERT_COPLANAR;
	amesh.setVert(0, v0);
	amesh.setVert(1, v1);
	amesh.setVert(2, v2);
	amesh.setVert(3, v3);
	amesh.setVert(4, v4);
	amesh.setVert(5, v5);

	if(textured)
	{
		amesh.setTVert(0, UVVert(0.125f,1.0f,0.0f));
		amesh.setTVert(1, UVVert(0.375f,1.0f,0.0f));
		amesh.setTVert(2, UVVert(0.625f,1.0f,0.0f));
		amesh.setTVert(3, UVVert(0.875f,1.0f,0.0f));
		amesh.setTVert(4, UVVert(0.0f,0.5f,0.0f));
		amesh.setTVert(5, UVVert(0.25f,0.5f,0.0f));
		amesh.setTVert(6, UVVert(0.5f,0.5f,0.0f));
		amesh.setTVert(7, UVVert(0.75f,0.5f,0.0f));
		amesh.setTVert(8, UVVert(1.0f,0.5f,0.0f));
		amesh.setTVert(9, UVVert(0.125f,0.0f,0.0f));
		amesh.setTVert(10, UVVert(0.375f,0.0f,0.0f));
		amesh.setTVert(11, UVVert(0.625f,0.0f,0.0f));
		amesh.setTVert(12, UVVert(0.875f,0.0f,0.0f));

		amesh.getTVPatch(0).setTVerts(3,7,8);
		amesh.getTVPatch(1).setTVerts(0,4,5);
		amesh.getTVPatch(2).setTVerts(1,5,6);
		amesh.getTVPatch(3).setTVerts(2,6,7);
		amesh.getTVPatch(4).setTVerts(12,8,7);
		amesh.getTVPatch(5).setTVerts(9,5,4);
		amesh.getTVPatch(6).setTVerts(10,6,5);
		amesh.getTVPatch(7).setTVerts(11,7,6);
	}

	// Create the edge vectors
	float vecLen = CIRCLE_VECTOR_LENGTH * radius;
	Point3 xVec(vecLen, 0.0f, 0.0f);
	Point3 yVec(0.0f, vecLen, 0.0f);
	Point3 zVec(0.0f, 0.0f, vecLen);
	amesh.setVec(0, v0 - yVec);
	amesh.setVec(2, v0 + xVec);
	amesh.setVec(4, v0 + yVec);
	amesh.setVec(6, v0 - xVec);
	amesh.setVec(8, v1 - yVec);
	amesh.setVec(10, v1 + xVec);
	amesh.setVec(12, v1 + yVec);
	amesh.setVec(14, v1 - xVec);
	amesh.setVec(9, v2 - zVec);
	amesh.setVec(16, v2 + xVec);
	amesh.setVec(1, v2 + zVec);
	amesh.setVec(23, v2 - xVec);
	amesh.setVec(11, v3 - zVec);
	amesh.setVec(18, v3 + yVec);
	amesh.setVec(3, v3 + zVec);
	amesh.setVec(17, v3 - yVec);
	amesh.setVec(13, v4 - zVec);
	amesh.setVec(20, v4 - xVec);
	amesh.setVec(5, v4 + zVec);
	amesh.setVec(19, v4 + xVec);
	amesh.setVec(15, v5 - zVec);
	amesh.setVec(22, v5 - yVec);
	amesh.setVec(7, v5 + zVec);
	amesh.setVec(21, v5 + yVec);

	// Create the patches
	amesh.MakeTriPatch(np++, 0, 0, 1, 2, 16, 17, 3, 3, 2, 24, 25, 26, smooth);
	amesh.MakeTriPatch(np++, 0, 2, 3, 3, 18, 19, 4, 5, 4, 27, 28, 29, smooth);
	amesh.MakeTriPatch(np++, 0, 4, 5, 4, 20, 21, 5, 7, 6, 30, 31, 32, smooth);
	amesh.MakeTriPatch(np++, 0, 6, 7, 5, 22, 23, 2, 1, 0, 33, 34, 35, smooth);
	amesh.MakeTriPatch(np++, 1, 10, 11, 3, 17, 16, 2, 9, 8, 36, 37, 38, smooth);
	amesh.MakeTriPatch(np++, 1, 12, 13, 4, 19, 18, 3, 11, 10, 39, 40, 41, smooth);
	amesh.MakeTriPatch(np++, 1, 14, 15, 5, 21, 20, 4, 13, 12, 42, 43, 44, smooth);
	amesh.MakeTriPatch(np++, 1, 8, 9, 2, 23, 22, 5, 15, 14, 45, 46, 47, smooth);

	// Create all the interior vertices and make them non-automatic
	float chi = 0.5893534f * radius;

	int interior = 24;
	amesh.setVec(interior++, Point3(chi, -chi, radius));
	amesh.setVec(interior++, Point3(chi, -radius, chi));
	amesh.setVec(interior++, Point3(radius, -chi, chi));

	amesh.setVec(interior++, Point3(chi, chi, radius));
	amesh.setVec(interior++, Point3(radius, chi, chi));
	amesh.setVec(interior++, Point3(chi, radius, chi));

	amesh.setVec(interior++, Point3(-chi, chi, radius));
	amesh.setVec(interior++, Point3(-chi, radius, chi));
	amesh.setVec(interior++, Point3(-radius, chi, chi));

	amesh.setVec(interior++, Point3(-chi, -chi, radius));
	amesh.setVec(interior++, Point3(-radius, -chi, chi));
	amesh.setVec(interior++, Point3(-chi, -radius, chi));

	amesh.setVec(interior++, Point3(chi, -chi, -radius));
	amesh.setVec(interior++, Point3(radius, -chi, -chi));
	amesh.setVec(interior++, Point3(chi, -radius, -chi));

	amesh.setVec(interior++, Point3(chi, chi, -radius));
	amesh.setVec(interior++, Point3(chi, radius, -chi));
	amesh.setVec(interior++, Point3(radius, chi, -chi));

	amesh.setVec(interior++, Point3(-chi, chi, -radius));
	amesh.setVec(interior++, Point3(-radius, chi, -chi));
	amesh.setVec(interior++, Point3(-chi, radius, -chi));

	amesh.setVec(interior++, Point3(-chi, -chi, -radius));
	amesh.setVec(interior++, Point3(-chi, -radius, -chi));
	amesh.setVec(interior++, Point3(-radius, -chi, -chi));

	for(int i = 0; i < 8; ++i)
		amesh.patches[i].SetAuto(FALSE);

	// Finish up patch internal linkages (and bail out if it fails!)
	assert(amesh.buildLinkages());

	// Calculate the interior bezier points on the PatchMesh's patches
	amesh.computeInteriors();
	amesh.InvalidateGeomCache();
}

//=============================================================================
// Build a NURBS rep of the sphere for use with ConvertToType().

Object *
BuildNURBSSphere(float radius, float hemi, BOOL recenter, BOOL genUVs)
{
	NURBSSet nset;

	Point3 center(0,0,0);
	Point3 northAxis(0,0,1);
	Point3 refAxis(0,-1,0);

	float startAngleU = -PI;
	float endAngleU = PI;
	float startAngleV = -HALFPI + (hemi * PI);
	float endAngleV = HALFPI;
	if (recenter)
		center = Point3(0.0, 0.0, -cos((1.0f-hemi) * PI) * radius);

	NURBSCVSurface *surf = new NURBSCVSurface();
	nset.AppendObject(surf);
	surf->SetGenerateUVs(genUVs);

	surf->SetTextureUVs(0, 0, Point2(0.0f, hemi));
	surf->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
	surf->SetTextureUVs(0, 2, Point2(1.0f, hemi));
	surf->SetTextureUVs(0, 3, Point2(1.0f, 1.0f));

	surf->FlipNormals(TRUE);
	surf->Renderable(TRUE);
	char bname[80];
	char sname[80];
	strcpy(bname, GetString(IDS_RB_OBJECT_NAME));
	sprintf(sname, "%s%s", bname, GetString(IDS_CT_SURF));
	surf->SetName(sname);
	GenNURBSSphereSurface(radius, center, northAxis, refAxis,
					startAngleU, endAngleU, startAngleV, endAngleV, *surf);


#define F(s1, s2, s1r, s1c, s2r, s2c) \
	fuse.mSurf1 = (s1); \
	fuse.mSurf2 = (s2); \
	fuse.mRow1 = (s1r); \
	fuse.mCol1 = (s1c); \
	fuse.mRow2 = (s2r); \
	fuse.mCol2 = (s2c); \
	nset.mSurfFuse.Append(1, &fuse);

	NURBSFuseSurfaceCV fuse;

	// poles
	for (int f = 1; f < surf->GetNumVCVs(); f++) {
		if (hemi <= 0.0f) {
			// south pole
			F(0, 0, 0, 0, 0, f);
		}
		//north pole
		F(0, 0, surf->GetNumUCVs()-1, 0, surf->GetNumUCVs()-1, f);
	}
	// seam
	for (f = 0; f < surf->GetNumUCVs()-1; f++) {
		F(0, 0, f, 0, f, surf->GetNumVCVs()-1);
	}

	if (hemi > 0.0f) {
		// Cap
		// we need to make a cap that is a NURBS surface one edge of which matches
		// the south polar edge and the rest is degenerate...
		Point3 bot;
		if (recenter)
			bot = Point3(0,0,0);
		else
			bot = Point3(0.0, 0.0, cos((1.0-hemi) * PI)*radius);

		NURBSCVSurface *s0 = (NURBSCVSurface*)nset.GetNURBSObject(0);
		NURBSCVSurface *s = new NURBSCVSurface();
		nset.AppendObject(s);

		// we'll be cubic in on direction and match the sphere in the other
		s->SetUOrder(4);
		s->SetNumUKnots(8);
		for (int i = 0; i < 4; i++) {
			s->SetUKnot(i, 0.0);
			s->SetUKnot(i+4, 1.0);
		}

		s->SetVOrder(s0->GetVOrder());
		s->SetNumVKnots(s0->GetNumVKnots());
		for (i = 0; i < s->GetNumVKnots(); i++)
			s->SetVKnot(i, s0->GetVKnot(i));

		int numU, numV;
		s0->GetNumCVs(numU, numV);
		s->SetNumCVs(4, numV);

		for (int v = 0; v < numV; v++) {
			Point3 edge = s0->GetCV(0, v)->GetPosition(0);
			double w = s0->GetCV(0, v)->GetWeight(0);
			for (int u = 0; u < 4; u++) {
				NURBSControlVertex ncv;
				ncv.SetPosition(0, bot + ((edge - bot)*((float)u/3.0f)));
				ncv.SetWeight(0, w);
				s->SetCV(u, v, ncv);
			}
			// fuse the cap to the hemi
			F(1, 0, 3, v, 0, v);

			// fuse the center degeneracy
			if (v > 0) {
				F(1, 1, 0, 0, 0, v);
			}
		}

		// fuse the remaining two end sections
		F(1, 1, 1, 0, 1, numV-1);
		F(1, 1, 2, 0, 2, numV-1);

		s->SetGenerateUVs(genUVs);

		s->SetTextureUVs(0, 0, Point2(1.0f, 1.0f));
		s->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
		s->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
		s->SetTextureUVs(0, 3, Point2(0.0f, 0.0f));

		s->FlipNormals(TRUE);
		s->Renderable(TRUE);
		sprintf(sname, "%s%s%01", bname, GetString(IDS_CT_CAP));
		s->SetName(sname);
	}

	Matrix3 mat;
	mat.IdentityMatrix();
	Object *ob = CreateNURBSObject(NULL, &nset, mat);
	for (int i = 0; i < nset.GetNumObjects(); i++)
		delete (NURBSCVSurface*)nset.GetNURBSObject(i);
	return ob;
}
#endif

//=============================================================================

Point3 CycloneTerrainObject::GetSurfacePoint(
		TimeValue t, float u, float v,Interval &iv)
{
	float rad;
	pblock->GetValue(PB_RADIUS, t, rad, iv);
	Point3 pos;
	v -= 0.5f;
	float ar = (float)cos(v*PI);
	pos.x = rad * float(cos(u*TWOPI)) * ar;
	pos.y = rad * float(sin(u*TWOPI)) * ar;
	pos.z = rad * float(sin(v*PI));
	return pos;
}

//=============================================================================
//=============================================================================

int CycloneTerrainObject::IntersectRay(
		TimeValue t, Ray& ray, float& at, Point3& norm)
	{
	int smooth, recenter;
	pblock->GetValue(PB_SMOOTH,t,smooth,FOREVER);
	pblock->GetValue(PB_RECENTER,t,recenter,FOREVER);
	float hemi;
	pblock->GetValue(PB_HEMI,t,hemi,FOREVER);
	if (!smooth || hemi!=0.0f || recenter) {
		return SimpleObject::IntersectRay(t,ray,at,norm);
		}

	float r;
	float a, b, c, ac4, b2, at1, at2;
	float root;
	BOOL neg1, neg2;

	pblock->GetValue(PB_RADIUS,t,r,FOREVER);

	a = DotProd(ray.dir,ray.dir);
	b = DotProd(ray.dir,ray.p) * 2.0f;
	c = DotProd(ray.p,ray.p) - r*r;

	ac4 = 4.0f * a * c;
	b2 = b*b;

	if (ac4 > b2) return 0;

	// We want the smallest positive root
	root = float(sqrt(b2-ac4));
	at1 = (-b + root) / (2.0f * a);
	at2 = (-b - root) / (2.0f * a);
	neg1 = at1<0.0f;
	neg2 = at2<0.0f;
	if (neg1 && neg2) return 0;
	else
	if (neg1 && !neg2) at = at2;
	else
	if (!neg1 && neg2) at = at1;
	else
	if (at1<at2) at = at1;
	else at = at2;

	norm = Normalize(ray.p + at*ray.dir);

	return 1;
	}

// --- Inherited virtual methods of GenTerrain ---

#if 0
void
CycloneTerrainObject::SetParams(float rad, int segs, BOOL smooth, BOOL genUV,
	 float hemi, BOOL squash, BOOL recenter)
{
	pblock->SetValue(PB_RADIUS,0, rad);
	pblock->SetValue(PB_HEMI,0, hemi);
	pblock->SetValue(PB_SEGS,0, segs);
	pblock->SetValue(PB_SQUASH,0, squash);
	pblock->SetValue(PB_SMOOTH,0, smooth);
	pblock->SetValue(PB_RECENTER,0, recenter);
	pblock->SetValue(PB_GENUVS,0, genUV);
}
#endif

