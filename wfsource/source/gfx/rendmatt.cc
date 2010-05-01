//=============================================================================
// Rendmatt.cc: render matte (2d scrolling background)
// Copyright ( c ) 1998,1999,2000,2001 World Foundry Group  
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

#include <gfx/rendmatt.hp>
#include <gfx/viewport.hp>
#include <gfx/material.hp>

#if defined ( __PSX__ )
#	include <sys/types.h>
#	include <libetc.h>
#	include <libgte.h>
#	include <libgpu.h>
#	include <inline_c.h>
#elif defined ( __WIN__ )
#	include <new.h>
#pragma message( "move to correct place when completed" )
void SetDrawMode(DR_MODE *p, int dfe, int dtd, int tpage, RECT *tw)
{
}
#elif defined ( __LINUX__ )
#pragma message( "move to correct place when completed" )
void SetDrawMode(DR_MODE* /*p*/, int /*dfe*/, int /*dtd*/, int /*tpage*/, psxRECT* /*tw*/)
{
}
#else
#error OS not defined!
#endif

//=============================================================================

ScrollingMatte::ScrollingMatte(const char* textureName, const Texture& texture, int32 userData, const GfxCallbacks& callbacks, int depth) :
	_texture(texture),_depth(depth)
{
#if defined(USE_ORDER_TABLES)
	for(int xIndex=0;xIndex<MATTE_XSIZE;xIndex++)
		for(int yIndex=0;yIndex<MATTE_YSIZE;yIndex++)
#pragma message( "Move this to the outer loop" )
			for(int page=0;page<ORDER_TABLES;page++)
			{
				SPRT_16& sprite = mem[page][xIndex][yIndex];
				setSprt16(&sprite); 				// set SPRT_16 primitve ID
				setSemiTrans(&sprite, 0);			// semi-amibient is OFF
				setRGB0(&sprite, 0x80, 0x80, 0x80); // half color
			}
	for(int page=0;page<ORDER_TABLES;page++)
		SetDrawMode(&dr_mode[page], 0, 0, getTPage(TEXTURE_MODE_16BIT_DIRECT, 0, _texture.u, _texture.v), 0);
#else
   AssertMsg(strlen(textureName),"invalid texture name in matte:" << textureName);
   LookupTextureStruct lts = callbacks.LookupTexture(textureName,userData);

   texturePixelMap = &lts.texturePixelMap;
#endif
}

//=============================================================================

#define CalcVRAMuv(uin,vin,resultu,resultv,texture) \
	{ \
	Scalar u(uin); \
	Scalar v(vin); \
	Scalar vramU = (u *	int(texture.w-1)) + Scalar(texture.u,0); \
	AssertMsg(vramU >= Scalar::zero,"u = " << u << ", width= " << texture.w << ", u = " << texture.u << ", vramU = " << vramU); \
	resultu = vramU.WholePart(); \
	Scalar vramV = (v * int(texture.h-1)) + Scalar(texture.v,0); \
	AssertMsg(vramV >= Scalar::zero,"v = " << v << ", height= " << texture.h << ", v = " << texture.v << ", vramV = " << vramV); \
	resultv = vramV.WholePart(); \
	}

static void 
CalcAndSetUV(unsigned char uin, unsigned char vin, const PixelMap& texturePixelMap) 
{
    ulong u(uin); 
    ulong v(vin); 

#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)
    float uResult(float(u)/VRAM_WIDTHF);                            
    float vResult(float(v)/VRAM_HEIGHTF);                           
#else
    float uResult(float(u)/texturePixelMap.GetBaseXSize());                            
    float vResult(float(v)/texturePixelMap.GetBaseYSize());
#endif
    glTexCoord2f(uResult, vResult);                         
}

//==============================================================================

void
ScrollingMatte::Render(ViewPort& 
#if defined(USE_ORDER_TABLES)
   vp,
#else
   /*vp*/,
#endif
    const TileMap& map,int xOffset, int yOffset)
{
	while(xOffset < 0)
	{
		xOffset += (map.xSize*TILE_SIZE);
	}

	while(yOffset < 0)
	{
		yOffset += (map.ySize*TILE_SIZE);
	}

	int xSmoothScroll = (xOffset % TILE_SIZE);
	int xCoarseScroll = xOffset / 16;
	int ySmoothScroll = (yOffset % TILE_SIZE);
	int yCoarseScroll = yOffset / 16;

	RangeCheckExclusive(-17,xSmoothScroll,TILE_SIZE);
	RangeCheckExclusive(-17,ySmoothScroll,TILE_SIZE);

	ASSERTIONS( int loopCounter=0; )
	while(xCoarseScroll < 0)
	{
		assert(loopCounter++ < 10);
		xCoarseScroll += 1000*map.xSize;
	}
	ASSERTIONS( loopCounter=0; )
	while(yCoarseScroll < 0)
	{
		assert(loopCounter++ < 10);
		yCoarseScroll += 1000*map.ySize;
	}

	assert(xCoarseScroll >= 0);
	assert(yCoarseScroll >= 0);
//	cout << xCoarseScroll << "," << yCoarseScroll << ";" << xSmoothScroll << "," << ySmoothScroll << endl;

#if defined(USE_ORDER_TABLES)
#else
   glLoadIdentity();
   glEnable( GL_TEXTURE_2D );
   texturePixelMap->SetGLTexture();
   glDisable(GL_LIGHTING);
   glDisable(GL_FOG);
   glColor3f(1.0, 1.0, 1.0);
#endif


	int tileModulo = _texture.w/16;
#if 1
#if 0
   for(int yIndex=2;yIndex<3;yIndex++)
      for(int xIndex=2;xIndex<3;xIndex++)
#else
   for(int yIndex=0;yIndex<MATTE_YSIZE;yIndex++)
      for(int xIndex=0;xIndex<MATTE_XSIZE;xIndex++)
#endif
		{
			RangeCheck(0,((xIndex+xCoarseScroll)% map.xSize),map.xSize);
			RangeCheck(0,((yIndex+yCoarseScroll)% map.ySize),map.ySize);

			const char* mapPtr = &map.map +
				(((yIndex+yCoarseScroll)% map.ySize)*map.xSize) +
				((xIndex+xCoarseScroll) % map.xSize)
				;

         //std::cout << "x portion of offset: " << ((xIndex+xCoarseScroll) % map.xSize) << ", y portion of offset: " << (((yIndex+yCoarseScroll)% map.ySize)*map.xSize) << std::endl;
			int tileIndex = *mapPtr;

			if(tileIndex != 0xff)
			{
            //tileIndex = 12;
				int xMapIndex = tileIndex % tileModulo;
				int yMapIndex = tileIndex / tileModulo;
            //std::cout << "xIndex:" << xIndex << ", yIndex:" << yIndex << ", xMapIndex:" << xMapIndex << ", yMapIndex:" << yMapIndex << ", tileIndex:" << tileIndex << std::endl;

#if defined(USE_ORDER_TABLES)
				SPRT_16& sprite = mem[vp.GetConstructionOrderTableIndex()][xIndex][yIndex];
				setXY0(&sprite, (xIndex*16)-xSmoothScroll, (yIndex*16)-ySmoothScroll);
				setUV0(&sprite, xMapIndex*16, yMapIndex*16);
				vp.AddPrimitive(*(Primitive*)&sprite,_depth);
#else

            glBegin( GL_QUADS );
            const int XOFFSET = - ((320/2)+16);
            const int YOFFSET = - ((224/2)+16+(16/2));         // add a tile & a half
            //const int ZOFFSET = -350;                     // switch to this one to see the entire map
            const int ZOFFSET = -277;

            int uResult,vResult;

            Scalar xTileOffset = Scalar(xMapIndex*16,0)/Scalar(_texture.w,0);
            Scalar yTileOffset = Scalar(_texture.h-(yMapIndex*16),0)/Scalar(_texture.h,0);
            Scalar xTileOffsetRight = Scalar((xMapIndex*16)+16,0)/Scalar(_texture.w,0);
            Scalar yTileOffsetBottom = Scalar(_texture.h-((yMapIndex*16)+16),0)/Scalar(_texture.h,0);

            //std::cout << "  xTileOffset = " << xTileOffset << ", xTileOffsetRight = " << xTileOffsetRight << std::endl;
            //std::cout << "  yTileOffset = " << yTileOffset << ", yTileOffsetBottom = " << yTileOffsetBottom <<std::endl;

            CalcVRAMuv(xTileOffset,yTileOffset,uResult,vResult,_texture);
            CalcAndSetUV(uResult,vResult,*texturePixelMap);
            glVertex3i( ((xIndex*16)-xSmoothScroll)+XOFFSET,    -(((yIndex*16)-ySmoothScroll)+YOFFSET),    ZOFFSET );
            CalcVRAMuv(xTileOffset,yTileOffsetBottom,uResult,vResult,_texture);
            CalcAndSetUV(uResult,vResult,*texturePixelMap);
            glVertex3i( ((xIndex*16)-xSmoothScroll)+XOFFSET,    -(((yIndex*16)-ySmoothScroll)+YOFFSET+16), ZOFFSET );
            CalcVRAMuv(xTileOffsetRight,yTileOffsetBottom,uResult,vResult,_texture);
            CalcAndSetUV(uResult,vResult,*texturePixelMap);
            glVertex3i( ((xIndex*16)-xSmoothScroll)+XOFFSET+16, -(((yIndex*16)-ySmoothScroll)+YOFFSET+16), ZOFFSET );
            CalcVRAMuv(xTileOffsetRight,yTileOffset,uResult,vResult,_texture);
            CalcAndSetUV(uResult,vResult,*texturePixelMap);
            glVertex3i( ((xIndex*16)-xSmoothScroll)+XOFFSET+16, -(((yIndex*16)-ySmoothScroll)+YOFFSET),    ZOFFSET );
            glEnd();
            AssertGLOK();
#endif
			}
		}
#endif
#if defined(USE_ORDER_TABLES)
	vp.AddPrimitive(*(Primitive*)&dr_mode[vp.GetConstructionOrderTableIndex()],_depth);
#else


#if 0
   // kts test code
   //glDisable(GL_TEXTURE_2D);
   glBegin( GL_TRIANGLES );
   const int XOFFSET = -176;
   const int YOFFSET = -100;
   //const int ZOFFSET = -350;
   const int ZOFFSET = -350;

   static int foo=0;
   foo++;
   if(foo > 100)
      foo = 0;



   glTexCoord2f(0, 0); 
   glVertex3i( XOFFSET+foo, YOFFSET, ZOFFSET );                    // bottom
   glTexCoord2f(0, 0.2); 
   glVertex3i( XOFFSET, YOFFSET+200, ZOFFSET );            // upper left
   glTexCoord2f(0.2, 0.2); 
   glVertex3i( XOFFSET+200, YOFFSET+200, ZOFFSET );            // upper right
   //glTexCoord2f(0.2, 0); 
   //glVertex3i( XOFFSET+200, YOFFSET, ZOFFSET );
   glEnd();
   AssertGLOK();
#endif

   glEnable(GL_LIGHTING);
   glEnable(GL_FOG);
#endif
}

//=============================================================================
