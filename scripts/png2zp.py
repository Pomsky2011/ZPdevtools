#!/usr/bin/env python3
"""png2zp: PNG art (and Tiled maps) -> the ROM assets the layers C library loads.

  png2zp.py assets/assets.json -o build [--rom-base 0x110000] [--quantize[=N]]

Writes <out>/assets.bin (every piece 256-aligned, to be placed at --rom-base) and <out>/assets.h (where each piece sits, and how
many of each). The manifest is JSON; image paths are relative to it:

  {
    "rom_base": "0x110000",                       optional (--rom-base wins)
    "quantize": 48,                               optional: accept RGB(A) PNGs, reduce each one to at most that many colours
    "bg0": {"image": "ground.png"},               BG0 (tile bank 0, opaque unless it has transparent pixels)
    "bg1": {"tiled": "level.tmj", "layer": "fg"}, BG1 (tile bank 2, see-through); from a PNG or a Tiled map
    "sprites": [{"name": "hero", "image": "hero.png", "frame_w": 2, "frame_h": 2}],      (tile bank 1)
    "font": {"image": "font.png", "first": 32},   16 glyphs a row, 8x8, glyph i is ASCII first+i       (tile bank 3)
    "frame": "frame.png",                         one 8x8 tile: the border of a box
    "boxes": [{"name": "msg", "w": 28, "h": 5, "border": true, "pad": 2, "lines": ["HELLO", "", "WORLD"]}]
  }

Per image: "transparent": "alpha" (PNG transparency, the default when the PNG has any), "index0" (palette index 0 of an indexed PNG),
"#rrggbb" (a key colour) or "none". Per layer: "fill": "repeat" (default: the image tiles over the 64x64 map) or "clear";
"flips": true folds tiles that are mirror images of each other into one (the cell carries the flip bits).

The rules the PPU sets: one 256-entry palette shared by everything (RGB555); entries 0, 16, 32.. have alpha 0, so an opaque colour is
never given one of those indices and a transparent pixel is always index 0. Four tile banks of 256 tiles. Maps are 64x64 cells of
4 bytes. See ZPdevtools/docs/tools/png2zp.md.
"""
import argparse
import base64
import json
import os
import struct
import sys
import zlib

try:
    from PIL import Image
except ImportError:                                      # pragma: no cover
    sys.exit("png2zp needs Pillow (pip install pillow, or your distro's python-pillow)")

TRANSPARENT = -1
MAP_CELLS = 64
BANK_BG0, BANK_SPR, BANK_BG1, BANK_WIN = 0, 1, 2, 3     # the fixed bank convention
BANK_TILES = 256
SPRITE_CELLS = 256
WINDOW_POOL = 408


class AssetError(Exception):
    pass


def rgb555(r, g, b):
    """The key of a colour: the three 5-bit channels, so colours the console can't tell apart share an entry."""
    return ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3)


def pal_word(key):
    """BBBBBGGGGGRRRRR1: bit 0 is alpha."""
    r, g, b = (key >> 10) & 31, (key >> 5) & 31, key & 31
    return (b << 11) | (g << 6) | (r << 1) | 1


# ---- images ------------------------------------------------------------------------------------------------------------------------
class Img:
    def __init__(self, path, w, h, pixels, order):
        self.path, self.w, self.h = path, w, h
        self.pixels = pixels          # row-major: an RGB555 key, or TRANSPARENT
        self.order = order            # the keys in the order the artist's palette had them (stable palette allocation)

    def has_alpha(self):
        return TRANSPARENT in self.pixels


def _parse_key(s):
    s = s.lstrip("#")
    if len(s) != 6:
        raise AssetError("transparent colour must be #rrggbb, not %r" % s)
    return rgb555(int(s[0:2], 16), int(s[2:4], 16), int(s[4:6], 16))


def _flat(im):
    """The pixels of an image as a flat list (Pillow 12 deprecates getdata)."""
    return list(im.get_flattened_data()) if hasattr(im, "get_flattened_data") else list(im.getdata())


def load_image(path, transparent=None, quantize=0):
    """Read a PNG into an Img. Indexed PNGs are taken as they are; RGB(A) PNGs only with quantize > 0 (colours per image)."""
    try:
        im = Image.open(path)
        im.load()
    except (OSError, ValueError) as e:
        raise AssetError("%s: cannot read the image (%s)" % (path, e))
    w, h = im.size
    indexed = im.mode == "P"
    if not indexed and not quantize:
        raise AssetError("%s: is %s, not an indexed (palette) PNG. Convert it to indexed colour, or pass --quantize to reduce it "
                         "for you" % (path, im.mode))
    if quantize and not 1 <= quantize <= 240:
        raise AssetError("quantize has to be 1..240 colours (a console palette holds 240 opaque ones), not %d" % quantize)
    rgba = _flat(im.convert("RGBA"))
    has_t = (indexed and "transparency" in im.info) or im.mode in ("RGBA", "LA", "PA")
    mode = transparent or ("alpha" if has_t else "none")
    key_colour = None
    if mode not in ("alpha", "index0", "none"):
        key_colour = _parse_key(mode)
        mode = "key"
    if mode == "index0" and not indexed:
        raise AssetError('%s: "transparent": "index0" needs an indexed PNG' % path)

    raw = _flat(im) if indexed else None
    tmask = []
    for i, (r, g, b, a) in enumerate(rgba):
        if mode == "alpha":
            t = a < 128
        elif mode == "index0":
            t = raw[i] == 0
        elif mode == "key":
            t = rgb555(r, g, b) == key_colour
        else:
            t = False
        tmask.append(t)

    keys = [rgb555(r, g, b) for (r, g, b, a) in rgba]
    if indexed:
        pairs = {}
        for i, k in enumerate(keys):
            if not tmask[i] and raw[i] not in pairs:
                pairs[raw[i]] = k
        order = [pairs[i] for i in sorted(pairs)]       # artist's palette order, used entries only
    else:
        seen = []
        have = set()
        for i, k in enumerate(keys):
            if not tmask[i] and k not in have:
                have.add(k)
                seen.append(k)
        if len(seen) > quantize:
            q = Image.new("RGB", (w, h))
            q.putdata([(((k >> 10) & 31) << 3, ((k >> 5) & 31) << 3, (k & 31) << 3) for k in keys])
            q = q.quantize(colors=quantize, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
            qp = q.getpalette()
            keys = [rgb555(qp[3 * v], qp[3 * v + 1], qp[3 * v + 2]) for v in _flat(q)]
            seen = []
            have = set()
            for i, k in enumerate(keys):
                if not tmask[i] and k not in have:
                    have.add(k)
                    seen.append(k)
        order = seen
    pixels = [TRANSPARENT if tmask[i] else keys[i] for i in range(w * h)]
    return Img(path, w, h, pixels, order)


# ---- palette, tile banks -----------------------------------------------------------------------------------------------------------
class Palette:
    def __init__(self):
        self.index = {}
        self.entries = [0] * 256
        self.next = 1

    def add(self, keys):
        for k in keys:
            if k in self.index:
                continue
            while self.next < 256 and self.next % 16 == 0:
                self.next += 1                     # alpha-0 slots are for transparent pixels only
            if self.next >= 256:
                raise AssetError("more than 240 opaque colours in all the images together (indices 0, 16, 32.. are the "
                                 "see-through ones); reduce colours or pass --quantize with a smaller N")
            self.index[k] = self.next
            self.entries[self.next] = pal_word(k)
            self.next += 1

    def bytes(self):
        return b"".join(struct.pack("<H", e) for e in self.entries)


def _flip_h(t):
    return bytes(t[y * 8 + 7 - x] for y in range(8) for x in range(8))


def _flip_v(t):
    return bytes(t[(7 - y) * 8 + x] for y in range(8) for x in range(8))


class Bank:
    def __init__(self, number, name):
        self.number, self.name = number, name
        self.tiles = []
        self.ids = {}

    def add(self, tile, flips=False):
        """-> (tile id, hflip, vflip) such that the stored tile, flipped that way, is `tile`."""
        if tile in self.ids:
            return self.ids[tile], 0, 0
        if flips:
            for h, v in ((1, 0), (0, 1), (1, 1)):
                c = tile
                if h:
                    c = _flip_h(c)
                if v:
                    c = _flip_v(c)
                if c in self.ids:
                    return self.ids[c], h, v
        if len(self.tiles) >= BANK_TILES:
            raise AssetError("tile bank %d (%s) is full: more than %d different tiles%s" %
                             (self.number, self.name, BANK_TILES, "" if flips else " (\"flips\": true folds mirrored tiles)"))
        self.ids[tile] = len(self.tiles)
        self.tiles.append(tile)
        return self.ids[tile], 0, 0

    def blob(self):
        return b"".join(self.tiles)


def attr(bank, hflip=0, vflip=0, transparent=0):
    return 0x1000 | (bank & 3) | (0x04 if transparent else 0) | (0x08 if hflip else 0) | (0x10 if vflip else 0)


def cell(tile_id, bank, hflip=0, vflip=0):
    return struct.pack("<HH", tile_id, attr(bank, hflip, vflip))


def slice_tiles(img, pal, x0=0, y0=0, cols=None, rows=None, stride_x=8, stride_y=8):
    """The 8x8 tiles of an image region as palette-index bytes, row-major; a transparent pixel is index 0."""
    cols = img.w // 8 if cols is None else cols
    rows = img.h // 8 if rows is None else rows
    out = []
    for ty in range(rows):
        for tx in range(cols):
            px, py = x0 + tx * stride_x, y0 + ty * stride_y
            t = bytearray(64)
            for y in range(8):
                for x in range(8):
                    k = img.pixels[(py + y) * img.w + px + x]
                    t[y * 8 + x] = 0 if k == TRANSPARENT else pal.index[k]
            out.append(bytes(t))
    return out


def need_tile_aligned(img):
    if img.w % 8 or img.h % 8:
        raise AssetError("%s: %dx%d is not a multiple of 8 in both directions" % (img.path, img.w, img.h))


# ---- Tiled maps --------------------------------------------------------------------------------------------------------------------
GID_H, GID_V, GID_D, GID_HEX = 0x80000000, 0x40000000, 0x20000000, 0x10000000


def _tiled_gids(layer, path):
    data = layer.get("data")
    if isinstance(data, list):
        return data
    if layer.get("encoding") != "base64":
        raise AssetError("%s: tile layer %r has no usable data (infinite/chunked maps are not supported)" % (path, layer.get("name")))
    raw = base64.b64decode(data)
    comp = layer.get("compression", "")
    if comp == "zlib":
        raw = zlib.decompress(raw)
    elif comp == "gzip":
        raw = zlib.decompress(raw, 16 + zlib.MAX_WBITS)
    elif comp:
        raise AssetError("%s: %s-compressed layer data is not supported (use CSV, or base64 with zlib/gzip/none)" % (path, comp))
    return list(struct.unpack("<%dI" % (len(raw) // 4), raw))


def load_tiled(path, layer_name, transparent, quantize):
    """-> (width, height, [(gid-without-flags, hflip, vflip)], tilesets) with tilesets = [(firstgid, count, [tile pixels getter])]."""
    base = os.path.dirname(os.path.abspath(path))
    try:
        with open(path) as f:
            m = json.load(f)
    except (OSError, ValueError) as e:
        raise AssetError("%s: cannot read the Tiled map (%s); export it as JSON (.tmj / .json)" % (path, e))
    if m.get("orientation", "orthogonal") != "orthogonal":
        raise AssetError("%s: only orthogonal Tiled maps are supported" % path)
    if m.get("infinite"):
        raise AssetError("%s: infinite Tiled maps are not supported (turn off 'Infinite' in the map properties)" % path)
    if m.get("tilewidth") != 8 or m.get("tileheight") != 8:
        raise AssetError("%s: the map's tile size is %sx%s, it has to be 8x8" % (path, m.get("tilewidth"), m.get("tileheight")))
    layers = [l for l in m.get("layers", []) if l.get("type") == "tilelayer"]
    if layer_name is not None:
        layers = [l for l in layers if l.get("name") == layer_name]
    if not layers:
        raise AssetError("%s: no tile layer%s" % (path, " named %r" % layer_name if layer_name else ""))
    layer = layers[0]
    gids = _tiled_gids(layer, path)
    w, h = layer["width"], layer["height"]
    if w > MAP_CELLS or h > MAP_CELLS:
        raise AssetError("%s: the layer is %dx%d cells, the console's maps are %dx%d" % (path, w, h, MAP_CELLS, MAP_CELLS))
    if len(gids) != w * h:
        raise AssetError("%s: layer data has %d cells, expected %d" % (path, len(gids), w * h))
    cells = []
    for g in gids:
        if g & (GID_D | GID_HEX):
            raise AssetError("%s: a tile is rotated (diagonal flip); the cells only carry horizontal and vertical flips" % path)
        cells.append((g & 0x0FFFFFFF, 1 if g & GID_H else 0, 1 if g & GID_V else 0))
    sets = []
    for ts in m.get("tilesets", []):
        first = ts["firstgid"]
        tbase = base
        if "source" in ts:
            tpath = os.path.join(base, ts["source"])
            try:
                with open(tpath) as f:
                    ts = json.load(f)
            except (OSError, ValueError) as e:
                raise AssetError("%s: cannot read the tileset (%s); external tilesets must be JSON (.tsj)" % (tpath, e))
            tbase = os.path.dirname(os.path.abspath(tpath))
        if "image" not in ts:
            raise AssetError("%s: image-collection tilesets are not supported; use one tileset image" % path)
        if ts.get("tilewidth") != 8 or ts.get("tileheight") != 8:
            raise AssetError("%s: tileset %r has %sx%s tiles, they have to be 8x8" % (path, ts.get("name"), ts.get("tilewidth"),
                                                                                   ts.get("tileheight")))
        img = load_image(os.path.join(tbase, ts["image"]), transparent, quantize)
        margin, spacing = ts.get("margin", 0), ts.get("spacing", 0)
        cols = ts.get("columns") or (img.w - 2 * margin + spacing) // (8 + spacing)
        count = ts.get("tilecount") or cols * ((img.h - 2 * margin + spacing) // (8 + spacing))
        sets.append((first, count, img, cols, margin, spacing))
    sets.sort(key=lambda s: s[0])
    return w, h, cells, sets


# ---- the build ---------------------------------------------------------------------------------------------------------------------
class Assets:
    def __init__(self):
        self.pal = Palette()
        self.banks = {n: Bank(n, name) for n, name in ((BANK_BG0, "BG0"), (BANK_SPR, "sprites"), (BANK_BG1, "BG1"),
                                                         (BANK_WIN, "window"))}
        self.maps = {}                 # layer number -> 16 KiB
        self.has_alpha = {0: False, 1: False}
        self.sprites = []              # (name, cell0, w, h, frames)
        self.cells = []                # sprite cells, 4 bytes each
        self.boxes = []                # (name, w, h, cells bytes)
        self.font_ids = {}             # char -> tile id in bank 3
        self.frame_id = None


def _opts(spec, quantize):
    return spec.get("transparent"), spec.get("quantize", quantize)


def build_layer(a, layer, spec, base, quantize):
    bank = a.banks[BANK_BG0 if layer == 0 else BANK_BG1]
    flips = bool(spec.get("flips"))
    fill = spec.get("fill", "repeat")
    if fill not in ("repeat", "clear"):
        raise AssetError('bg%d: "fill" is "repeat" or "clear", not %r' % (layer, fill))
    clear_id = None

    def clear_cell():
        nonlocal clear_id
        if clear_id is None:
            clear_id = bank.add(bytes(64))[0]
        return cell(clear_id, bank.number)

    transparent, q = _opts(spec, quantize)
    if "image" in spec:
        img = load_image(os.path.join(base, spec["image"]), transparent, q)
        need_tile_aligned(img)
        if img.w > MAP_CELLS * 8 or img.h > MAP_CELLS * 8:
            raise AssetError("%s: %dx%d pixels, a map is %dx%d" % (img.path, img.w, img.h, MAP_CELLS * 8, MAP_CELLS * 8))
        a.pal.add(img.order)
        a.has_alpha[layer] = img.has_alpha()
        cw, ch = img.w // 8, img.h // 8
        src = []
        for t in slice_tiles(img, a.pal):
            tid, hf, vf = bank.add(t, flips)
            src.append(cell(tid, bank.number, hf, vf))
    elif "tiled" in spec:
        cw, ch, gcells, sets = load_tiled(os.path.join(base, spec["tiled"]), spec.get("layer"), transparent, q)
        cache = {}
        src = []
        for gid, mh, mv in gcells:
            if gid == 0:
                src.append(clear_cell())
                continue
            for first, count, img, cols, margin, spacing in sets:
                if first <= gid < first + count:
                    break
            else:
                raise AssetError("%s: tile gid %d belongs to no tileset" % (spec["tiled"], gid))
            if id(img) not in cache:
                a.pal.add(img.order)
                cache[id(img)] = img
            local = gid - first
            px, py = margin + (local % cols) * (8 + spacing), margin + (local // cols) * (8 + spacing)
            if px + 8 > img.w or py + 8 > img.h:
                raise AssetError("%s: tile gid %d lies outside the tileset image %s (%dx%d)" % (spec["tiled"], gid, img.path, img.w, img.h))
            t = slice_tiles(img, a.pal, px, py, 1, 1)[0]
            if any(img.pixels[(py + y) * img.w + px + x] == TRANSPARENT for y in range(8) for x in range(8)):
                a.has_alpha[layer] = True
            tid, hf, vf = bank.add(t, flips)
            # a flip set in Tiled applies on top of the tile: combine with the fold's own flip
            src.append(cell(tid, bank.number, hf ^ mh, vf ^ mv))
    else:
        raise AssetError('bg%d needs "image" or "tiled"' % layer)

    grid = []
    for y in range(MAP_CELLS):
        for x in range(MAP_CELLS):
            if fill == "repeat":
                grid.append(src[(y % ch) * cw + (x % cw)])
            else:
                grid.append(src[y * cw + x] if (x < cw and y < ch) else clear_cell())
    a.maps[layer] = b"".join(grid)


def build_sprites(a, specs, base, quantize):
    bank = a.banks[BANK_SPR]
    for s in specs:
        name = s.get("name")
        if not name or not name.replace("_", "").isalnum():
            raise AssetError("every sprite needs a \"name\" (letters, digits, _): %r" % s)
        transparent, q = _opts(s, quantize)
        img = load_image(os.path.join(base, s["image"]), transparent, q)
        need_tile_aligned(img)
        fw, fh = s.get("frame_w", img.w // 8), s.get("frame_h", img.h // 8)
        if fw < 1 or fh < 1 or img.w % (fw * 8) or img.h % (fh * 8):
            raise AssetError("%s: %dx%d pixels do not divide into frames of %dx%d tiles" % (img.path, img.w, img.h, fw, fh))
        a.pal.add(img.order)
        tiles = slice_tiles(img, a.pal)
        cols, rows = img.w // 8, img.h // 8
        fcols = cols // fw
        frames = fcols * (rows // fh)
        cell0 = len(a.cells)
        for f in range(frames):
            fx, fy = (f % fcols) * fw, (f // fcols) * fh
            for ty in range(fh):
                for tx in range(fw):
                    tid = bank.add(tiles[(fy + ty) * cols + fx + tx])[0]
                    a.cells.append(cell(tid, BANK_SPR))
        if len(a.cells) > SPRITE_CELLS:
            raise AssetError("sprites use %d cells, the console holds %d" % (len(a.cells), SPRITE_CELLS))
        a.sprites.append((name, cell0, fw, fh, frames))
    a.cells = b"".join(a.cells)


def build_font_and_boxes(a, spec, base, quantize):
    bank = a.banks[BANK_WIN]
    font = spec.get("font")
    if not font:
        if spec.get("boxes"):
            raise AssetError('"boxes" need a "font"')
        return
    first = font.get("first", 32)
    transparent, q = _opts(font, quantize)
    img = load_image(os.path.join(base, font["image"]), transparent, q)
    need_tile_aligned(img)
    if img.has_alpha():
        raise AssetError("%s: the font has transparent pixels; window boxes are opaque ink, so paint the background in" % img.path)
    a.pal.add(img.order)
    for i, t in enumerate(slice_tiles(img, a.pal)):
        a.font_ids[chr(first + i)] = bank.add(t)[0]
    if "frame" in spec:
        fimg = load_image(os.path.join(base, spec["frame"]), None, quantize)
        if fimg.w != 8 or fimg.h != 8:
            raise AssetError("%s: the frame is %dx%d, it has to be one 8x8 tile" % (fimg.path, fimg.w, fimg.h))
        if fimg.has_alpha():
            raise AssetError("%s: the frame has transparent pixels" % fimg.path)
        a.pal.add(fimg.order)
        a.frame_id = bank.add(slice_tiles(fimg, a.pal)[0])[0]
    blank = a.font_ids.get(" ")
    for b in spec.get("boxes", []):
        name, w, h = b.get("name"), b["w"], b["h"]
        if not name or not name.replace("_", "").isalnum():
            raise AssetError("every box needs a \"name\": %r" % b)
        border = b.get("border", False)
        if border and a.frame_id is None:
            raise AssetError('box %s has a border but there is no "frame" image' % name)
        if blank is None:
            raise AssetError("the font has no space glyph (first=%d, %d glyphs)" % (first, len(a.font_ids)))
        pad = b.get("pad", 1 if border else 0)
        top = 1 if border else 0
        lines = b.get("lines", [])
        if top + len(lines) > h - (1 if border else 0):
            raise AssetError("box %s: %d lines do not fit in %d rows" % (name, len(lines), h))
        out = []
        for ty in range(h):
            text = lines[ty - top] if 0 <= ty - top < len(lines) else ""
            if pad + len(text) > w - (1 if border else 0):
                raise AssetError("box %s: line %r is too wide for %d columns" % (name, text, w))
            for tx in range(w):
                if border and (ty in (0, h - 1) or tx in (0, w - 1)):
                    out.append(cell(a.frame_id, BANK_WIN))
                    continue
                k = tx - pad
                ch = text[k] if 0 <= k < len(text) else " "
                if ch not in a.font_ids:
                    raise AssetError("box %s: %r is not in the font (ASCII %d..%d)" % (name, ch, first, first + len(a.font_ids) - 1))
                out.append(cell(a.font_ids[ch], BANK_WIN))
        a.boxes.append((name, w, h, b"".join(out)))
    if sum(w * h for _, w, h, _ in a.boxes) > WINDOW_POOL:
        raise AssetError("the boxes hold %d cells, the window's pool is %d" % (sum(w * h for _, w, h, _ in a.boxes), WINDOW_POOL))


def build(manifest_path, quantize=0):
    """-> an Assets with every piece converted (nothing written)."""
    base = os.path.dirname(os.path.abspath(manifest_path))
    try:
        with open(manifest_path) as f:
            spec = json.load(f)
    except (OSError, ValueError) as e:
        raise AssetError("%s: cannot read the manifest (%s)" % (manifest_path, e))
    quantize = spec.get("quantize", quantize) or 0
    a = Assets()
    a.rom_base = int(str(spec.get("rom_base", "0x110000")), 0)
    if "bg0" in spec:
        build_layer(a, 0, spec["bg0"], base, quantize)
    if "bg1" in spec:
        build_layer(a, 1, spec["bg1"], base, quantize)
    build_sprites(a, spec.get("sprites", []), base, quantize)
    build_font_and_boxes(a, spec, base, quantize)
    return a


def write(a, out_dir, rom_base=None):
    rom_base = a.rom_base if rom_base is None else rom_base
    blob = bytearray()
    addr = {}

    def put(name, data):
        while len(blob) % 256:
            blob.append(0)
        addr[name] = rom_base + len(blob)
        blob.extend(data)

    put("PAL", a.pal.bytes())
    counts = {}
    for n in (0, 1, 2, 3):
        b = a.banks[n]
        if b.tiles:
            put("TILES%d" % n, b.blob())
            counts["TILES%d_N" % n] = len(b.tiles)
    if a.cells:
        put("CELLS", a.cells)
        counts["CELLS_N"] = len(a.cells) // 4
    for name, w, h, data in a.boxes:
        put("BOX_" + name.upper(), data)
        counts["BOX_%s_N" % name.upper()] = w * h
    for layer in sorted(a.maps):
        put("MAP%d" % layer, a.maps[layer])
    while len(blob) % 256:
        blob.append(0)
    os.makedirs(out_dir, exist_ok=True)
    with open(os.path.join(out_dir, "assets.bin"), "wb") as f:
        f.write(bytes(blob))
    with open(os.path.join(out_dir, "assets.h"), "w") as f:
        f.write("/* generated by png2zp.py: where the assets sit in ROM, and how many of each (do not edit) */\n")
        for name, v in addr.items():
            f.write("#define ASSET_%s 0x%06XUL\n" % (name, v))
        for name, v in counts.items():
            f.write("#define ASSET_%s %d\n" % (name, v))
        for layer in sorted(a.maps):
            f.write("#define ASSET_BG%d_HAS_ALPHA %d\n" % (layer, 1 if a.has_alpha[layer] else 0))
        for name, cell0, w, h, frames in a.sprites:
            u = name.upper()
            f.write("#define ASSET_SPR_%s_CELL %d\n#define ASSET_SPR_%s_W %d\n#define ASSET_SPR_%s_H %d\n"
                    "#define ASSET_SPR_%s_FRAMES %d\n" % (u, cell0, u, w, u, h, u, frames))
        for name, w, h, _ in a.boxes:
            f.write("#define ASSET_BOX_%s_W %d\n#define ASSET_BOX_%s_H %d\n" % (name.upper(), w, name.upper(), h))
    return addr, len(blob)


def main(argv=None):
    ap = argparse.ArgumentParser(description="PNG art and Tiled maps -> ZeroPoint layers-library assets")
    ap.add_argument("manifest")
    ap.add_argument("-o", "--out", default="build", help="output directory (default build)")
    ap.add_argument("--rom-base", help="where assets.bin is placed in ROM (default: the manifest's rom_base, 0x110000)")
    ap.add_argument("--quantize", nargs="?", const=48, type=int, default=0, metavar="N",
                    help="accept RGB(A) PNGs: reduce each to at most N colours (default 48)")
    args = ap.parse_args(argv)
    try:
        a = build(args.manifest, args.quantize)
        rb = int(args.rom_base, 0) if args.rom_base else None
        addr, size = write(a, args.out, rb)
    except AssetError as e:
        print("png2zp: error: %s" % e, file=sys.stderr)
        return 1
    print("%s: %d bytes of assets, %d palette colours" % (os.path.join(args.out, "assets.bin"), size, len(a.pal.index)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
