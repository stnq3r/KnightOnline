import struct, os, glob

V = "/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode/v1279b/KnightOnline/Data"
DIRS = ["/Users/berkgulher/OpenKO-Client/assets/Client/Data",
        "/Users/berkgulher/OpenKO-Client/assets/Client2/Data"]

DWORD, BYTE, STRING, SHORT, WORD, INT, CHAR, FLOAT, DOUBLE = 6, 2, 7, 3, 4, 5, 1, 8, 9
SIZE = {1: 1, 2: 1, 3: 2, 4: 2, 5: 4, 6: 4, 8: 4, 9: 8}
TNAME = {6: "UInt32", 2: "Byte", 7: "String", 3: "Int16", 4: "UInt16", 5: "Int32", 1: "SByte", 8: "Float", 9: "Double"}
SIGNED = {1, 3, 5}


def decrypt(data):
    kr, c1, c2 = 0x0816, 0x6081, 0x1608
    out = bytearray(len(data))
    for i, c in enumerate(data):
        out[i] = c ^ ((kr >> 8) & 0xFF)
        kr = ((c + kr) * c1 + c2) & 0xFFFF
    return bytes(out)


def encrypt(data):
    kr, c1, c2 = 0x0816, 0x6081, 0x1608
    out = bytearray(len(data))
    for i, p in enumerate(data):
        c = p ^ ((kr >> 8) & 0xFF)
        out[i] = c
        kr = ((c + kr) * c1 + c2) & 0xFFFF
    return bytes(out)


def parse(blob):
    off = 0
    (n,) = struct.unpack_from("<i", blob, off); off += 4
    types = list(struct.unpack_from("<%di" % n, blob, off)); off += 4 * n
    (rc,) = struct.unpack_from("<i", blob, off); off += 4
    rows = []
    for _ in range(rc):
        cols = []
        for ty in types:
            if ty == STRING:
                (ln,) = struct.unpack_from("<i", blob, off); off += 4
                cols.append(blob[off:off + ln]); off += ln
            else:
                sz = SIZE.get(ty, 1); cols.append(blob[off:off + sz]); off += sz
        rows.append(cols)
    return types, rows


def serial(blob):
    off = 0
    (n,) = struct.unpack_from("<i", blob, off); off += 4
    types = list(struct.unpack_from("<%di" % n, blob, off)); off += 4 * n
    (rc,) = struct.unpack_from("<i", blob, off); off += 4
    return types, off, rc


def enc_val(v, ty):
    if ty == STRING:
        return struct.pack("<i", len(v)) + v
    return v


def write_tbl(types, rows, path):
    out = bytearray()
    out += struct.pack("<i", len(types))
    for t in types:
        out += struct.pack("<i", t)
    out += struct.pack("<i", len(rows))
    for r in rows:
        for v, ty in zip(r, types):
            out += enc_val(v, ty)
    open(path, "wb").write(encrypt(bytes(out)))


def write_csv(types, rows, path):
    lines = [",".join('"%d - %s"' % (i + 1, TNAME[t]) for i, t in enumerate(types))]
    for r in rows:
        cells = []
        for v, ty in zip(r, types):
            if ty == STRING:
                cells.append('"%s"' % v.decode("cp949"))
            else:
                iv = int.from_bytes(v, "little", signed=(ty in SIGNED))
                cells.append('"%d"' % iv)
        lines.append(",".join(cells))
    open(path, "w", encoding="utf-8").write("\n".join(lines) + "\n")


# ---- Item_Org : V1279 36-col -> add trailing byGrade(BYTE)=0 -> 37-col ----
org = open(os.path.join(V, "item_org_us.tbl"), "rb").read()
otypes, orows = parse(decrypt(org))
assert len(otypes) == 36, len(otypes)
new_types = otypes + [BYTE]
new_rows = [r + [b"\x00"] for r in orows]
print("Item_Org: %d rows, %d -> %d cols" % (len(new_rows), len(otypes), len(new_types)))
for d in DIRS:
    write_tbl(new_types, new_rows, os.path.join(d, "Item_Org_us.tbl"))
    write_csv(new_types, new_rows, os.path.join(d, "Item_Org_us.tbl.csv"))

# ---- Item_Ext : V1279 files are schema-identical -> copy verbatim + regenerate csv ----
for i in range(24):
    f = "Item_Ext_%d_us.tbl" % i
    src = os.path.join(V, f)
    blob = open(src, "rb").read()
    types, rows = parse(decrypt(blob))
    for d in DIRS:
        open(os.path.join(d, f), "wb").write(blob)  # verbatim encrypted copy
        write_csv(types, rows, os.path.join(d, f + ".csv"))

print("done")
