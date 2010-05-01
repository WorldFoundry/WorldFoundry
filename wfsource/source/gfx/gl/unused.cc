//=============================================================================

#if 0
void
SaveVRAM()
{
#pragma pack( 1 )
	typedef struct
        {
        char IDLength;
        char ColorMapType;
        char ImageType;
		int16 CMapStart;
		int16 CMapLength;
        char CMapDepth;
        int16 XOffset;
        int16 YOffset;
        int16 Width;
        int16 Height;
        char PixelDepth;
        char ImageDescriptor;
        } TGA_HEADER;
#pragma pack()

	static TGA_HEADER	th =
		{
        0,	// char IDLength;
        0,	// char ColorMapType;
        2,	// char ImageType;		only write out 16-bit images
		0,	// int16 CMapStart;
		0,	// int16 CMapLength;
        0,	// char CMapDepth;
        0,	// int16 XOffset;
        0,	// int16 YOffset;
        0,	// int16 Width;
        0,	// int16 Height;
        0,	// char PixelDepth;
        0	// char ImageDescriptor;
		};

	th.Width = VRAM_WIDTH;
	th.Height = VRAM_HEIGHT;
	th.PixelDepth = 24;

	char szOutputBitmap[ _MAX_PATH ] = "vram.tga";
	FILE* fp = fopen( szOutputBitmap, "wb" );
	assert( fp );

	fwrite( &th, sizeof( th ), 1, fp );

	RGB_pixel* pixels = &vram[ 0 ][ 0 ];

	for ( int y = th.Height-1; y >= 0; --y )
	{
		RGB_pixel* pixeldata = pixels + (y * VRAM_WIDTH);
		int cbWritten = fwrite( pixeldata, sizeof( RGB_pixel ), th.Width, fp );
		assert( cbWritten = th.Width );
	}

	fclose( fp );
}
#endif

//==============================================================================

#pragma message ("KTS " __FILE__ ": move this into the win directory")
//=============================================================================
// * 'ConvertRGB()' - Convert a DIB/BMP image to 24-bit RGB pixels.
// *
// * Returns an RGB pixel array if successful and NULL otherwise.

GLubyte*
ConvertRGB(BITMAPINFO *info,		// I - Original bitmap information
           void       *bits)		// I - Original bitmap pixels
{
  int		i, j,			// Looping vars
  			bitsize,		// Total size of bitmap
			width;			// Aligned width of bitmap
  GLubyte	*newbits;		// New RGB bits
  GLubyte	*from, *to,		// RGB looping vars
			temp;			// Temporary var for swapping


//  Allocate memory for the RGB bitmap...

  width   = 3 * info->bmiHeader.biWidth;
  width   = (width + 3) & ~3;
  bitsize = width * info->bmiHeader.biHeight;
  if ((newbits = (GLubyte *)calloc(bitsize, 1)) == NULL)
    return (NULL);

//  Copy the original bitmap to the new array, converting as necessary.

  switch (info->bmiHeader.biCompression)
  {
    case BI_RGB :
        if (info->bmiHeader.biBitCount == 24)
	{
//          Swap red & blue in a 24-bit image...

          for (i = 0; i < info->bmiHeader.biHeight; i ++)
	    for (j = 0, from = ((GLubyte *)bits) + i * width,
	             to = newbits + i * width;
		 j < info->bmiHeader.biWidth;
		 j ++, from += 3, to += 3)
            {
              to[0] = from[2];
              to[1] = from[1];
              to[2] = from[0];
            };
	};
	break;
    case BI_RLE4 :
    case BI_RLE8 :
    case BI_BITFIELDS :
        break;
  };

  return (newbits);
}

//=============================================================================

#if 0
void*
LoadDIBitmap(char	*filename,	// I - File to load
             BITMAPINFO	**info)		// O - Bitmap information
{
  FILE				*fp;		// Open file pointer
  void				*bits;		// Bitmap pixel bits
  long				bitsize,	// Size of bitmap
					infosize;	// Size of header information
  BITMAPFILEHEADER	header;		// File header


  // Try opening the file; use "rb" mode to read this *binary* file.

  if ((fp = fopen(filename, "rb")) == NULL)
    return (NULL);

  // Read the file header and any following bitmap information...

  if (fread(&header, sizeof(BITMAPFILEHEADER), 1, fp) < 1)
  {
  	// Couldn't read the file header - return NULL...

    fclose(fp);
    return (NULL);
  };

  if (header.bfType != 'MB')	// Check for BM reversed...
  {
    // Not a bitmap file - return NULL...

    fclose(fp);
    return (NULL);
  };

  infosize = header.bfOffBits - sizeof(BITMAPFILEHEADER);
  if ((*info = (BITMAPINFO *)malloc(infosize)) == NULL)
  {
    // Couldn't allocate memory for bitmap info - return NULL...

    fclose(fp);
    return (NULL);
  };

  if (fread(*info, 1, infosize, fp) < infosize)
  {
    // Couldn't read the bitmap header - return NULL...

    free(*info);
    fclose(fp);
    return (NULL);
  };

  // Now that we have all the header info read in, allocate memory for the
  // bitmap and read *it* in...

  if ((bitsize = (*info)->bmiHeader.biSizeImage) == 0)
    bitsize = ((*info)->bmiHeader.biWidth *
               (*info)->bmiHeader.biBitCount + 7) / 8 *
  	      abs((*info)->bmiHeader.biHeight);

  if ((bits = malloc(bitsize)) == NULL)
  {
    // Couldn't allocate memory - return NULL!

    free(*info);
    fclose(fp);
    return (NULL);
  };

  if (fread(bits, 1, bitsize, fp) < bitsize)
  {
    // Couldn't read bitmap - free memory and return NULL!

    free(*info);
    free(bits);
    fclose(fp);
    return (NULL);
  };

  // OK, everything went fine - return the allocated bitmap...

  fclose(fp);
  return (bits);
}
#endif		// 0

//=============================================================================

#if 0 && defined(RENDERER_GL)
int
TextureLoadBitmap(char *filename)		// I - Bitmap file to load
{
	void		*bits;					// Bitmap pixel bits
	GLubyte	*rgb;						// Bitmap RGB pixels

	// Try loading the bitmap and converting it to RGB...
#if defined(__WIN__)
	BITMAPINFO	*info;					// Bitmap information
	bits = LoadDIBitmap(filename, &info);
	if (bits == NULL)
		return (-1);

	rgb = ConvertRGB(info, bits);
	if (rgb == NULL)
	{
		free(info);
		free(bits);

		return (-1);
	};
#elif defined(__LINUX__)
	printf("kts write bitmap loader!\n");
#else
#error os not defined
#endif

#if defined(RENDERER_GL)
	// Define the texture image.  If the width or height is 1, then we define a 1D texture.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	// Force 4-byte alignment
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);

	if (info->bmiHeader.biHeight == 1)
	{
		glTexImage1D(GL_TEXTURE_1D, 0, 3, info->bmiHeader.biWidth, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if (info->bmiHeader.biWidth == 1)
	{
		glTexImage1D(GL_TEXTURE_1D, 0, 3, info->bmiHeader.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else
	{
		assert(0);
//		glTexImage2D(GL_TEXTURE_2D, 0, 3, info->bmiHeader.biWidth, info->bmiHeader.biHeight, 0,
//                		GL_RGB, GL_UNSIGNED_BYTE, rgb);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	};

	// Free the bitmap and RGB images, then return 0 (no errors).
	free(rgb);
	free(info);
	free(bits);
#endif
	return (0);
}
#endif

//==============================================================================

#if 0
void
TakeOver()
{
	WNDCLASS	wc;			// Windows class structure
    DDSURFACEDESC       ddsd;
    DDSCAPS             ddscaps;
    HRESULT             ddrval;
    char                buf[256];

    // set up and register window class
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = NULL;
    wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = NULL;
    wc.lpszMenuName = lpszAppName;
    wc.lpszClassName = lpszAppName;
	// Register the window class
	if(RegisterClass(&wc) == 0)
	{
		assert(0);
		sys_exit(1);
	}

    hWnd = CreateWindowEx(			// create a window
//#if !RUN_IN_WINDOW
	WS_EX_TOPMOST|
//#endif
	0,
	lpszAppName,
	lpszAppName,
//#if !RUN_IN_WINDOW
	WS_POPUP|
//#endif
	WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
#if 0
	// Window position and size
	100, 100,
	250, 250,
#else
	0, 0,
	GetSystemMetrics( SM_CXSCREEN ),
	GetSystemMetrics( SM_CYSCREEN ),
#endif
	NULL,
	NULL,
//	hInstance,
	NULL,
	NULL );

	// If window was not created, quit
	if(hWnd == NULL)
	{
		assert(0);
		sys_exit(1);
	}

	hardwaredevicecontext = GetDC(hWnd);
	assert(hardwaredevicecontext);
	// Select the pixel format
	SetDCPixelFormat(hardwaredevicecontext);

	// Create the rendering context and make it current
	assert(hardwaredevicecontext);
	hRC = wglCreateContext(hardwaredevicecontext);
	assert(hRC);
	if(!wglMakeCurrent(hardwaredevicecontext, hRC))
		assert(0);

	// Display the window
	ShowWindow(hWnd,SW_SHOW);
	UpdateWindow(hWnd);

#if 0
    // create the main DirectDraw object
    ddrval = DirectDrawCreate( NULL, &lpDD, NULL );
    if( ddrval == DD_OK )
    {
		// Get exclusive mode
		ddrval = lpDD->SetCooperativeLevel( hWnd,
					DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT );
		if(ddrval == DD_OK )
		{
#if 1
	    	ddrval = lpDD->SetDisplayMode( 640, 480, 16 );
	    	if( ddrval == DD_OK )
	    	{
				// Create the primary surface with 1 back buffer
				ddsd.dwSize = sizeof( ddsd );
				ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
				ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
				      		DDSCAPS_FLIP |
				      		DDSCAPS_COMPLEX;
				ddsd.dwBackBufferCount = 1;
				ddrval = lpDD->CreateSurface( &ddsd, &lpDDSPrimary, NULL );
				if( ddrval == DD_OK )
				{
		    		// Get a pointer to the back buffer
		    		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		    		ddrval = lpDDSPrimary->GetAttachedSurface(&ddscaps,
							  		&lpDDSBack);
		    		if( ddrval == DD_OK )
		    		{
						// draw some text.
						HDC hdc;
						if (lpDDSPrimary->GetDC(&hdc) == DD_OK)
						{
			    			SetBkColor( hdc, RGB( 0, 0, 255 ) );
			    			SetTextColor( hdc, RGB( 255, 255, 0 ) );
			    			TextOut( hdc, 0, 0, szFrontMsg, lstrlen(szFrontMsg) );

							// Select the pixel format
							SetDCPixelFormat(hdc);

							assert(hdc);
							hRC = wglCreateContext(hdc);
							assert(hRC);
							if(!wglMakeCurrent(hdc, hRC))
								assert(0);

			    			lpDDSPrimary->ReleaseDC(hdc);
						}

						if (lpDDSBack->GetDC(&hdc) == DD_OK)
						{
			    			SetBkColor( hdc, RGB( 0, 0, 255 ) );
			    			SetTextColor( hdc, RGB( 255, 255, 0 ) );
			    			TextOut( hdc, 0, 0, szBackMsg, lstrlen(szBackMsg) );
			    			lpDDSBack->ReleaseDC(hdc);
						}
		    		}
				}
	    	}
#endif
		}
    }

#endif

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for(int index=0;index<50;index++)
	{

		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);		// Set background clearing color to blue
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear the window with current clearing color
		glDisable(GL_LIGHTING);

 		glBegin( GL_TRIANGLES );

		glColor3ub( 128, 128, 0 );
		glVertex2i( 0+index, 200-0 );
		glColor3ub( 0,0,128);
		glVertex2i( 320-index, 200-index );
		glColor3ub( 128,128,128);
		glVertex2i( 320, 200-200 );
		glEnd();

		glFlush();

		// Call function to swap the buffers
		SwapBuffers(hardwaredevicecontext);

		// Validate the newly painted client area
		ValidateRect(hWnd,NULL);
	}

	printf("its over\n");
	sys_exit(1);
}
#endif

//==============================================================================
