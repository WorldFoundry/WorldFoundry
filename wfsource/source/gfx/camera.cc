//=============================================================================
// Camera.cc:
// Copyright ( c ) 1997,1998,1999,2000,2001,2002 World Foundry Group  
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org

// ===========================================================================
// Description:
//
// Original Author: Kevin T. Seghetti
//============================================================================

#include <gfx/camera.hp>
#include <hal/platform.h>
#include <gfx/rendmatt.hp>
#include <cpplib/libstrm.hp>

#if defined(__PSX__)
#include <inline_c.h>
#elif defined(__WIN__)
#define scratchPadMemory _HALLmalloc
#elif defined(__LINUX__)
#define scratchPadMemory _HALLmalloc
#endif
        
        
#if defined(RENDERER_PIPELINE_DIRECTX)
#define D3D_OVERLOADS
#include <ddraw.h>
#include <d3d.h>
#include <gfx/directx/winmain.hp>
#include <gfx/directx/winproc.hp>
#include <gfx/directx/d3dutils.hp>
#include <gfx/math.hp>
#endif

//============================================================================

#if DO_ASSERTIONS
bool RenderCamera::_renderInProgress = false;	// there can be only one (camera rendering at a time)
#endif

//============================================================================

RenderCamera::RenderCamera(ViewPort& viewPort ) : _viewPort(viewPort)
#if defined(DO_STEREOGRAM)
	,_eyeAngle(Angle::Degree(SCALAR_CONSTANT(2.5)))
	,_eyeDistance(SCALAR_CONSTANT(0.025))
#endif
{
	_viewPort.Validate();
}

//============================================================================

RenderCamera::~RenderCamera()
{
	assert(!_renderInProgress);
}

//============================================================================
#if defined(DO_STEREOGRAM)

void
RenderCamera::SetStereogram(Scalar eyeDistance, Angle eyeAngle)
{
	_eyeDistance = eyeDistance;
	_eyeAngle = eyeAngle;
}
#endif

//============================================================================

Matrix34 viewToScreen = Matrix34(
   Vector3(SCALAR_CONSTANT(2.163085),SCALAR_CONSTANT(0),SCALAR_CONSTANT(0)),
   Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(2.163085),SCALAR_CONSTANT(0)),
   Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),SCALAR_CONSTANT(1.001953)),
   Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),SCALAR_CONSTANT(0.200164))
   );

//============================================================================
#if defined(RENDERER_PIPELINE_PSX)

#define gte_SetDQA( r0 ) __asm__ volatile (		\
	"addu	$12,$0,%0;"						\
	"ctc2	$12, $27"					\
	:							\
	: "r"( r0 ) 			\
	: "a0" )

//-----------------------------------------------------------------------------

#define gte_SetDQB( r0 ) __asm__ volatile (		\
	"addu	$12,$0,%0;"				\
	"ctc2	$12, $28"					\
	:							\
	: "r"( r0 )             \
	: "$12"  )


#else

#if defined(RENDERER_PIPELINE_SOFTWARE)
int32 gte_DQA;
int32 gte_DQB;
Color gte_lcm[3];
Color gte_farColor;
Color gte_ambientColor;
Matrix34 gte_llm;

#define gte_SetDQA(a) gte_DQA = (a)
#define gte_SetDQB(a) gte_DQB = (a)
#endif
#endif

//=============================================================================

#if defined(RENDERER_PIPELINE_SOFTWARE) || defined(RENDERER_PIPELINE_PSX)

void
WF_SetFogNearFar(int16 fogNear, int16 fogFar,int16 h)
{
	assert(fogNear > h);
	assert(fogNear > 0);
	assert(fogFar > 0);
	AssertMsg(fogNear <= fogFar,"fogNear = " << fogNear << ", fogFar = " << fogFar);
	assert(h > 0);
	int16 delta = fogFar-fogNear;
	if(delta >= 0x64)
	{
		int32 DQA,DQB;

		assert(delta);
		assert(h);
		DQA = (
			  	(
					(
						(0-fogNear)*fogFar
					) / delta
				) << 0x8
			  ) / h;

		if(DQA < -0x8000)
			DQA = -0x8000;

		if(DQA > 0x7fff)
			DQA = 0x7fff;

		DQB = ((fogFar << 0xc) / delta) << 0xc;

//		cout << hex << "DQA = " << DQA << ", DQB = " << DQB << std::endl;

		gte_SetDQA(DQA);
		gte_SetDQB(DQB);
	}
	// kts added
	else
		assert(0);
}

#endif

//============================================================================

#if defined(DO_STEREOGRAM)

void
FntPrintScalar(Scalar q)
{
	Scalar temp;
	if (q < Scalar::zero)
	{
		Scalar foo((long)((~q.AsLong())+1));
		temp = foo;
		FntPrint("-");
	}
	else
		temp = q;

	// dump out the top 16 bits (whole part)
	FntPrint("%d",temp.WholePart());

	// get the bottom 16 bits (fractional part)
	Scalar frac = Scalar((long)temp.AsUnsignedFraction());					// mask off whole part
	if( frac != Scalar::zero )
	{
		FntPrint(".");

		assert(frac.WholePart() == 0);

		int digits = 0;
		const int max_scalar_fractional_digits = 6;
		while( frac.AsLong() && digits < max_scalar_fractional_digits )
		{
			frac *= SCALAR_CONSTANT(10);
			int digit = frac.WholePart();
			FntPrint("%d",digit);

			frac = Scalar((long)frac.AsUnsignedFraction());
			assert(frac.WholePart() == 0);

			++digits;
		}
	}
}

void
AdjustStereoCameraParameters(Angle& eyeAngle, Scalar& eyeDistance)
{
#if defined( __PSX__ )
	uint32 padd = PadRead(1);
	padd >>= 16;

	if(padd)
	{
#if SW_DBSTREAM
		cscreen << "\n\nEyeAngle = " << eyeAngle << " (each)" << std::endl;
		cscreen << "EyeDistance = " << eyeDistance*SCALAR_CONSTANT(2) << std::endl;
#endif

		FntPrint("\n\nEyeAngle = ");
		FntPrintScalar(eyeAngle.AsDegree());
		FntPrint(" (each)\nEyeDistance =");
		FntPrintScalar(eyeDistance*SCALAR_CONSTANT(2));
		FntPrint("\n");
	}

//      if (padd & PADselect)   ret = -1;

	const Scalar distanceDelta(SCALAR_CONSTANT(0.0005));
	const Angle angleDelta(Angle::Degree(SCALAR_CONSTANT(0.01)));

	// change the rotation angles for the cube and the light source
//	if (padd & PADRup)
//		rotation.SetA(rotation.GetA() + Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
//	if (padd & PADRdown)
//		rotation.SetA(rotation.GetA() - Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
	if (padd & PADRleft)
		eyeAngle -= angleDelta;
	if (padd & PADRright)
		eyeAngle += angleDelta;
//	if (padd & PADR1)
//		rotation.SetC(rotation.GetC() - Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
//	if (padd & PADR2)
//		rotation.SetC(rotation.GetC() + Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));

//	if (padd & PADLup)
//		position.SetY(position.Y() + SCALAR_CONSTANT(ADJUST_DELTA));
//	if (padd & PADLdown)
//		position.SetY(position.Y() - SCALAR_CONSTANT(ADJUST_DELTA));
	if (padd & PADLleft)
		eyeDistance -= distanceDelta;
	if (padd & PADLright)
		eyeDistance += distanceDelta;

//	if (padd & PADL1)
//		position.SetZ(position.Z() - SCALAR_CONSTANT(ADJUST_DELTA));
//	if (padd & PADL2)
//		position.SetZ(position.Z() + SCALAR_CONSTANT(ADJUST_DELTA));
#endif

	// distance from screen
//      if (padd & PADR1)       vec.vz += 8;
//      if (padd & PADR2)       vec.vz -= 8;

//#if defined( __PSX__ )
//	DBSTREAM1(cscreen << "camera position:" << std::endl << position << std::endl;)
//	DBSTREAM1(cscreen << "camera rotation:" << std::endl << rotation << std::endl;)
//#endif
}
#endif

//============================================================================


inline void 
ConvertToGLColor(const Color& in, GLfloat* out)
{
   out[0] = float(in.Red())/256.0;
   out[1] = float(in.Green())/256.0;
   out[2] = float(in.Blue())/256.0;
   out[3] = 1.0;
}


//==============================================================================

GLenum GLLightTable[] = 
{
   GL_LIGHT0,
   GL_LIGHT1,
   GL_LIGHT2
};

//==============================================================================

void
RenderCamera::RenderBegin()
{
    DBSTREAM1( cgfx<< "RenderCamera::RenderBegin" << std::endl; )
	assert(!_renderInProgress);
#if DO_ASSERTIONS
	_renderInProgress = true;
#endif
	_viewPort.RenderBegin();

	assert(ValidPtr(scratchPadMemory));
//	_scratchMemory = (RendererScratchVariablesStruct*)scratchPadMemory->Allocate(sizeof(RendererScratchVariablesStruct));
	_scratchMemory = new (*scratchPadMemory) RendererScratchVariablesStruct;
	assert(ValidPtr(_scratchMemory));

#if defined(DO_STEREOGRAM)
	Matrix34 tempPosition = _position;
	AdjustStereoCameraParameters(_eyeAngle,_eyeDistance);

	if(_viewPort.GetConstructionOrderTableIndex() & 1)
	{
		// kts: right eye, two inches apart
//		cscreen << "\n\n\neyedist = " << _eyeDistance << std::endl;
		Vector3 offset = Vector3(_eyeDistance, Scalar::zero, Scalar::zero) * tempPosition;
		Matrix34 rotation( Euler( Angle::zero, Angle::zero, _eyeAngle ), Vector3::zero );		// euler rotation matrix
		tempPosition[3] = Vector3::zero;
		tempPosition *= rotation;
		tempPosition[3] = offset;
	}
	else
	{
		// kts: left eye, two inches apart
		Scalar negEyeDistance = Scalar::zero - _eyeDistance;
//		cscreen << "\n\nneg: " << negEyeDistance << std::endl;
		Vector3 offset = Vector3(negEyeDistance, Scalar::zero, Scalar::zero) * tempPosition;
		Matrix34 rotation( Euler( Angle::zero, Angle::zero, -_eyeAngle ), Vector3::zero );		// euler rotation matrix
		tempPosition[3] = Vector3::zero;
		tempPosition *= rotation;
		tempPosition[3] = offset;
	}
#define _position tempPosition
#endif

#if DO_ASSERTIONS
	Scalar dtm =
#endif
	_invertedPosition.Inverse(_position);
	assertNe(dtm,Scalar::zero);		// if zero, matrix could not be inverted

#if defined(RENDERER_PIPELINE_SOFTWARE)
	_invertedPosition *= viewToScreen;
	for(int index=0;index < 3;index++)
		gte_lcm[index] = Color(_dirLightColors[index]);

	gte_ambientColor = _ambientColor;
	gte_farColor = _fogColor;
	assert(_fogNear > Scalar::zero);
	assert(_fogFar > Scalar::zero);
	WF_SetFogNearFar(_fogNear.AsLong()>>8, _fogFar.AsLong()>>8, 110);
//	cout << "gte_lcm = " << *gte_lcm << std::endl;
#elif defined(RENDERER_PIPELINE_PSX)
	MATRIX gte_lcm;
	for(int index=0;index < MAX_LIGHTS;index++)
	{
		gte_lcm.m[0][index] = _dirLightColors[index].Red();
		gte_lcm.m[1][index] = _dirLightColors[index].Green();
		gte_lcm.m[2][index] = _dirLightColors[index].Blue();
	}
	gte_SetColorMatrix(&gte_lcm);
	gte_SetBackColor(_ambientColor.Red(),_ambientColor.Green(),_ambientColor.Blue());
	gte_SetFarColor(_fogColor.Red(),_fogColor.Green(),_fogColor.Blue());
	WF_SetFogNearFar(_fogNear.AsLong()>>8, _fogFar.AsLong()>>8, 110);
#if 0
	cout << "gte_lcm = ";
	for(int dy=0;dy<3;dy++)
	{
		cout << "[" << dy << "] ";
		for(int dx=0;dx<3;dx++)
			cout << gte_lcm.m[dy][dx] << ",";
		cout << std::endl;
	}
	for(int dt=0;dt<3;dt++)
		cout << "t = " << gte_lcm.t[dt] << ",";
	cout << std::endl;
#endif

#elif defined(RENDERER_PIPELINE_GL)

   GLfloat lightBlack[] = {
       0.0, 0.0, 0.0, 1.0
   };

   GLfloat lightColor[4];

   ConvertToGLColor(_ambientColor, lightColor);
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT,lightColor);

#if MAX_LIGHTS > GL_MAX_LIGHTS
#error This GL implemenation does not provide enough lights
#endif

//#if GL_LIGHT1 != (GL_LIGHT0+1) || GL_LIGHT2 != (GL_LIGHT1+1)
//#error GL light assumption violated
//#endif

   glMatrixMode(GL_MODELVIEW);               // so that lights don't get rotated
   LoadGLMatrixFromMatrix34(_invertedPosition);

    for(int index=0;index < MAX_LIGHTS;index++)
    {
        GLfloat lightDirection[4];
        lightDirection[3] = 0.0;
        
        // negate because we store the direction the light travels, where GL stores the lights position
        lightDirection[0] = -_dirLightDirections[index].X().AsFloat();
        lightDirection[1] = -_dirLightDirections[index].Y().AsFloat();
        lightDirection[2] = -_dirLightDirections[index].Z().AsFloat();
//        cout << "light direction[" << index << "]: " << _dirLightDirections[index] << endl;
//        cout << "light color: " << _dirLightColors[index] << endl;
        glLightfv(GLLightTable[index],GL_POSITION,lightDirection);
        
        ConvertToGLColor(_dirLightColors[index],lightColor);
        
        glLightfv(GLLightTable[index],GL_AMBIENT,lightBlack);
        glLightfv(GLLightTable[index],GL_DIFFUSE,lightColor);
        glLightfv(GLLightTable[index],GL_SPECULAR,lightBlack);
        AssertGLOK();
    }

   ConvertToGLColor(_fogColor,lightColor);
   glFogfv(GL_FOG_COLOR, lightColor); 
   glFogf(GL_FOG_START,_fogNear.AsFloat());
   glFogf(GL_FOG_END,_fogFar.AsFloat());
   glFogf(GL_FOG_MODE,GL_LINEAR);

#elif defined(RENDERER_PIPELINE_DIRECTX)

	// Get D3D window pointer
    LPD3DWindow lpd3dWindow =  (LPD3DWindow)GetWindowLong (g_hMainWindow, GWL_USERDATA);
	ValidatePtr(lpd3dWindow);

	LPDIRECT3DDEVICE2 lpd3dDevice = lpd3dWindow->GetD3DDevice();
	ValidatePtr(lpd3dDevice);

#if 0
	// kts test code
	{				                    // 3d poly
		static float zRot = 3.1415;
//		zRot += 0.01;

		static int zPos = 0;
		zPos +=1;
		if(zPos > 0)
			zPos = -100;

		D3DMATRIX m_View = ViewMatrix(D3DVECTOR(0,0,zPos), D3DVECTOR(sin(zRot),0,cos(zRot)+zPos), D3DVECTOR(0,1,0), 0);
		lpd3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &m_View);

		D3DMATRIX m_World = IdentityMatrix();
		lpd3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &m_World);

		D3DLVERTEX v[3];
		v[0] = D3DLVERTEX(D3DVECTOR( 0, 10,0),D3DRGB(1,0,0),D3DRGB(1,0,0),0,0);
		v[1] = D3DLVERTEX(D3DVECTOR( 10,-10,0),D3DRGB(0,1,0),D3DRGB(1,0,0),0,0);
		v[2] = D3DLVERTEX(D3DVECTOR(-10,-10,0),D3DRGB(0,0,1),D3DRGB(1,0,0),0,0);
		lpd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,D3DVT_LVERTEX,(LPVOID)v,3,NULL);
	}
	{			                        // 2d triangle
		static int x = 0;
		x+=10;
		if ( x > 200)
			x = 10;
		D3DTLVERTEX v[3];

		v[0] = D3DTLVERTEX(D3DVECTOR(240,50,0),1,D3DRGB(1,0,0),D3DRGB(0,0,0),0,0);
		v[1] = D3DTLVERTEX(D3DVECTOR(x,200,0),1,D3DRGB(0,1,0),D3DRGB(0,0,0),0,0);
		v[2] = D3DTLVERTEX(D3DVECTOR(80,200,0),1,D3DRGB(0,0,1),D3DRGB(0,0,0),0,0);
		lpd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,D3DVT_TLVERTEX,(LPVOID)v,3,NULL);
	}
#endif
	D3DMATRIX m_Projection = ProjectionMatrix(0.01f, 2000000.0f, (float)(60*PI/180)); // 60 degree FOV

// kts supposed to invert projection matrix to change handedness
//	cout << "proj\n" << m_Projection << std::endl;
//	m_Projection._13 = -m_Projection._13;
//	m_Projection._23 = -m_Projection._23;
//	m_Projection._33 = -m_Projection._33;
//	m_Projection._43 = -m_Projection._43;
//	cout << "proj after\n" << m_Projection << std::endl;
	lpd3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &m_Projection);

	D3DMATRIX cameraMatrix;
	Matrix34ToD3DMATRIX(cameraMatrix, _invertedPosition);
	cameraMatrix._13 = -cameraMatrix._13;
	cameraMatrix._23 = -cameraMatrix._23;
	cameraMatrix._33 = -cameraMatrix._33;
	cameraMatrix._43 = -cameraMatrix._43;
	lpd3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &cameraMatrix);
#else
#error renderer pipeline not defined!
#endif									// psx

#if 0
	for(int debugIndex=0;debugIndex < 3;debugIndex++)
		cout << "dirlight[" << debugIndex << "] = " << _dirLightColors[debugIndex] << std::endl;
#endif

    DBSTREAM1( cgfx<< "RenderCamera::RenderBegin: _position = " << std::endl << _position << std::endl; )
    DBSTREAM1( cgfx<< "RenderCamera::RenderBegin: _invertedPosition = " << std::endl << _invertedPosition << std::endl; )
    DBSTREAM1( cgfx<< "RenderCamera::RenderBegin done" << std::endl; )
}

//=============================================================================

void
RenderCamera::RenderEnd()
{
    DBSTREAM1( cgfx<< "RenderCamera::RenderEnd" << std::endl; )
	_viewPort.RenderEnd();
#if DO_ASSERTIONS
	_renderInProgress = false;
#endif
	scratchPadMemory->Free(_scratchMemory,sizeof(RendererScratchVariablesStruct));
    DBSTREAM1( cgfx<< "RenderCamera::RenderEnd done" << std::endl; )
}

//============================================================================

void
RenderCamera::RenderObject(RenderObject3D& object,const Matrix34& objectPosition)
{
    DBSTREAM1( cgfx<< "RenderCamera::RenderObject" << std::endl; )
	_viewPort.Validate();
	assert(_renderInProgress);
	// set up lighting

#if defined( RENDERER_PIPELINE_SOFTWARE)
	Matrix34 invertedObjectMatrix;
	invertedObjectMatrix[3] = Vector3::zero;
	invertedObjectMatrix.InverseDetOne(objectPosition);			// rotate lights into local coordinate space
    //cout << "Object position: " << objectPosition << std::endl;
    //cout << "invertedObjectMatrix = " << invertedObjectMatrix << std::endl;
    invertedObjectMatrix[3] = Vector3::zero;
	for(int index=0;index < 3;index++)
    {
		gte_llm[index] = _dirLightDirections[index] * invertedObjectMatrix;
    //    cout << "dirLightDirections[" << index << "] = " << _dirLightDirections[index] << std::endl;
    }

	//for(int debugIndex=0;debugIndex < 3;debugIndex++)
	//   cout << "gte_llm[" << debugIndex << "] = " << gte_llm[debugIndex] << std::endl;
	Matrix34 temp(objectPosition);
	temp *= _invertedPosition;
	object.Render(_viewPort,temp);
#elif defined(RENDERER_PIPELINE_PSX)
	Matrix34 invertedObjectMatrix;
	invertedObjectMatrix[3] = Vector3::zero;
	invertedObjectMatrix.InverseDetOne(objectPosition);			// rotate lights into local coordinate space
	MATRIX llm;
	for(int index=0;index < 3;index++)
	{
		Vector3 worldCoord(_dirLightDirections[index] * invertedObjectMatrix);
		*((Vector3_PS*)(&llm.m[index])) = Vector3ToPS12(worldCoord);
	}
	gte_SetLightMatrix(&llm);
	Matrix34 temp(objectPosition);
	temp *= _invertedPosition;
	object.Render(_viewPort,temp);
#elif defined(RENDERER_PIPELINE_GL)
//#error kts write code here
	Matrix34 invertedObjectMatrix;
	invertedObjectMatrix[3] = Vector3::zero;
	invertedObjectMatrix.InverseDetOne(objectPosition);			// rotate lights into local coordinate space
    //cout << "Object position: " << objectPosition << std::endl;
    //cout << "invertedObjectMatrix = " << invertedObjectMatrix << std::endl;
    invertedObjectMatrix[3] = Vector3::zero;

	Matrix34 temp(objectPosition);
	temp *= _invertedPosition;

   //cout << "Final matrix: " << temp << endl;
	object.Render(_viewPort,temp);

#elif defined(RENDERER_PIPELINE_DIRECTX)
	object.Render(_viewPort,objectPosition);
#else
#error renderer pipeline not defined!
#endif
    DBSTREAM1( cgfx<< "RenderCamera::RenderObject done" << std::endl; )
}

//============================================================================

void
RenderCamera::RenderMatte(ScrollingMatte& _matte, const TileMap& map, Scalar xMult, Scalar yMult)
{
	_viewPort.Validate();
//	cout << "RenderCamera::RenderMatte:" << std::endl;
	assert(_renderInProgress);

	Vector3 probe(Scalar::zero, Scalar::zero, Scalar::one);
	Matrix34 rotOnly(_invertedPosition);
	rotOnly[3] = Vector3::zero;
	probe *= rotOnly;
	// kts vector to euler
	Scalar x( probe[0]);
	Scalar y( probe[1]);
//	Scalar z( probe[2]);

//	FntPrint("vector = \n");
//	FntPrintScalar(probe[0]);
//	FntPrint("\n");
//	FntPrintScalar(probe[1]);
//	FntPrint("\n");
//	FntPrintScalar(probe[2]);

	Angle theta( y.ATan2( x ) );
	probe.RotateZ( -theta );
	Angle phi( probe.X().ATan2( probe.Z() ) - Angle::Revolution( SCALAR_CONSTANT( 0.25 ) ) );
	Angle r( Angle::zero );

//	FntPrint("euler = \n");
//	FntPrintScalar(r.AsScalar());
//	FntPrint("\n");
//	FntPrintScalar(phi.AsScalar());
//	FntPrint("\n");
//	FntPrintScalar(theta.AsScalar());

	Euler camEuler(r,phi,theta);

	Scalar xSize(map.xSize*ScrollingMatte::TILE_SIZE,0);
	Scalar ySize(map.ySize*ScrollingMatte::TILE_SIZE,0);
	int xOffset = -(xMult * xSize * camEuler.GetC().AsRevolution()).WholePart();
	int yOffset = -(yMult * ySize * camEuler.GetB().AsRevolution()).WholePart();

//	cscreen << "xOffset = " << xOffset << std::endl;
//	cscreen << "yOffset = " << yOffset << std::endl;

//	xOffset = 0;			            // kts temp
//	yOffset = 0;
#if defined(DO_STEREOGRAM)
#pragma message ("KTS " __FILE__ ": kludge, do proper math for mattes")
	if(_viewPort.GetConstructionOrderTableIndex() & 1)
		xOffset -= 30;
#endif

	_matte.Render(_viewPort,map,xOffset,yOffset);
//	cout << "RenderCamera::RenderObject: done" << std::endl;
}

//============================================================================

