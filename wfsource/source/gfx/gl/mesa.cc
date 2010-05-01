//=============================================================================
// gfx/win/mesa.cc: mesa gl specific portion of interface, included by display.cc
// currently includes X windows interface portions as well
// Copyright ( c ) 1999 World Foundry Group  
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
// Description: The Display class encapsulates data and behavior for a single
//       hardware screen
// Original Author: Kevin T. Seghetti
//============================================================================

#include <time.h>

// kts major kludge, both GL and WF should use namespaces              
#define Display XDisplay
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#undef Display

//==============================================================================

struct HalDisplay
{
    XDisplay *mainDisplay;
    int screenIndex;
    XVisualInfo *visInfo;
    Window win;
};

HalDisplay halDisplay;

//==============================================================================

#include <stdio.h>
#include <stdlib.h>

//static GLint Black, Red, Green, Blue;

#include <X11/keysym.h>
#include <X11/Xlib.h>

//#include <string.h>

static int attributeList[] = { GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
    GLX_BLUE_SIZE, 1, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 1, None};



//==============================================================================
    //Override the focusIn and foucusOut events,

// void
// XXX::focusInEvent(QFocusEvent *)
// {
//   autoRepeat(0);
// }
//
// void
// XXX::focusOutEvent(QFocusEvent *)
// {
//   autoRepeat(1);
// }


void
SetX11AutoRepeat(int state)
{
  RangeCheckInclusive(0,state,1);

  XKeyboardControl values;
  values.auto_repeat_mode = state;
  values.key = -1;         // ALL keys
  XChangeKeyboardControl(halDisplay.mainDisplay, KBAutoRepeatMode, &values);
}


static void
_atExitTermDisplay(int code)
{
	(void)code;					// suppress unused warnings
   SetX11AutoRepeat(1);
   XFlush(halDisplay.mainDisplay);
}

//==============================================================================

void 
OpenMainWindow( char *title )
{
    int x = 0, y = 0;
    XSetWindowAttributes attr;
    Window root; 

    sys_atexit(&_atExitTermDisplay);	// Make sure it gets called on error exit


    halDisplay.mainDisplay = XOpenDisplay(NULL);
    if(!halDisplay.mainDisplay)
        FatalError("Couldn't open default display!");

#if 1
    halDisplay.visInfo = glXChooseVisual(halDisplay.mainDisplay, DefaultScreen(halDisplay.mainDisplay), attributeList);
    if(!halDisplay.visInfo)
        FatalError("no suitable visual");

    GLXContext cx = glXCreateContext(halDisplay.mainDisplay, halDisplay.visInfo, 0, GL_TRUE);

    root = RootWindow(halDisplay.mainDisplay,halDisplay.visInfo->screen);

    attr.border_pixel = 0;
    attr.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | FocusChangeMask;
    attr.colormap = XCreateColormap(halDisplay.mainDisplay, root, halDisplay.visInfo->visual, AllocNone);

    halDisplay.win = XCreateWindow(halDisplay.mainDisplay, root, x, y, wfWindowWidth, wfWindowHeight,
                                   0, halDisplay.visInfo->depth, InputOutput, halDisplay.visInfo->visual,
                                   CWBorderPixel|CWColormap|CWEventMask, &attr);
    if(!halDisplay.win)
        FatalError("Couldn't open X window!");
    XStoreName(halDisplay.mainDisplay, halDisplay.win, title);
    XMapWindow(halDisplay.mainDisplay, halDisplay.win);
    glXMakeCurrent(halDisplay.mainDisplay, halDisplay.win, cx);
    AssertGLOK();
#else
    XEvent e;

    halDisplay.screenIndex = DefaultScreen(halDisplay.mainDisplay);
    root = RootWindow(halDisplay.mainDisplay, halDisplay.screenIndex);

   // alloc halDisplay.visInfo struct 
    halDisplay.visInfo = (XVisualInfo *) malloc( sizeof(XVisualInfo) );

    if(!XMatchVisualInfo( halDisplay.mainDisplay, halDisplay.screenIndex,16, 1, halDisplay.visInfo ))
        FatalError("Couldn't get 16 bit visual!");
//      Black = Red = Green = Blue = 0;

   // set window attributes 
    attr.border_pixel = BlackPixel( halDisplay.mainDisplay, halDisplay.screenIndex );
    attr.event_mask = ExposureMask | StructureNotifyMask;
    attr.colormap = XCreateColormap( halDisplay.mainDisplay, root, halDisplay.visInfo->visual, AllocNone );
    attr.background_pixel = BlackPixel( halDisplay.mainDisplay, halDisplay.screenIndex );

   // Create the window 
    halDisplay.win = XCreateWindow( halDisplay.mainDisplay, root, x,y, wfWindowWidth, wfWindowHeight, 0,
                                    halDisplay.visInfo->depth, InputOutput,
                                    halDisplay.visInfo->visual,
                                    CWBorderPixel|CWColormap|CWEventMask|CWBackPixel, &attr);
    if(!halDisplay.win)
        FatalError("Couldn't open X window!");

    XTextProperty tp;                             
    XStringListToTextProperty(&title, 1, &tp);
    XSizeHints sh;
    sh.flags = USPosition | USSize;
    XSetWMProperties(halDisplay.mainDisplay, halDisplay.win, &tp, &tp, 0, 0, &sh, 0, 0);
    XSelectInput(halDisplay.mainDisplay, halDisplay.win, ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask|SubstructureNotifyMask);
    XMapWindow(halDisplay.mainDisplay, halDisplay.win);
#endif
}

//==============================================================================

bool
InitWindow( int /*xPos*/, int /*yPos*/, int /*xSize*/, int /*ySize*/ )
{
    OpenMainWindow( "World Foundry");
    return true;
}

//==============================================================================

#include <X11/keysym.h>

extern void
_HALSetJoystickButtons(joystickButtonsF joystickButtons);

//==============================================================================
// kts FIX: quick hack to get onto linux, relace with a key table

static joystickButtonsF _joystickButtons = 0;

static 
void ProcessXEvents(XEvent event)
{
    KeySym key;

    switch(event.type)
    {
        case ConfigureNotify: 
            {
                /* this approach preserves a 1:1 viewport aspect ratio */
                int vX, vY, vW, vH;
                int eW = event.xconfigure.width, eH = event.xconfigure.height;
                if(eW >= eH)
                {
                    vX = 0;
                    vY = (eH - eW) >> 1;
                    vW = vH = eW;
                }
                else
                {
                    vX = (eW - eH) >> 1;
                    vY = 0;
                    vW = vH = eH;
                }
                glViewport(vX, vY, vW, vH);
                AssertGLOK();
            }
            break;

        case KeyPress:
            // printf("key %x pressed\n",key);
            key = XLookupKeysym(&event.xkey, 0);
            switch(key)
            {
                case(XK_KP_4):
                case(XK_Left):
                case(XK_KP_Left):
                case(XK_j):
                case(XK_J):
                    _joystickButtons |= EJ_BUTTONF_LEFT;
                    break;
                case(XK_KP_6):
                case(XK_Right):
                case(XK_KP_Right):
                case(XK_L):
                case(XK_l):
                    _joystickButtons |= EJ_BUTTONF_RIGHT;
                    break;
                case(XK_KP_8):
                case(XK_Up):
                case(XK_KP_Up):
                case(XK_i):
                case(XK_I):
                    _joystickButtons |= EJ_BUTTONF_UP;
                    break;
                case(XK_KP_2):
                case(XK_Down):
                case(XK_KP_Down):
                case(XK_k):
                case(XK_K):
                    _joystickButtons |= EJ_BUTTONF_DOWN;
                    break;
                case XK_KP_Insert:
                case XK_1:
                    _joystickButtons |= EJ_BUTTONF_A;
                    break;
                case XK_KP_Delete:
                case XK_2:
                    _joystickButtons |= EJ_BUTTONF_B;
                    break;
                case XK_KP_Enter:
                case XK_3:
                    _joystickButtons |= EJ_BUTTONF_C;
                    break;
                case(XK_KP_Add):
                case XK_4:
                    _joystickButtons |= EJ_BUTTONF_D;
                    break;
                case(XK_KP_Subtract):
                case XK_5:
                    _joystickButtons |= EJ_BUTTONF_E;
                    break;
                case(XK_KP_Multiply):
                case XK_6:
                    _joystickButtons |= EJ_BUTTONF_F;
                    break;
                case(XK_KP_Divide):
                case(XK_7):
                    _joystickButtons |= EJ_BUTTONF_G;
                    break;
                case(XK_KP_7):
                case(XK_8):
                    _joystickButtons |= EJ_BUTTONF_H;
                    break;
                case(XK_KP_9):
                case XK_9:
                    _joystickButtons |= EJ_BUTTONF_I;
                    break;
                case(XK_KP_3):
                case XK_0:
                    _joystickButtons |= EJ_BUTTONF_J;
                    break;
                case(XK_KP_1):
                case XK_exclam:
                    _joystickButtons |= EJ_BUTTONF_K;
                    break;
                case XK_Escape:
                    sys_exit(0);
//                 case XK_Escape:
//                     _joystickButtons |= 0x80000000;
                    break;
                default: 
                    printf("unknown key %x pressed, XK_KP_Enter = %x\n",key,XK_KP_Enter);
                    break;
            }
            break;

        case KeyRelease:
            // printf("key %x released\n",key);
            key = XLookupKeysym(&event.xkey, 0);
            switch(key)
            {
                case(XK_KP_4):
                case(XK_Left):
                case(XK_KP_Left):
                case(XK_J):
                case(XK_j):
                    _joystickButtons &= ~EJ_BUTTONF_LEFT;
                    break;
                case(XK_KP_6):
                case(XK_Right):
                case(XK_KP_Right):
                case(XK_L):
                case(XK_l):
                    _joystickButtons &= ~EJ_BUTTONF_RIGHT;
                    break;
                case(XK_KP_8):
                case(XK_Up):
                case(XK_KP_Up):
                case(XK_i):
                case(XK_I):
                    _joystickButtons &= ~EJ_BUTTONF_UP;
                    break;
                case(XK_KP_2):
                case(XK_Down):
                case(XK_KP_Down):
                case(XK_K):
                case(XK_k):
                    _joystickButtons &= ~EJ_BUTTONF_DOWN;
                    break;
                case XK_KP_Insert:
                case XK_1:
                    _joystickButtons &= ~EJ_BUTTONF_A;
                    break;
                case XK_KP_Delete:
                case XK_2:
                    _joystickButtons &= ~EJ_BUTTONF_B;
                    break;
                case XK_KP_Enter:
                case XK_3:
                    _joystickButtons &= ~EJ_BUTTONF_C;
                    break;
                case(XK_KP_Add):
                case XK_4:
                    _joystickButtons &= ~EJ_BUTTONF_D;
                    break;
                case(XK_KP_Subtract):
                case XK_5:
                    _joystickButtons &= ~EJ_BUTTONF_E;
                    break;
                case(XK_KP_Multiply):
                case XK_6:
                    _joystickButtons &= ~EJ_BUTTONF_F;
                    break;
                case(XK_KP_Divide):
                case XK_7:
                    _joystickButtons &= ~EJ_BUTTONF_G;
                    break;
                case(XK_KP_7):
                case XK_8:
                    _joystickButtons &= ~EJ_BUTTONF_H;
                    break;
                case(XK_KP_9):
                case XK_9:
                    _joystickButtons &= ~EJ_BUTTONF_I;
                    break;
                case(XK_KP_3):
                case XK_0:
                    _joystickButtons &= ~EJ_BUTTONF_J;
                    break;
                case(XK_KP_1):
                case XK_exclam:
                    _joystickButtons &= ~EJ_BUTTONF_K;
                    break;
//                case XK_Escape:
//                    _joystickButtons &= ~0x8000000;
                    break;
                default: 
                    printf("unknown key %x released\n",key);
                    break;
            }
            break;


        case ButtonPressMask:
            printf("You pressed button %d\n", event.xbutton.button);
            break;

       case FocusIn :
          SetX11AutoRepeat(0);
          break;
       case FocusOut :
          SetX11AutoRepeat(1);
          break;

          // kts this doesn't seem to work (at least it doesn't send me a message when the window is being destroyed
       case DestroyNotify:
          printf("destroynotify!!\n");
          SetX11AutoRepeat(1);
          break;

        default:
            break;
    }

     //printf("pxevent: _joystickButtons = %x\n", _joystickButtons);
    _HALSetJoystickButtons(_joystickButtons);
}

//==============================================================================

void XEventLoop()
{
    XEvent xev;
    int num_events;

    XFlush(halDisplay.mainDisplay);
    num_events = XPending(halDisplay.mainDisplay);
    while((num_events != 0))
    {
        num_events--;
        XNextEvent(halDisplay.mainDisplay, &xev);
        ProcessXEvents(xev);
    }

    // printf("_joystickButtons = %x\n", _joystickButtons);
}

//==============================================================================
