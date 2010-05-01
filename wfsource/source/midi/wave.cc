//=============================================================================
// wave.cc
// Copyright ( c ) 1997,99 World Foundry Group  
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

#if defined( __WIN__ )
#include <windows.h>

// Plays a given WAVE file by using MCI_OPEN, MCI_PLAY.
// Returns when playback
// begins. Returns 0L on success;
// otherwise returns an MCI error code.
DWORD playWAVEFile(HWND hWndNotify, LPSTR lpszWAVEFileName)
{
    UINT wDeviceID;
    DWORD dwReturn;
    MCI_OPEN_PARMS mciOpenParms;
    MCI_PLAY_PARMS mciPlayParms;

    // Open the device by specifying the
    // device name and device element.
    // MCI will choose a device capable
    // of playing the given file.
    mciOpenParms.lpstrDeviceType = "waveaudio";
    mciOpenParms.lpstrElementName = lpszWAVEFileName;

    if (dwReturn = mciSendCommand(0, MCI_OPEN,
            MCI_OPEN_TYPE | MCI_OPEN_ELEMENT,
            (DWORD)(LPVOID) &mciOpenParms)) {
        // Failed to open device;
        // don't close it, just return error.
        return dwReturn;
    }

    // Device opened successfully. Get the device ID.
    wDeviceID = mciOpenParms.wDeviceID;

    // Begin playback. The window-procedure function
    // for the parent window is notified with an
    // MM_MCINOTIFY message when playback is
    // complete. The window procedure then
    // closes the device.

    mciPlayParms.dwCallback = (DWORD) hWndNotify;
    if (dwReturn = mciSendCommand(wDeviceID, MCI_PLAY,
            MCI_NOTIFY, (DWORD)(LPVOID) &mciPlayParms)) {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        return dwReturn;
    }

    return 0;
}

//=============================================================================
#endif // defined( __WIN__ )
//=============================================================================
