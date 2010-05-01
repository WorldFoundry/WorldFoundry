//=============================================================================
// midi.cc
// Copyright ( c ) 1997,98,99 World Foundry Group  
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

// Plays a given MIDI file using MCI_OPEN, MCI_PLAY.
// Returns as soon as playback begins. The window
// procedure function for the given window is notified
// when playback is complete. Returns 0L on success;
// otherwise returns an MCI error code.

DWORD playMIDIFile(HWND hWndNotify, LPSTR lpszMIDIFileName)
{
    UINT wDeviceID;
    DWORD dwReturn;
    MCI_OPEN_PARMS mciOpenParms;
    MCI_PLAY_PARMS mciPlayParms;
    MCI_STATUS_PARMS mciStatusParms;

    // Open the device by specifying the
    // device name and device element.
    // MCI will attempt to choose the
    // MIDI Mapper as the output port.

    mciOpenParms.lpstrDeviceType = "sequencer";
    mciOpenParms.lpstrElementName = lpszMIDIFileName;
    if (dwReturn = mciSendCommand(NULL, MCI_OPEN,
            MCI_OPEN_TYPE | MCI_OPEN_ELEMENT,
            (DWORD)(LPVOID) &mciOpenParms)) {
        // Failed to open device;
        // don't close it, just return error.
        return dwReturn;
    }

    // Device opened successfully. Get the device ID.
    wDeviceID = mciOpenParms.wDeviceID;

    // See if the output port is the MIDI Mapper.
    mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT;

    if (dwReturn = mciSendCommand(wDeviceID, MCI_STATUS,
            MCI_STATUS_ITEM,
            (DWORD)(LPVOID) &mciStatusParms)) {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        return dwReturn;
    }

    // The output port is not the MIDI Mapper.
    // Ask if user wants to continue.
    if (LOWORD(mciStatusParms.dwReturn) != MIDI_MAPPER) {
        if (MessageBox( NULL,
                "The MIDI Mapper is not available. Continue?",
                "", MB_YESNO) == IDNO)
		{
            // User does not want to continue. Not an error;
            // just close the device and return.
            mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
            return 0;
        }
    }

    // Begin playback. The window procedure function
    // for the parent window is notified with an
    // MM_MCINOTIFY message when playback is complete.
    // The window procedure then closes the device.
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
