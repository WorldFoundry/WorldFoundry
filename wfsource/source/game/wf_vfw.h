#ifndef VFW_H
#define VFW_H

#if defined( __WIN__ ) || defined( _WINDOWS )
#	include <windows.h>
#	include <mmsystem.h>
#else
typedef unsigned char BYTE;
typedef short WORD;
typedef long DWORD;
typedef long LONG;
typedef DWORD FOURCC;

#if !( defined( _BRTGA_H ) || defined( _BRBMP_H ) )
typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;
//typedef RGBQUAD FAR* LPRGBQUAD;

typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;
//, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

/* constants for the biCompression field */
#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;	//, FAR *LPBITMAPINFO, *PBITMAPINFO;
#endif

#endif	/* !defined( __WIN__ ) */


typedef struct {
	DWORD topleft;
	DWORD bottomright;
	} vfwRECT;

// The file begins with the main header. In the AVI file, this header is
// identified by the 'avih' four-character code. The header contains general
// information about the file, such as the number of streams within the file
// and the width and height of the AVI sequence. The main header has the
// following data structure defined for it:

typedef struct {
    DWORD  dwMicroSecPerFrame;
    DWORD  dwMaxBytesPerSec;
    DWORD  dwReserved1;
    DWORD  dwFlags;
    DWORD  dwTotalFrames;
    DWORD  dwInitialFrames;
    DWORD  dwStreams;
    DWORD  dwSuggestedBufferSize;
    DWORD  dwWidth;
    DWORD  dwHeight;
    DWORD  dwReserved[4];
} MainAVIHeader;

enum
	{
	AVIF_HASINDEX = 	0x00000010,
	AVIF_MUSTUSEINDEX = 0x00000020,
	AVIF_ISINTERLEAVED = 0x00000100,
	AVIF_COPYRIGHTED = 	0x00010000,
	AVIF_WASCAPTUREFILE = 0x00020000
	};

// The dwMicroSecPerFrame member specifies the period between video frames.
//   This value indicates the overall timing for the file.
// The dwMaxBytesPerSec member specifies the approximate maximum data rate of
//   the file. This value indicates the number of bytes per second the system
//   must handle to present an AVI sequence as specified by the other
//   parameters contained in the main header and stream header chunks.
// The dwFlags member contains any flags for the file. The AVIF_HASINDEX,
//   AVIF_MUSTUSEINDEX, AVIF_ISINTERLEAVED, AVIF_COPYRIGHTED, and
//   AVIF_WASCAPTUREFILE flags are defined.
// The AVIF_HASINDEX and AVIF_MUSTUSEINDEX flags apply to files with an index
//   chunk. The AVIF_HASINDEX flag indicates an index is present. The
//   AVIF_MUSTUSEINDEX flag indicates the index should be used to determine
//   the order of the presentation of the data. When this flag is set, it
//   implies the physical ordering of the chunks in the file does not
//   correspond to the presentation order.
//
// The AVIF_ISINTERLEAVED flag indicates the AVI file has been interleaved.
//   The system can stream interleaved data from a CD-ROM more efficiently
//   than non-interleaved data. For more information about interleaved files,
//   see <arif_special>.
// The AVIF_WASCAPTUREFILE flag indicates the AVI file is a specially
//   allocated file used for capturing real-time video. Typically, capture
//   files have been defragmented by the user so video capture data can be
//   efficiently streamed into the file. If this flag is set, an application
//   should warn the user before writing over the file.
// The AVIF_COPYRIGHTED flag indicates the AVI file contains copyrighted data.
//   When this flag is set, applications should not let users duplicate the
//   file or the data in the file.
// The dwTotalFrames member of the main header specifies the total number of
//   frames of data in file.
//
// The dwInitialFrames member is used for interleaved files. If you are
//   creating interleaved files, specify the number of frames in the file
//   prior to the initial frame of the AVI sequence in this member. For more
//   information about the contents of this member, see <arif_special>.
// The dwStreams member specifies the number of streams in the file. For
//   example, a file with audio and video has two streams.
// The dwSuggestedBufferSize member specifies the suggested buffer size for
//   reading the file. Generally, this size should be large enough to contain
//   the largest chunk in the file. If set to zero, or if it is too small, the
//   playback software will have to reallocate memory during playback, which
//   will reduce performance. For an interleaved file, the buffer size should
//   be large enough to read an entire record, and not just a chunk.
// The dwWidth and dwHeight members specify the width and height of the AVI
//   file in pixels.



// The main header is followed by one or more 'strl' chunks. (A 'strl' chunk
//   is required for each data stream.) These chunks contain information about
//   the streams in the file. Each 'strl' chunk must contain a stream header
//   and stream format chunk. Stream header chunks are identified by the
//   four-character code 'strh' and stream format chunks are identified by the
//   four-character code 'strf'. In addition to the stream header and stream
//   format chunks, the 'strl' chunk might also contain a stream-header data
//   chunk and a stream name chunk. Stream-header data chunks are identified
//   by the four-character code 'strd'. Stream name chunks are identified by
//   the four-character code 'strn'.

typedef struct {
    FOURCC		fccType;
    FOURCC		fccHandler;
    DWORD		dwFlags;	/* Contains AVITF_* flags */
    WORD		wPriority;
    WORD		wLanguage;
    DWORD		dwInitialFrames;
    DWORD		dwScale;
    DWORD		dwRate;	/* dwRate / dwScale == samples/second */
    DWORD		dwStart;
    DWORD		dwLength; /* In units above... */
    DWORD		dwSuggestedBufferSize;
    DWORD		dwQuality;
    DWORD		dwSampleSize;
    vfwRECT		rcFrame;
} AVIStreamHeader;

enum { AVISF_VIDEO_PALCHANGES = 0x00010000 };

// The stream header specifies the type of data the stream contains, such as audio or video, by means of a four-character code. The fccType member is set to 'vids' if the stream it specifies contains video data. It is set to 'auds' if it contains audio data. It is set to 'txts' if it contains text data.
// The fccHandler member contains a four-character code describing the stream handler for the data. For audio and video streams, this specifies the installable compressor or decompressor.
// The dwFlags member contains any flags for the data stream. The AVISF_DISABLED flag indicates that the stream data should be rendered only when explicitly enabled by the user. The AVISF_VIDEO_PALCHANGES flag indicates palette changes are embedded in the file.
// The dwInitialFrames member is used for interleaved files. If you are creating interleaved files, specify the number of frames in the file prior to the initial frame of the AVI sequence in this member. For more information about the contents of this member, see <arif_special>.
// The remaining members describe the playback characteristics of the stream. These factors include the playback rate (dwScale and dwRate), the starting time of the sequence (dwStart), the length of the sequence (dwLength), the size of the playback buffer (dwSuggestedBuffer), an indicator of the data quality (
// dwQuality), and the sample size (dwSampleSize).
//
// Some of the members in the stream header structure are also present in the main header structure. The data in the main header structure applies to the whole file, while the data in the stream header structure applies only to a stream.
// A stream format ('strf') chunk must follow a stream header ('strh') chunk. The stream format chunk describes the format of the data in the stream. For video streams, the information in this chunk is a BITMAPINFO structure (including palette information if appropriate). For audio streams, the information in this chunk is a WAVEFORMATEX or PCMWAVEFORMAT structure. (The WAVEFORMATEX structure is an extended version of the WAVEFORMAT structure.) For more information about this structure and other stream types, see the New Multimedia Data Types and Data Techniques Standards Update.
// The 'strl' chunk might also contain an additional stream-header data ('strd') chunk. If used, this chunk follows the stream format chunk. The format and content of this chunk is defined by installable compression or decompression drivers. Typically, drivers use this information for configuration. Applications that read and write RIFF files do not need to decode this information. They transfer this data to and from a driver as a memory block.
//
// The optional 'strn' stream name chunk provides a zero-terminated text string describing the stream. (The AVIFile functions can use this chunk to let applications identify the steams they want to access by their names.)
// An AVI player associates the stream headers in the LIST 'hdrl' chunk with the stream data in the LIST 'movi' chunk by using the order of the 'strl' chunks. The first 'strl' chunk applies to stream 0, the second applies to stream 1, and so forth. For example, if the first 'strl' chunk describes the wave audio data, the wave audio data is contained in stream 0. Similarly, if the second 'strl' chunk describes video data, then the video data is contained in stream 1.





#endif	/* VFW_H */
