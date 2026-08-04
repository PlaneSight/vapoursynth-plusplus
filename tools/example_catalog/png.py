from __future__ import annotations

import struct
import zlib

import vapoursynth as vs

from .model import FrameSelection, RenderedImage


def clamp_unit(value: float) -> float:
    return min(1.0, max(0.0, value))


def sample_to_byte(value: float) -> int:
    return round(clamp_unit(value) * 255.0)


def render_frame(selection: FrameSelection) -> RenderedImage:
    frame = selection.clip.get_frame(selection.index)
    color_family = frame.format.color_family
    pixels = bytearray()

    if color_family == vs.GRAY:
        plane = frame[0]
        for y in range(frame.height):
            for x in range(frame.width):
                value = sample_to_byte(float(plane[y, x]))
                pixels.extend((value, value, value))
    elif color_family == vs.RGB:
        planes = (frame[0], frame[1], frame[2])
        for y in range(frame.height):
            for x in range(frame.width):
                pixels.extend(sample_to_byte(float(plane[y, x])) for plane in planes)
    else:
        raise RuntimeError(f"cannot render {frame.format.name}; expected Gray or RGB")

    return RenderedImage(frame.width, frame.height, bytes(pixels))


def png_chunk(kind: bytes, payload: bytes) -> bytes:
    body = kind + payload
    return struct.pack(">I", len(payload)) + body + struct.pack(">I", zlib.crc32(body))


def encode_png(image: RenderedImage) -> bytes:
    stride = image.width * 3
    scanlines = b"".join(
        b"\x00" + image.pixels[offset : offset + stride]
        for offset in range(0, len(image.pixels), stride)
    )
    header = struct.pack(">IIBBBBB", image.width, image.height, 8, 2, 0, 0, 0)
    return (
        b"\x89PNG\r\n\x1a\n"
        + png_chunk(b"IHDR", header)
        + png_chunk(b"IDAT", zlib.compress(scanlines, level=9))
        + png_chunk(b"IEND", b"")
    )


def decode_png(encoded: bytes) -> RenderedImage:
    if not encoded.startswith(b"\x89PNG\r\n\x1a\n"):
        raise RuntimeError("catalog asset is not a PNG")

    cursor = 8
    width = height = 0
    compressed = bytearray()
    while cursor < len(encoded):
        payload_size = struct.unpack_from(">I", encoded, cursor)[0]
        kind = encoded[cursor + 4 : cursor + 8]
        payload = encoded[cursor + 8 : cursor + 8 + payload_size]
        cursor += 12 + payload_size
        if kind == b"IHDR":
            width, height, bit_depth, color_type, compression, filtering, interlace = (
                struct.unpack(">IIBBBBB", payload)
            )
            if (bit_depth, color_type, compression, filtering, interlace) != (
                8,
                2,
                0,
                0,
                0,
            ):
                raise RuntimeError("catalog PNG must be non-interlaced 8-bit RGB")
        elif kind == b"IDAT":
            compressed.extend(payload)
        elif kind == b"IEND":
            break

    stride = width * 3
    scanlines = zlib.decompress(compressed)
    expected_size = height * (stride + 1)
    if len(scanlines) != expected_size:
        raise RuntimeError("catalog PNG has unexpected decompressed size")

    pixels = bytearray()
    for offset in range(0, len(scanlines), stride + 1):
        if scanlines[offset] != 0:
            raise RuntimeError("catalog PNG uses an unsupported row filter")
        pixels.extend(scanlines[offset + 1 : offset + stride + 1])
    return RenderedImage(width, height, bytes(pixels))
