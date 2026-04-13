//! General-purpose IFF chunk reader and writer.
//!
//! World Foundry IFF on-disk layout (same as `iffcomp` writes and `iffdump` reads):
//!
//! ```text
//! [4 bytes] Chunk ID   — big-endian u32 (FOURCC, e.g. `PLAY`)
//! [4 bytes] Payload size — little-endian u32
//! [N bytes] Payload
//! [0–3 bytes] Alignment padding to 4-byte boundary (not counted in size)
//! ```
//!
//! # Reading
//!
//! ```rust,no_run
//! use wf_iff::{read_chunk, id_to_str};
//!
//! let data = std::fs::read("enemy.iff").unwrap();
//! let chunk = read_chunk(&data).unwrap();
//! println!("FOURCC: {}", id_to_str(chunk.id));
//! println!("payload: {} bytes", chunk.payload.len());
//! ```
//!
//! # Writing
//!
//! ```rust
//! use wf_iff::{IffBuilder, str_to_id, write_chunk};
//!
//! let mut b = IffBuilder::new(str_to_id("PLAY"));
//! b.write_le(42_i32, 4);   // int field
//! b.write_bytes(b"hello"); // string field
//! let bytes: Vec<u8> = b.finish();
//!
//! // Or directly from a complete payload:
//! let chunk = write_chunk(str_to_id("TEST"), &[0x01, 0x00, 0x00, 0x00]);
//! ```

// ── error type ────────────────────────────────────────────────────────────────

/// Error returned by IFF parsing functions.
#[derive(Debug, Clone, PartialEq)]
pub struct IffError {
    pub message: String,
}

impl IffError {
    fn new(msg: impl Into<String>) -> Self {
        IffError { message: msg.into() }
    }
}

impl std::fmt::Display for IffError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(&self.message)
    }
}

impl std::error::Error for IffError {}

// ── chunk ─────────────────────────────────────────────────────────────────────

/// A parsed IFF chunk: a FOURCC identifier and its raw payload bytes.
///
/// For wrapper (container) chunks, the payload contains nested sub-chunks;
/// call [`read_chunks`] on `chunk.payload` to walk them.
#[derive(Debug, Clone, PartialEq)]
pub struct IffChunk {
    /// Big-endian FOURCC chunk identifier (e.g. `str_to_id("PLAY")`).
    pub id: u32,
    /// Raw payload bytes, excluding the 8-byte header and alignment padding.
    pub payload: Vec<u8>,
}

// ── FOURCC helpers ────────────────────────────────────────────────────────────

/// Convert a string (up to 4 ASCII chars) to a big-endian FOURCC u32.
/// Strings shorter than 4 chars are space-padded on the right.
///
/// ```rust
/// use wf_iff::str_to_id;
/// assert_eq!(str_to_id("TEST"), 0x54455354);
/// assert_eq!(str_to_id("RM0"), 0x524D3020); // "RM0 "
/// ```
pub fn str_to_id(s: &str) -> u32 {
    let mut arr = [b' '; 4];
    for (i, b) in s.bytes().take(4).enumerate() {
        arr[i] = b;
    }
    u32::from_be_bytes(arr)
}

/// Convert a big-endian FOURCC u32 to a printable string.
///
/// Trailing non-printable/space bytes are stripped; interior non-printable
/// bytes become `.`.  Matches the display behaviour of `iffdump`.
///
/// ```rust
/// use wf_iff::{str_to_id, id_to_str};
/// assert_eq!(id_to_str(str_to_id("PLAY")), "PLAY");
/// assert_eq!(id_to_str(str_to_id("RM0")), "RM0"); // trailing space stripped
/// ```
pub fn id_to_str(id: u32) -> String {
    let bytes = id.to_be_bytes();
    let len = bytes
        .iter()
        .rposition(|&b| b.is_ascii_graphic())
        .map(|i| i + 1)
        .unwrap_or(0);
    bytes[..len]
        .iter()
        .map(|&b| if b.is_ascii_graphic() || b == b' ' { b as char } else { '.' })
        .collect()
}

// ── reading ───────────────────────────────────────────────────────────────────

/// Parse all top-level IFF chunks from `data`.
///
/// Returns one [`IffChunk`] per chunk found.  Nested (wrapper) chunks are
/// returned with their raw payload bytes — call [`read_chunks`] recursively
/// on `chunk.payload` to descend into them.
///
/// Returns an error if any chunk header is truncated or claims more bytes than
/// are available.
pub fn read_chunks(data: &[u8]) -> Result<Vec<IffChunk>, IffError> {
    let mut chunks = Vec::new();
    let mut pos = 0;

    while pos < data.len() {
        if pos + 8 > data.len() {
            return Err(IffError::new(format!(
                "truncated chunk header at offset {pos:#x} (buffer length {:#x})",
                data.len()
            )));
        }

        let id   = u32::from_be_bytes(data[pos..pos + 4].try_into().unwrap());
        let size = u32::from_le_bytes(data[pos + 4..pos + 8].try_into().unwrap()) as usize;

        let payload_start = pos + 8;
        let payload_end   = payload_start + size;
        let aligned_end   = (payload_end + 3) & !3;

        if payload_end > data.len() {
            return Err(IffError::new(format!(
                "chunk '{}' at {pos:#x}: size {size} extends past end of buffer ({} bytes available)",
                id_to_str(id),
                data.len().saturating_sub(payload_start),
            )));
        }

        chunks.push(IffChunk {
            id,
            payload: data[payload_start..payload_end].to_vec(),
        });

        pos = aligned_end;
    }

    Ok(chunks)
}

/// Parse exactly one IFF chunk from the beginning of `data`.
///
/// Returns [`IffError`] if `data` is empty, contains a truncated header, or
/// the chunk claims more bytes than are available.
pub fn read_chunk(data: &[u8]) -> Result<IffChunk, IffError> {
    let chunks = read_chunks(data)?;
    chunks.into_iter().next().ok_or_else(|| IffError::new("no IFF chunks found"))
}

// ── writing ───────────────────────────────────────────────────────────────────

/// Serialize a FOURCC + payload as a complete IFF chunk (8-byte header +
/// payload + alignment padding to a 4-byte boundary).
///
/// The returned bytes are suitable for writing directly to a `.iff` file or
/// for embedding as the payload of a wrapper chunk.
pub fn write_chunk(id: u32, payload: &[u8]) -> Vec<u8> {
    let size        = payload.len();
    let aligned_len = (size + 3) & !3;

    let mut out = Vec::with_capacity(8 + aligned_len);
    out.extend_from_slice(&id.to_be_bytes());
    out.extend_from_slice(&(size as u32).to_le_bytes());
    out.extend_from_slice(payload);
    // Zero-pad to 4-byte alignment (not counted in the size field).
    out.resize(8 + aligned_len, 0u8);
    out
}

/// Builder for incrementally constructing an IFF chunk's payload before
/// wrapping it with a header via [`IffBuilder::finish`].
///
/// ```rust
/// use wf_iff::{IffBuilder, str_to_id};
///
/// let mut b = IffBuilder::new(str_to_id("PLAY"));
/// b.write_le(100_i32, 4);       // 4-byte LE integer
/// b.write_le(-1_i32,  2);       // 2-byte LE integer (low 2 bytes)
/// b.write_bytes(b"hello");      // raw bytes (e.g. string field)
/// let data = b.finish();
/// ```
pub struct IffBuilder {
    id:      u32,
    payload: Vec<u8>,
}

impl IffBuilder {
    /// Start a new chunk with the given FOURCC `id`.
    pub fn new(id: u32) -> Self {
        IffBuilder { id, payload: Vec::new() }
    }

    /// Append `width` little-endian bytes of `value`.
    ///
    /// `width` must be 1, 2, or 4; it is silently clamped to `[1, 4]`.
    pub fn write_le(&mut self, value: i32, width: usize) {
        let bytes = value.to_le_bytes();
        let w = width.clamp(1, 4);
        self.payload.extend_from_slice(&bytes[..w]);
    }

    /// Append raw bytes (e.g. a string field's UTF-8 content).
    pub fn write_bytes(&mut self, b: &[u8]) {
        self.payload.extend_from_slice(b);
    }

    /// Return the current payload byte count (before the chunk header).
    pub fn payload_len(&self) -> usize {
        self.payload.len()
    }

    /// Wrap the accumulated payload with an IFF chunk header and alignment
    /// padding, consuming the builder.
    pub fn finish(self) -> Vec<u8> {
        write_chunk(self.id, &self.payload)
    }
}

// ── tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn write_then_read_round_trip() {
        let payload: &[u8] = &[0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00];
        let data  = write_chunk(str_to_id("TEST"), payload);
        let chunk = read_chunk(&data).unwrap();
        assert_eq!(id_to_str(chunk.id), "TEST");
        assert_eq!(chunk.payload, payload);
    }

    #[test]
    fn alignment_padding_excluded_from_payload() {
        // 3-byte payload → aligned to 4 → 8 header + 3 data + 1 pad = 12 bytes
        let data  = write_chunk(str_to_id("TSTX"), b"abc");
        assert_eq!(data.len(), 12);
        let chunk = read_chunk(&data).unwrap();
        assert_eq!(chunk.payload, b"abc");
    }

    #[test]
    fn read_multiple_chunks() {
        let mut data = write_chunk(str_to_id("AA  "), b"\x01\x02");
        data.extend(write_chunk(str_to_id("BB  "), b"\x03\x04\x05\x06"));
        let chunks = read_chunks(&data).unwrap();
        assert_eq!(chunks.len(), 2);
        assert_eq!(id_to_str(chunks[0].id), "AA");
        assert_eq!(id_to_str(chunks[1].id), "BB");
        assert_eq!(chunks[1].payload, &[0x03, 0x04, 0x05, 0x06]);
    }

    #[test]
    fn str_to_id_and_back() {
        assert_eq!(id_to_str(str_to_id("PLAY")), "PLAY");
        // Trailing space is stripped
        assert_eq!(id_to_str(str_to_id("RM0")), "RM0");
        assert_eq!(id_to_str(str_to_id("T")), "T");
    }

    #[test]
    fn builder_matches_write_chunk() {
        let mut b = IffBuilder::new(str_to_id("ABCD"));
        b.write_le(0x1234_i32, 4);
        b.write_bytes(b"hi");
        let built = b.finish();

        let mut expected_payload = vec![0x34, 0x12, 0x00, 0x00];
        expected_payload.extend_from_slice(b"hi");
        let expected = write_chunk(str_to_id("ABCD"), &expected_payload);

        assert_eq!(built, expected);
    }

    #[test]
    fn truncated_header_returns_error() {
        let err = read_chunk(&[0x54, 0x45, 0x53]).unwrap_err();
        assert!(err.message.contains("truncated"));
    }

    #[test]
    fn size_past_end_returns_error() {
        // Header says size=100 but buffer only has 8 bytes total
        let mut bad = vec![b'T', b'E', b'S', b'T'];
        bad.extend_from_slice(&100u32.to_le_bytes());
        let err = read_chunk(&bad).unwrap_err();
        assert!(err.message.contains("extends past end"));
    }
}
