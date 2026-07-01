#!/usr/bin/env python3
# Parst die CONST-Byte-Arrays (Sprites) aus SPACDATA.PAS und erzeugt sprites.h.
# Erhaelt die Original-BGI-Bytes exakt.
import re, sys, io

SRC = sys.argv[1] if len(sys.argv) > 1 else r"..\..\SPACDATA.PAS"
OUT = sys.argv[2] if len(sys.argv) > 2 else r"..\sprites.h"

# Nur diese Bezeichner uebernehmen (die Sprite-/Bild-Konstanten):
WANTED = ["Jet","Meteorit","MeteoritLoesch","Ufo","UfoLoesch","Leben",
          "LebenLoesch","Barrel","Ship1","Wuerfel","Brocken"]

text = open(SRC, "r", encoding="latin-1").read()

# Bereich zwischen 'CONST' (das mit den Arrays) und dem folgenden Prozedur-Block.
# Wir suchen einfach jede Definition der Form  Name : Array [...] OF Byte = (...);
pat = re.compile(
    r"(\b[A-Za-z_]\w*)\s*:\s*Array\s*\[([^\]]*)\]\s*OF\s*Byte\s*=\s*\((.*?)\)\s*;",
    re.IGNORECASE | re.DOTALL)

def parse_dims(dimstr):
    # "1..487" -> [487] ; "0..4,1..135" -> [5,135]
    dims = []
    for part in dimstr.split(","):
        lo, hi = part.split("..")
        dims.append(int(hi) - int(lo) + 1)
    return dims

def numbers(body):
    return [int(n) for n in re.findall(r"-?\d+", body)]

out = io.StringIO()
out.write("/* Auto-generiert aus SPACDATA.PAS durch tools/extract_sprites.py.\n")
out.write("   Original-Sprite-Bytes im Turbo-Pascal BGI-GetImage-Format (CGA, 2bpp).\n")
out.write("   NICHT von Hand editieren. */\n")
out.write("#ifndef SPRITES_H\n#define SPRITES_H\n\n")

found = {}
for m in pat.finditer(text):
    name, dimstr, body = m.group(1), m.group(2), m.group(3)
    if name not in WANTED:
        continue
    dims = parse_dims(dimstr)
    nums = numbers(body)
    total = 1
    for d in dims:
        total *= d
    if len(nums) != total:
        sys.stderr.write(f"WARN {name}: erwartet {total} Werte, gefunden {len(nums)}\n")
    found[name] = (dims, nums)

    if len(dims) == 1:
        out.write(f"static const unsigned char {name}[{dims[0]}] = {{\n")
        for i in range(0, len(nums), 16):
            out.write("  " + ",".join(f"{v:3d}" for v in nums[i:i+16]) + ",\n")
        out.write("};\n\n")
    elif len(dims) == 2:
        rows, cols = dims
        out.write(f"static const unsigned char {name}[{rows}][{cols}] = {{\n")
        for r in range(rows):
            chunk = nums[r*cols:(r+1)*cols]
            out.write("  {\n")
            for i in range(0, len(chunk), 16):
                out.write("    " + ",".join(f"{v:3d}" for v in chunk[i:i+16]) + ",\n")
            out.write("  },\n")
        out.write("};\n\n")
    else:
        sys.stderr.write(f"WARN {name}: >2 Dimensionen nicht unterstuetzt\n")

out.write("#endif /* SPRITES_H */\n")

missing = [w for w in WANTED if w not in found]
if missing:
    sys.stderr.write("FEHLEND: " + ", ".join(missing) + "\n")

open(OUT, "w", encoding="utf-8").write(out.getvalue())
sys.stderr.write("OK -> " + OUT + " (" + ", ".join(found.keys()) + ")\n")
