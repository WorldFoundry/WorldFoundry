//=============================================================================
// Material.cc:
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

#include <gfx/material.hp>
#include <gfx/rendobj2.hp>
#include <gfx/rendobj3.hp>

//============================================================================

Texture emptyTexture = {"",0,320,0,0,0,0,0,0,16};

//============================================================================

Material::Material()
{
}

//============================================================================

Material::Material(const _MaterialOnDisk& mod, const Texture& texture, const PixelMap* texturePixelMap)
:
	_color(mod._color),
	// I should add a smoothing group flag
	_materialFlags(mod._materialFlags),
	_texture(texture),
    _texturePixelMap(texturePixelMap)
{
	ValidateRenderMask(_materialFlags);
	Construct();
	Validate();
}

//============================================================================

pRenderObj3DFunc
Material::Get3DRenderObjectPtr() const
{
	static const pRenderObj3DFunc _rendererList[] =
	{
		#if defined(RENDERER_PIPELINE_SOFTWARE)
			#include <gfx/softwarepipeline/renderer.ext>
		#elif defined( RENDERER_PIPELINE_PSX )
			#include <gfx/psx/renderer.ext>
		#elif defined( RENDERER_PIPELINE_GL )
			#include <gfx/glpipeline/renderer.ext>
		#elif defined( RENDERER_PIPELINE_DIRECTX )
			#include <gfx/directxpipeline/renderer.ext>
		#else
			#error No platform...
		#endif
	};
	ValidateRenderMask(_materialFlags);
	return _rendererList[_materialFlags&RENDERER_SELECTION_MASK];
}

//============================================================================

Material::Material(Color color, int materialFlags,const Texture& texture,const PixelMap* texturePixelMap)
	:
	_color(color),
#if defined(RENDERER_PIPELINE_SOFTWARE)
	_materialFlags(materialFlags|Material::GOURAUD_SHADED),
#else
   _materialFlags(materialFlags),
#endif
	_texture(texture),
    _texturePixelMap(texturePixelMap)

{
	ValidateRenderMask(_materialFlags);
	Construct();
	Validate();
}

//============================================================================

//enum
//{
//	TEXTURE_TRANS_HALF_BACK_HALF_PRIMITIVE,
//	TEXTURE_TRANS_ONE_BACK_ONE_PRIMITIVE,
//	TEXTURE_TRANS_ONE_BACK_NEGATIVE_ONE_PRIMITIVE,
//	TEXTURE_TRANS_ONE_BACK_QUARTER_PRIMITIVE
//};

//-----------------------------------------------------------------------------

void
Material::Construct()
{

//#if defined(__WIN__)
//	_materialFlags &= ~Material::TEXTURE_MAPPED;
//#endif

   // kts kludge until all models have been updated to be flat shaded

#if defined(RENDERER_PIPELINE_SOFTWARE)
		// kts this is so fogging will always be smooth
      _materialFlags |= GOURAUD_SHADED;
#elif defined( RENDERER_PIPELINE_PSX )
		// kts this is so fogging will always be smooth
      _materialFlags |= GOURAUD_SHADED;
#elif defined( RENDERER_PIPELINE_GL )
   if(_materialFlags & GOURAUD_SHADED)
      _materialFlags &= ~GOURAUD_SHADED;
#elif defined( RENDERER_PIPELINE_DIRECTX )
	#error hasnt been implemented yet, not sure what it will need	
#else
	#error No platform...
#endif

	// kts kludge until we can get translucency from material
	if(_materialFlags & TEXTURE_MAPPED)
	{
		if(_texture.bTranslucent)
			_materialFlags |= TEXTURE_TRANSLUCENCY_HALF_BACK_HALF_PRIMITIVE;
	}

	_renderer3D = Get3DRenderObjectPtr();

	switch ( _materialFlags & POLYGON_TYPE_FLAGS )
	{
		case FLAT_SHADED|SOLID_COLOR:
		{
#if defined(RENDERER_PIPELINE_SOFTWARE)
			assert(0);              // software pipeline needs all polys to be gouraud shaded so that fogging will work
#endif
			POLY_F3 poly;
			setPolyF3(&poly);
			Color color = GetColor();
			setRGB0(&poly, color.Red(),color.Green(),color.Blue());
			_cdColor = *((P_CODE*)&poly.r0);  // copy cd & color
			break;
		}
		case GOURAUD_SHADED|SOLID_COLOR:
		{
#if !defined(RENDERER_PIPELINE_SOFTWARE)
//         assert(0);           // kts we don't seem to have color by vertex finished
#endif
			POLY_G3 poly;
			setPolyG3(&poly);
			Color color = GetColor();
			setRGB0(&poly, color.Red(),color.Green(),color.Blue());
			setRGB1(&poly, color.Red(),color.Green(),color.Blue());
			setRGB2(&poly, color.Red(),color.Green(),color.Blue());
			_cdColor = *((P_CODE*)&poly.r0);  // copy cd & color
			break;
		}
		case FLAT_SHADED|TEXTURE_MAPPED:
		{
#if defined(RENDERER_PIPELINE_SOFTWARE)
			assert(0);              // software pipeline needs all polys to be gouraud shaded so that fogging will work
#endif
			POLY_FT3 poly;
			setPolyFT3(&poly);
			Color color = GetColor();
			setRGB0((&poly), color.Red(),color.Green(),color.Blue());
			_cdColor = *((P_CODE*)&poly.r0);  // copy cd & color
			break;
		}
		case GOURAUD_SHADED|TEXTURE_MAPPED:
		{
#if !defined(RENDERER_PIPELINE_SOFTWARE)
         //assert(0);           // kts we don't seem to have color by vertex finished
#endif
			POLY_GT3 poly;
			setPolyGT3(&poly);
			Color color = GetColor();
			setRGB0(&poly, color.Red(),color.Green(),color.Blue());
			setRGB1(&poly, color.Red(),color.Green(),color.Blue());
			setRGB2(&poly, color.Red(),color.Green(),color.Blue());
			_cdColor = *((P_CODE*)&poly.r0);  // copy cd & color
			break;
		}
		default:
			AssertMsg( 0, "_materialFlags = " << std::hex << _materialFlags );
			break;
	}
}

//-----------------------------------------------------------------------------

#define CLIP01(u) \
	if(u > Scalar::one) \
	{ \
		DBSTREAM5( cout << "clipping uv coordinate to 0-1" << std::endl; ) \
		u = Scalar::one; \
	} \
	if(u <  Scalar::zero) \
	{ \
		DBSTREAM5( cout << "clipping uv coordinate to 0-1" << std::endl; ) \
		u = Scalar::zero; \
	} \
	assert(u <= Scalar::one); \
	assert(u >= Scalar::zero);


#define DirectXCalcVRAMuv(uin,vin,resultu,resultv) \
	{ \
	Scalar u(uin); \
	Scalar v(vin); \
	CLIP01(u); \
	CLIP01(v); \
	Scalar vramU = (u *	int(_texture.w-1)) + Scalar(_texture.u,0); \
	AssertMsg(vramU < SCALAR_CONSTANT(Display::VRAMWidth),"u = " << u << ", width= " << _texture.w << ", u = " << _texture.u); \
	AssertMsg(vramU >= Scalar::zero,"u = " << u << ", width= " << _texture.w << ", u = " << _texture.u << ", vramU = " << vramU); \
	resultu = float(vramU.WholePart())/Display::VRAMWidth; \
	Scalar vramV = (v * int(_texture.h-1)) + Scalar(_texture.v,0); \
	AssertMsg(vramV < SCALAR_CONSTANT(Display::VRAMHeight),"vramV = " << vramV); \
	AssertMsg(vramV >= Scalar::zero,"v = " << v << ", height= " << _texture.h << ", v = " << _texture.v << ", vramV = " << vramV); \
	resultv = float(vramV.WholePart())/Display::VRAMHeight; \
	}

//-----------------------------------------------------------------------------

#if defined(RENDERER_PIPELINE_DIRECTX)
void
Material::InitPrimitive(TriFace& face, const Vertex3D& vertex0, const Vertex3D& vertex1, const Vertex3D& vertex2) const
{
	if(_materialFlags & TEXTURE_MAPPED)
	{
#pragma message ("KTS: handle 4 & 8 bit textures as soon as I get data from textile")

		Scalar minU = Scalar( vertex0.u.Min(vertex1.u).Min(vertex2.u).WholePart(), 0 );
		Scalar minV = Scalar( vertex0.v.Min(vertex1.v).Min(vertex2.v).WholePart(), 0 );

		assert(minU <= vertex0.u);
		assert(minU <= vertex1.u);
		assert(minU <= vertex2.u);
		assert(minV <= vertex0.v);
		assert(minV <= vertex1.v);
		assert(minV <= vertex2.v);

		float u0;
		float v0;
		DirectXCalcVRAMuv(vertex0.u-minU,vertex0.v-minV,u0,v0);

		float u1;
		float v1;
		DirectXCalcVRAMuv(vertex1.u-minU,vertex1.v-minV,u1,v1);

		float u2;
		float v2;
		DirectXCalcVRAMuv(vertex2.u-minU,vertex2.v-minV,u2,v2);

		face._u[0] = u0;
		face._v[0] = v0;
		face._u[1] = u1;
		face._v[1] = v1;
		face._u[2] = u2;
		face._v[2] = v2;
	}
}

#endif

//-----------------------------------------------------------------------------

#define TEXTURE_PAGE_XSIZE 256
#define TEXTURE_PAGE_XSTART_BOUNDRY 64
#define TEXTURE_PAGE_YSIZE 256
#define TEXTURE_PAGE_YSTART_BOUNDRY 256


#define WFdumpTPage(tpage)						\
	((GetGraphType()==1||GetGraphType()==2)?			\
	printf("tpage: (%d,%d,%d,%d)\n",				\
			   ((tpage)>>9)&0x003,((tpage)>>7)&0x003,	\
			   ((tpage)<<6)&0x7c0,((tpage)<<3)&0x300):	\
	printf("tpage: (%d,%d,%d,%d)\n",				\
			   ((tpage)>>7)&0x003,((tpage)>>5)&0x003,	\
			   ((tpage)<<6)&0x7c0,				\
			   (((tpage)<<4)&0x100)+(((tpage)>>2)&0x200)))



#define CalcVRAMuv(uin,vin,resultu,resultv,texture) \
	{ \
	Scalar u(uin); \
	Scalar v(vin); \
	CLIP01(u); \
	CLIP01(v); \
	Scalar vramU = (u *	int(texture.w-1)) + Scalar(texture.u%TEXTURE_PAGE_XSTART_BOUNDRY,0); \
	AssertMsg(vramU < SCALAR_CONSTANT(TEXTURE_PAGE_XSIZE),"u = " << u << ", width= " << texture.w << ", u = " << texture.u); \
	AssertMsg(vramU >= Scalar::zero,"u = " << u << ", width= " << texture.w << ", u = " << texture.u << ", vramU = " << vramU); \
	resultu = vramU.WholePart(); \
	Scalar vramV = (v * int(texture.h-1)) + Scalar(texture.v%TEXTURE_PAGE_YSTART_BOUNDRY,0); \
	AssertMsg(vramV < SCALAR_CONSTANT(TEXTURE_PAGE_YSIZE),"vramV = " << vramV); \
	AssertMsg(vramV >= Scalar::zero,"v = " << v << ", height= " << texture.h << ", v = " << texture.v << ", vramV = " << vramV); \
	resultv = vramV.WholePart(); \
	}

//-----------------------------------------------------------------------------

void
Material::InitPrimitive(Primitive& prim, const Vertex3D& vertex0, const Vertex3D& vertex1, const Vertex3D& vertex2) const
{
    Validate();
	switch ( _materialFlags & POLYGON_TYPE_FLAGS )
	{
		case FLAT_SHADED|SOLID_COLOR:
		{
			POLY_F3* poly = (POLY_F3*)&prim;
			setPolyF3(poly);
			Color color = GetColor();
			setRGB0(poly, color.Red(),color.Green(),color.Blue());
//			cout << "-- F3 RGB = " << color << std::endl;
			break;
		}
		case GOURAUD_SHADED|SOLID_COLOR:
		{
			POLY_G3* poly = (POLY_G3*)&prim;
			setPolyG3(poly);
			setRGB0(poly, vertex0.color.Red(),vertex0.color.Green(),vertex0.color.Blue());
			setRGB1(poly, vertex1.color.Red(),vertex1.color.Green(),vertex1.color.Blue());
			setRGB2(poly, vertex2.color.Red(),vertex2.color.Green(),vertex2.color.Blue());
			break;

		}
		case FLAT_SHADED|TEXTURE_MAPPED:
		{
			POLY_FT3* poly = (POLY_FT3*)&prim;
			setPolyFT3(poly);
			Color color = GetColor();
         color = Color::white;            // kts temp until all models are updated to have proper colors on their textured materials
			setRGB0(poly, color.Red(),color.Green(),color.Blue());
#pragma message ("KTS: handle 4 & 8 bit textures as soon as I get data from textile")
//			poly->tpage = getTPage(TEXTURE_MODE_16BIT_DIRECT,TEXTURE_TRANS_HALF_BACK_HALF_PRIMITIVE,_texture.u,_texture.v);
			poly->tpage = getTPage(TEXTURE_MODE_16BIT_DIRECT,((_materialFlags>>8)&0x3),_texture.u,_texture.v);

            poly->pPixelMap = _texturePixelMap;

			if(_materialFlags & TEXTURE_TRANSLUCENCY_ON)
				setSemiTrans(poly,1);

//			cout << "vertex uv:" <<
//				vertex0.u << "," << vertex0.v << ";" <<
//				vertex1.u << "," << vertex1.v << ";" <<
//				vertex2.u << "," << vertex2.v << std::endl;

//			WFdumpTPage(poly->tpage);
//			cout << "Texture:" << _texture << std::endl;

			Scalar minU = vertex0.u.Min(vertex1.u).Min(vertex2.u);
			Scalar minV = vertex0.v.Min(vertex1.v).Min(vertex2.v);

			Scalar maxU = vertex0.u.Max(vertex1.u).Max(vertex2.u);
			Scalar maxV = vertex0.v.Max(vertex1.v).Max(vertex2.v);

//			cout << "minUV = " << minU << "," << minV << std::endl;
//			cout << "maxUV = " << maxU << "," << maxV << std::endl;

			Scalar offsetU = minU;
			Scalar offsetV = minV;
			if(maxU-minU > Scalar::one)
			{
				offsetU = (minU+maxU)/Scalar::two;
				// + SCALAR_CONSTANT(0.5);
			}

			if(maxV-minV > Scalar::one)
			{
 				offsetV = (minV+maxV)/Scalar::two;
				// + SCALAR_CONSTANT(0.5);
			}

			offsetU = Scalar(offsetU.WholePart(),0);
			offsetV = Scalar(offsetV.WholePart(),0);
//			cout << "minUV = " << offsetU << "," << offsetV << std::endl;

			int8 u0;
			int8 v0;
			CalcVRAMuv(vertex0.u-offsetU,vertex0.v-offsetV,u0,v0,_texture);

			int8 u1;
			int8 v1;
			CalcVRAMuv(vertex1.u-offsetU,vertex1.v-offsetV,u1,v1,_texture);

			int8 u2;
			int8 v2;
			CalcVRAMuv(vertex2.u-offsetU,vertex2.v-offsetV,u2,v2,_texture);


//			cout << "uv: " <<
//			u0 << "," << v0 << ";" <<
//			u1 << "," << v1 << ";" <<
//			u2 << "," << v2 << std::endl;

			setUV3(poly,u0,v0,u1,v1,u2,v2);
			break;
		}
		case GOURAUD_SHADED|TEXTURE_MAPPED:
		{
//			cout << "Gouraud shadded texture map:" << std::endl;
//			cout << "v0 = " << vertex0 << "; v1 = " << vertex1 << "; v2 " << vertex2 << std::endl;
//			cout << *this << std::endl;

			POLY_GT3* poly = (POLY_GT3*)&prim;
			setPolyGT3(poly);
			setRGB0(poly, vertex0.color.Red(),vertex0.color.Green(),vertex0.color.Blue());
			setRGB1(poly, vertex1.color.Red(),vertex1.color.Green(),vertex1.color.Blue());
			setRGB2(poly, vertex2.color.Red(),vertex2.color.Green(),vertex2.color.Blue());
			poly->tpage = getTPage(TEXTURE_MODE_16BIT_DIRECT,((_materialFlags>>8)&0x3),_texture.u,_texture.v);
            poly->pPixelMap = _texturePixelMap;

			if(_materialFlags & TEXTURE_TRANSLUCENCY_ON)
				setSemiTrans(poly,1);

			Scalar minU = vertex0.u.Min(vertex1.u).Min(vertex2.u);
			Scalar minV = vertex0.v.Min(vertex1.v).Min(vertex2.v);

			Scalar maxU = vertex0.u.Max(vertex1.u).Max(vertex2.u);
			Scalar maxV = vertex0.v.Max(vertex1.v).Max(vertex2.v);

//			cout << "minUV = " << minU << "," << minV << std::endl;
//			cout << "maxUV = " << maxU << "," << maxV << std::endl;

			Scalar offsetU = minU;
			Scalar offsetV = minV;
			if(maxU-minU > Scalar::one)
			{
				offsetU = (minU+maxU)/Scalar::two;
				// + SCALAR_CONSTANT(0.5);
			}

			if(maxV-minV > Scalar::one)
			{
 				offsetV = (minV+maxV)/Scalar::two;
				// + SCALAR_CONSTANT(0.5);
			}

			offsetU = Scalar(offsetU.WholePart(),0);
			offsetV = Scalar(offsetV.WholePart(),0);
//			cout << "minUV = " << offsetU << "," << offsetV << std::endl;

			int8 u0;
			int8 v0;
			CalcVRAMuv(vertex0.u-offsetU,vertex0.v-offsetV,u0,v0,_texture);

			int8 u1;
			int8 v1;
			CalcVRAMuv(vertex1.u-offsetU,vertex1.v-offsetV,u1,v1,_texture);

			int8 u2;
			int8 v2;
			CalcVRAMuv(vertex2.u-offsetU,vertex2.v-offsetV,u2,v2,_texture);

//			cout << "uv's " <<
//				u0 << "," << v0 << ";" <<
//				u1 << "," << v1 << ";" <<
//				u2 << "," << v2 << std::endl;
			setUV3(poly,u0,v0,u1,v1,u2,v2);

#if 0
			cout << "vertex uv:" <<
				vertex0.u << "," << vertex0.v << ";" <<
				vertex1.u << "," << vertex1.v << ";" <<
				vertex2.u << "," << vertex2.v << std::endl;

			WFdumpTPage(poly->tpage);
			cout << "Texture:" << _texture << std::endl;
			cout << "uv: " <<
				u0 << "," << v0 << ";" <<
				u1 << "," << v1 << ";" <<
				u2 << "," << v2 << std::endl;
#endif



			break;
		}
		default:
			AssertMsg( 0, "_materialFlags = " << std::hex << _materialFlags );
			break;
	}
//#endif
}

//============================================================================

//Material::~Material()
//{
//}

//============================================================================
//============================================================================
