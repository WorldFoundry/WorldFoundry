//==============================================================================
// wfprim.h
//==============================================================================

#ifndef GFX_WFPRIM_H
#define GFX_WFPRIM_H

//==============================================================================


#if DO_ASSERTIONS

#if SW_DBSTREAM
                      
#define AssertGLOK()        \
{                           \
    GLenum glError = glGetError(); \
    if(glError != GL_NO_ERROR) \
    { \
        while(glError != GL_NO_ERROR) \
        { \
            cerror << "GL error: " << glError; \
            glError = glGetError(); \
        } \
        assert(0); \
    } \
}
#else
                      
#define AssertGLOK()        \
{                           \
    GLenum glError = glGetError(); \
    if(glError != GL_NO_ERROR) \
    { \
        while(glError != GL_NO_ERROR) \
        { \
            glError = glGetError(); \
        } \
        assert(0); \
    } \
}
#endif

#else // DO_ASSERTIONS
#define AssertGLOK()
#endif

//==============================================================================

          
class PixelMap;
          
typedef struct 
{
	unsigned char  r0, g0, b0, code;
} P_CODE;

typedef struct P_TAG 
{
	struct P_TAG*	addr;
//      unsigned        addr: 24;
//      unsigned        len:   8;
	unsigned char          r0, g0, b0, code;
} P_TAG;

typedef struct 
{
	P_TAG* tag;
	unsigned char  r0, g0, b0, code;
	short   x0,     y0;
	short   x1,     y1;
	short   x2,     y2;
} POLY_F3;                              // Flat Triangle 

typedef struct 
{
	P_TAG* tag;
	unsigned char  r0, g0, b0, code;
	short   x0,     y0;
	unsigned char  r1, g1, b1, pad1;
	short   x1,     y1;
	unsigned char  r2, g2, b2, pad2;
	short   x2,     y2;
} POLY_G3;                              // Gouraud Triangle 

typedef struct 
{
	P_TAG* tag;
	unsigned char  r0, g0, b0, code;
	short   x0,     y0;
	unsigned char  u0, v0; unsigned short clut;
	unsigned char  r1, g1, b1, p1;
	short   x1,     y1;
	unsigned char  u1, v1; unsigned short tpage;
	unsigned char  r2, g2, b2, p2;
	short   x2,     y2;
	unsigned char  u2, v2; unsigned short pad2;
    const PixelMap* pPixelMap;
} POLY_GT3;                             // Gouraud Textured Triangle 

typedef struct 
{
	P_TAG* tag;
	unsigned char  r0, g0, b0, code;
	short   x0,     y0;
	unsigned char  u0, v0; unsigned short clut;
	short   x1,     y1;
	unsigned char  u1, v1; unsigned short tpage;
	short   x2,     y2;
	unsigned char  u2, v2; unsigned short pad1;
    const PixelMap* pPixelMap;
} POLY_FT3;                             // Flat Textured Triangle

typedef struct 
{
	short x, y;		// offset into dest image
	short w, h;		// width and height 
} psxRECT;

typedef struct 
{
	unsigned long	tag;
	unsigned char	r0, g0, b0, code;
	short	x0, 	y0;
	unsigned char	u0, v0;	unsigned short	clut;
} SPRT_16;				/* 16x16 Sprite */

typedef struct 
{
	unsigned long	tag;
	unsigned long	code[2];
} DR_MODE;				// Drawing Mode 

typedef struct 
{
	int x, y;		// offset into dest image
	int w, h;		// width and height
} psxRECT32;


enum
{
	CODE_NOP = 0,
	CODE_POLY_F3 = 0x20,
	CODE_POLY_FT3 = 0x24,
	CODE_POLY_G3 = 0x30,
	CODE_POLY_GT3 = 0x34,
	CODE_SPRT_16 = 0x7C,
};

typedef struct                // short word type 3D vector 
{
	short   vx, vy;
	short   vz, pad;
} SVECTOR;


#define setlen( p, _len)                (0)
	//(((P_TAG *)(p))->len  = (unsigned char)(_len))

inline P_TAG*
setaddr( P_TAG* p, P_TAG* _addr )
{
	return p->addr = _addr;
}

#define setcode(p, _code)               (p)->code = (_code)
#define setSemiTrans(p, abe) 			((abe)?setcode(p, getcode(p)|0x02):setcode(p, getcode(p)&~0x02))

#define addPrim(ot, p)                  setaddr(p, getaddr(ot)), setaddr(ot, p)
#define addPrims(ot, p0, p1)    setaddr(p1, getaddr(ot)),setaddr(ot, p0)

//#define getlen(p)                     (unsigned char)(((P_TAG *)(p))->len)
#define getcode(p)                      (unsigned char)(((P_TAG *)(p))->code)
#define getaddr(p)                      (((P_TAG *)(p))->addr)


// tpage encoding:
// short
// bit 15-11 unused
// bit 10-9 mode (TEXTURE_MODE_16BIT_DIRECT)
// bit 8-7 material flags
// bit 5-4 y pos
// bit 3-0 x pos

#define getTPage(tp, abr, x, y)                                         \
/*      ((GetGraphType()==1||GetGraphType()==2)?*/                      \
	 ((((tp)&0x3)<<9)|(((abr)&0x3)<<7)|(((y)&0x300)>>3)|(((x)&0x3ff)>>6))/*:\*/
/*       ((((tp)&0x3)<<7)|(((abr)&0x3)<<5)|(((y)&0x100)>>4)|(((x)&0x3ff)>>6)|(((y)&0x200)<<2)))*/

#define DecodeTPageX(tpage)		(((tpage)<<6)&0x7c0)
#define DecodeTPageY(tpage)		(((tpage)<<3)&0x300)

#define nextPrim(p)						(((P_TAG *)p)->addr)
#define isendprim(p)                    ((((P_TAG *)(p))->addr)==(void*)~0)

#define setPolyF3(p)    setlen(p, 4),  setcode(p, CODE_POLY_F3)
#define setPolyFT3(p)   setlen(p, 7),  setcode(p, CODE_POLY_FT3)
#define setPolyG3(p)    setlen(p, 6),  setcode(p, CODE_POLY_G3)
#define setPolyGT3(p)   setlen(p, 9),  setcode(p, CODE_POLY_GT3)

#define setSprt16(p)	setlen(p, 3),  setcode(p, CODE_SPRT_16)

#define setRECT(r, _x, _y, _w, _h)		(r)->x = (_x),(r)->y = (_y),(r)->w = (_w),(r)->h = (_h)

#define setRGB0(p,_r0,_g0,_b0)  (p)->r0 = _r0,(p)->g0 = _g0,(p)->b0 = _b0
#define setRGB1(p,_r1,_g1,_b1)  (p)->r1 = _r1,(p)->g1 = _g1,(p)->b1 = _b1
#define setRGB2(p,_r2,_g2,_b2)  (p)->r2 = _r2,(p)->g2 = _g2,(p)->b2 = _b2
#define setRGB3(p,_r3,_g3,_b3)  (p)->r3 = _r3,(p)->g3 = _g3,(p)->b3 = _b3

#define setXY0(p,_x0,_y0)						(p)->x0 = (_x0), (p)->y0 = (_y0)
#define setUV0(p,_u0,_v0)						(p)->u0 = (_u0), (p)->v0 = (_v0)

#define setUV3(p,_u0,_v0,_u1,_v1,_u2,_v2)                               \
	(p)->u0 = (_u0), (p)->v0 = (_v0),                               \
	(p)->u1 = (_u1), (p)->v1 = (_v1),                               \
	(p)->u2 = (_u2), (p)->v2 = (_v2)

//void ClearOTagR( uint32* _orderTable, int _depth );
//void DrawOTag( uint32* _orderTable );
void ClearOTagR( P_TAG* _orderTable, int _depth );
void DrawOTag( P_TAG* _orderTable );
//void DrawOTag( Primitive* _orderTable );

int GfxLoadImage( psxRECT* rect, unsigned long* p );

//==============================================================================
#endif  // GFX_WF_PRIM_H
//==============================================================================

