
#include <pigsys/assert.hp>
#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libcd.h>
#include <r3000.h>
#include <asm.h>
#include <kernel.h>

struct Movie
{
	char* szFilename;
	int nFrames;
	int x, y;
};

#define NUM_ITEMS( __array__ )		( sizeof( __array__ ) / sizeof( *__array__) )

struct Movie theMovies[] = {
	{ "\\LOGO.STR;1", 60, 0, 0 },
	{ "\\CS_DEMO.STR;1", 975, 0, 0 },
	{ "\\BALLTENS.STR;1", 134, 0, 0 },
//	{ "\\BALLZ.STR;1", 197, 0, 0 },
	{ "\\BTLDTA01.STR;1", 129, 0, 0 },
	{ "\\BTLDTA02.STR;1", 195, 0, 0 },
	{ "\\BTLDTA03.STR;1", 158, 0, 0 },
	{ "\\BTLDTA05.STR;1", 187, 0, 0 },
	{ "\\COM_POS.STR;1", 153, 0, 0 },
	{ "\\CYCRUN.STR;1", 228, 0, 0 },
	{ "\\DYNBLEND.STR;1", 198, 0, 0 },
//	{ "\\FAN8.STR;1", 4, 0, 0 },
	{ "\\GLCTCD01.STR;1", 73, 0, 0 },
	{ "\\GLCTCD03.STR;1", 257, 0, 0 },
	{ "\\GRAVACL.STR;1", 134, 0, 0 },
//	{ "\\MOV.STR;1", 577, 0, 0 },
	{ "\\SHPDTA01.STR;1", 98, 0, 0 },
	{ "\\SHPDTA02.STR;1", 165, 0, 0 },
	{ "\\STRIDE.STR;1", 207, 0, 0 },
	{ "\\T057662A.STR;1", 201, 0, 0 },
	{ "\\BGSPLSHM.STR;1", 312, 0, 0 },
	{ "\\BTLDTA04.STR;1", 146, 0, 0 },
	{ "\\GLCTCD02.STR;1", 119, 0, 0 },
	{ "\\GRAVACL.STR;1", 134, 0, 0 },
	{ "\\KTX_BOOL.STR;1", 221, 0, 0 }
//	{ "\\SCRLLNGS.STR;1", 634, 0, 0 }
	};

#define START_FRAME 1
#define SCR_WIDTH   320
#define SCR_HEIGHT  240

// 16bit/pixel mode or 24 bit/pixel mode
#define IS_RGB24    1   // 0:RGB16, 1:RGB24
#if IS_RGB24==1
#define PPW 3/2     // pixel/short word :
#define DCT_MODE    3   // 24bit decode
#else
#define PPW 1       // pixel/short word
#define DCT_MODE    2   // 16 bit decode
#endif

//  decode environment
typedef struct {
    u_long*		vlcbuf[2];	// VLC buffer
    int			vlcid;      // current decode buffer id
    u_short*	imgbuf[2];	// decode image buffer
    int			imgid;      // corrently use buffer id
    RECT		rect[2];    // double buffer orign(upper left point) address on the VRAM (double buffer)
    int			rectid;     // currently translating buffer id
    RECT		slice;      // the region decoded by once DecDCTout()
    int			isdone;     // the flag of decoding whole frame
} DECENV;
static DECENV   dec;        // instance of DECENV

// Ring buffer for STREAMING minmum size is two frame
#define RING_SIZE   32      // 32 sectors
static u_long   Ring_Buff[RING_SIZE*SECTOR_SIZE];

//  VLC buffer(double buffer) stock the result of VLC decode
#define VLC_BUFF_SIZE 320/2*256     // not correct value
static u_long   vlcbuf0[VLC_BUFF_SIZE];
static u_long   vlcbuf1[VLC_BUFF_SIZE];

/*
 *  image buffer(double buffer)
 *  stock the result of MDEC
 *  rectangle of 16(width) by XX(height)
 */
#define SLICE_IMG_SIZE 16*PPW*SCR_HEIGHT
static u_short  imgbuf0[ SLICE_IMG_SIZE ];
static u_short  imgbuf1[ SLICE_IMG_SIZE ];

static int  StrWidth  = 0;  /* resolution of movie */
static int  StrHeight = 0;
static int  Rewind_Switch;  /* the end flag set after last frame */

static int anim( struct Movie* );
static void strSetDefDecEnv(DECENV *dec, int x0, int y0, int x1, int y1);
static void strInit(CdlLOC *loc, void (*callback)());
static void strCallback();
static void strNextVlc(DECENV *dec);
static void strSync(DECENV *dec, int mode);
static u_long *strNext(DECENV *dec);
static void strKickCD(CdlLOC *loc);

main()
{
	int i;

    ResetCallback();
    CdInit();
    PadInit( 0 );
    ResetGraph( 0 );
    SetGraphDebug( 0 );
//WBN	FntLoad( 640, 0 );
//WBN	SetDumpFnt( FntOpen (0, 12, 320, 200, 0, 1024 ) );

	for ( ;; )
	{
		for ( i=0; i<NUM_ITEMS( theMovies ); ++i )
		{
			if ( ( i % 5 ) == 4 )
    			anim( &theMovies[0] );

			printf( "Playing movie #%d \"%s\"\n", i, theMovies[i].szFilename );
    		if ( anim( &theMovies[i] ) == 0 )
				;	//return 0;     // animation subroutine
    	}
	}

    PadStop();
    ResetGraph( 3 );
    StopCallback();
    return 0;
}


struct Movie* theMovie;

// animation subroutine forground process
static int
anim( struct Movie* movie )
{
    DISPENV disp;       // display buffer
    DRAWENV draw;       // drawing buffer
    int id;     		// display buffer id
    CdlFILE file;

	assert( movie );
	theMovie = movie;

    // search file
    if ( CdSearchFile( &file, movie->szFilename ) == 0 )
	{
        printf( "file \"%s\" not found\n", movie->szFilename );
		return 0;
    }

    // set the position of vram
    strSetDefDecEnv( &dec, theMovie->x, theMovie->y,
		theMovie->x, theMovie->y+SCR_HEIGHT );

    // init streaming system & kick cd
    strInit( &file.pos, strCallback );

    // VLC decode the first frame
    strNextVlc( &dec );

    Rewind_Switch = 0;

	for ( ;; )
	{
        // start DCT decoding the result of VLC decoded data
        DecDCTin(dec.vlcbuf[dec.vlcid], DCT_MODE);

        // prepare for recieving the result of DCT decode
        // next DecDCTout is called in DecDCToutCallback
        DecDCTout( dec.imgbuf[dec.imgid], dec.slice.w * dec.slice.h / 2);

        // decode the next frame's VLC data
        strNextVlc( &dec );

        // wait for whole decode process per 1 frame
        strSync( &dec, 0 );

        // wait for V-Vlank
        VSync(0);

        // swap the display buffer
		//notice that the display buffer is the opossite side of decoding buffer
        id = dec.rectid ? 0 : 1;
        SetDefDispEnv( &disp, 0, (id)*240, SCR_WIDTH*PPW, SCR_HEIGHT );
//      SetDefDrawEnv(&draw, 0, (id)*240, SCR_WIDTH*PPW, SCR_HEIGHT);

#if IS_RGB24==1
        disp.isrgb24 = IS_RGB24;
        disp.disp.w = disp.disp.w * 2/3;
#endif
        PutDispEnv( &disp );
//      PutDrawEnv(&draw);
//WBN		FntPrint( "Movies" );
//WBN		FntFlush( -1 );
        SetDispMask( 1 );     // display enable

        if ( Rewind_Switch == 1 )
            break;

        if ( PadRead( 0 ) & PADh )
		{	// play button pressed exit animation routine
            break;
		}
    }

    // post processing of animation routine
    DecDCToutCallback( 0 );
    StUnSetRing();
    CdControlB( CdlPause, 0, 0 );

	return 0;
}


// init DECENV    buffer0(x0,y0),buffer1(x1,y1) :
static void
strSetDefDecEnv( DECENV* dec, int x0, int y0, int x1, int y1 )
{
    dec->vlcbuf[0] = vlcbuf0;
    dec->vlcbuf[1] = vlcbuf1;
    dec->vlcid     = 0;

    dec->imgbuf[0] = imgbuf0;
    dec->imgbuf[1] = imgbuf1;
    dec->imgid     = 0;

    // width and height of rect[] are set dynamicaly according to STR data
    dec->rect[0].x = x0;
    dec->rect[0].y = y0;
    dec->rect[1].x = x1;
    dec->rect[1].y = y1;
    dec->rectid    = 0;

    dec->slice.x = x0;
    dec->slice.y = y0;
    dec->slice.w = 16*PPW;

    dec->isdone    = 0;
}


// init the streaming environment and start the cdrom :
static void
strInit( CdlLOC* loc, void (*callback)() )
{
    // cold reset mdec
    DecDCTReset( 0 );

    // set the callback after 1 block MDEC decoding
    DecDCToutCallback( callback );

    // set the ring buffer
    StSetRing( Ring_Buff, RING_SIZE );

    // init the streaming library
    // end frame is set endless
    StSetStream( IS_RGB24, START_FRAME, 0xffffffff, 0, 0 );

    // start the cdrom
    strKickCD( loc );
}


//  back ground process callback of DecDCTout()
static void
strCallback()
{
  RECT snap_rect;
  int  id;


#if IS_RGB24==1
    extern StCdIntrFlag;
    if ( StCdIntrFlag )
	{
        StCdInterrupt();    // on the RGB24 bit mode , call StCdInterrupt manually at this timing
        StCdIntrFlag = 0;
    }
#endif

  id = dec.imgid;
  snap_rect = dec.slice;

    // switch the id of decoding buffer
	dec.imgid = dec.imgid? 0:1;

    // update slice(rectangle) position
    dec.slice.x += dec.slice.w;

    // remaining slice ?
    if ( dec.slice.x < dec.rect[dec.rectid].x + dec.rect[dec.rectid].w )
	{
        // prepare for next slice
        DecDCTout(dec.imgbuf[dec.imgid], dec.slice.w*dec.slice.h/2);
    }
    // last slice ; end of 1 frame
    else
	{
        // set the decoding done flag
        dec.isdone = 1;

        // update the position on VRAM
        dec.rectid = dec.rectid? 0: 1;
        dec.slice.x = dec.rect[dec.rectid].x;
        dec.slice.y = dec.rect[dec.rectid].y;
    }

  // transfer the decoded data to VRAM :
  LoadImage( &snap_rect, (u_long*)dec.imgbuf[id] );
}


//  execute VLC decoding the decoding data is the next frame's
static void
strNextVlc( DECENV* dec )
{
    int cnt = WAIT_TIME;
    u_long* next;
    static u_long* strNext();

    // get the 1 frame streaming data
    while ( ( next = strNext( dec ) ) == 0 )
	{
        if ( --cnt == 0 )
            return;
    }

    // switch the decoding area
    dec->vlcid = dec->vlcid ? 0 : 1;

    // VLC decode
    DecDCTvlc( next, dec->vlcbuf[dec->vlcid] );

    // free the ring buffer
    StFreeRing( next );

    return;
}


/*
 *  get the 1 frame streaming data
 *  return vale     normal end -> top address of 1 frame streaming data
 *                  error      -> NULL :
 */
static u_long*
strNext( DECENV* dec )
{
    u_long* addr;
    StHEADER* sector;
    int cnt = WAIT_TIME;

	assert( theMovie );

    // get the 1 frame streaming data withe TIME-OUT
    while ( StGetNext( (u_long* *)&addr, (u_long* *)&sector ) )
	{
        if ( --cnt == 0 )
            return 0;
    }

    // if the frame number greater than the end frame, set the end switch
    if ( sector->frameCount >= theMovie->nFrames-3 )
        Rewind_Switch = 1;

    // if the resolution is differ to previous frame, clear frame buffer
    if ( StrWidth != sector->width || StrHeight != sector->height )
	{
        RECT    rect;
        setRECT( &rect, 0, 0, SCR_WIDTH*PPW, SCR_HEIGHT*2 );
        ClearImage( &rect, 0, 0, 0 );

        StrWidth  = sector->width;
        StrHeight = sector->height;
    }

    // set DECENV according to the data on the STR format
    dec->rect[0].w = dec->rect[1].w = StrWidth*PPW;
    dec->rect[0].h = dec->rect[1].h = StrHeight;
    dec->slice.h   = StrHeight;

    return addr;
}


// wait for finish decodeing 1 frame with TIME-OUT
static void
strSync( DECENV* dec, int mode )
{
    volatile u_long cnt = WAIT_TIME;

    // wait for the decod is done flag set by background process
    while ( dec->isdone == 0 )
	{
        if ( --cnt == 0 )
		{
            // if timeout force to switch buffer
            printf("time out in decoding !\n");
            dec->isdone = 1;
            dec->rectid = dec->rectid? 0: 1;
            dec->slice.x = dec->rect[dec->rectid].x;
            dec->slice.y = dec->rect[dec->rectid].y;
        }
    }
    dec->isdone = 0;
}


// start streaming
static void
strKickCD( CdlLOC* loc )
{
	u_char param;

	param = CdlModeSpeed;

loop:
	// seek to the destination
	while ( CdControl( CdlSetloc, (u_char*)loc, 0 ) == 0 )
		;
	while ( CdControl( CdlSetmode, &param, 0 ) == 0 )
		;
	VSync( 3 );  // wait for 3 VSync when changing the speed
	/// out the read command with streaming mode
	if ( CdRead2( CdlModeStream|CdlModeSpeed|CdlModeRT ) == 0 )
		goto loop;
}
