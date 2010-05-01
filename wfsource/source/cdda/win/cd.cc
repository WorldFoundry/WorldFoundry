//=============================================================================
// cd.cc:
// Copyright ( c ) 1997,1998,1999,2000 World Foundry Group  
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

//=============================================================================

#define STRICT
#include <windows.h>
#include <mmsystem.h>

// * Plays a given compact disc track using
// * MCI_OPEN, MCI_PLAY. Returns as
// * soon as playback begins. The window procedure
// * function for the given window will be notified
// * when playback is complete. Returns 0L on
// * success; otherwise, it returns an MCI error code.
DWORD
playCDTrack(HWND hWndNotify, BYTE bTrack)
{
    static UINT wDeviceID;
    DWORD dwReturn;
    MCI_OPEN_PARMS mciOpenParms;
    MCI_SET_PARMS mciSetParms;
    MCI_PLAY_PARMS mciPlayParms;

    // Open the compact disc device by specifying
    // the device name.
    mciOpenParms.lpstrDeviceType = "cdaudio";

    dwReturn = mciSendCommand( NULL, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)(LPVOID)&mciOpenParms );

	if ( dwReturn != 0 && dwReturn != MCIERR_DEVICE_OPEN )
	{	// Failed to open device; don't close it, just return error.
        return dwReturn;
    }

	if ( dwReturn != MCIERR_DEVICE_OPEN )
	{	// Device opened successfully, get the device ID.
    	wDeviceID = mciOpenParms.wDeviceID;
	}

    // Set the time format to track/minute/second/frame.
    mciSetParms.dwTimeFormat = MCI_FORMAT_TMSF;
    if (dwReturn = mciSendCommand(wDeviceID, MCI_SET,
            MCI_SET_TIME_FORMAT, (DWORD)(LPVOID) &mciSetParms))
	{

        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        return dwReturn;
    }

	// Begin playback from the given track and play
	// until the beginning of the next track. The window
	// procedure function for the parent window will be
	// notified with an MM_MCINOTIFY message when playback
	// is complete. Unless the play command fails, the
	// window procedure closes the device.
    mciPlayParms.dwFrom = MCI_MAKE_TMSF(bTrack, 0, 0, 0);
    mciPlayParms.dwTo = MCI_MAKE_TMSF(bTrack + 1, 0, 0, 0);
    mciPlayParms.dwCallback = (DWORD) hWndNotify;
    if (dwReturn = mciSendCommand(wDeviceID, MCI_PLAY,
            MCI_FROM | MCI_TO | MCI_NOTIFY,

            (DWORD)(LPVOID) &mciPlayParms)) {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        return dwReturn;
    }

    return 0;
}
