# local-work — OpenKO (Mac/Wine) project backup

Backup of everything we changed/generated while getting the OpenKO client running
on macOS (Wine + DXVK) against a local server, and aligning the database with a
V1279 retail client's data tables.

## Repositories

| Repo | What | Where |
|------|------|-------|
| `stnq3r/KnightOnline` | Game client+server source (fork of `Open-KO/KnightOnline`) | https://github.com/stnq3r/KnightOnline |
| `stnq3r/OpenKO-db` | DB setup SQL (fork of `Open-KO/OpenKO-db`) — our ITEM/MAGIC changes | https://github.com/stnq3r/OpenKO-db |
| upstream | `Open-KO/KnightOnline`, `Open-KO/OpenKO-db`, `Open-KO/ko-client-assets` | git remotes |

Local clone of the game repo: `~/OpenKO-Client` (remote `origin`=fork, `upstream`=Open-KO).

## Layout of this folder

- `client-data/` — the client `Data/*.tbl` files we changed (V1279 alignment),
  mirroring `assets/Client/Data/`. Includes `Item_Org_us.tbl`, `Item_Ext_*_us.tbl`,
  `Skill_Magic_*`, and a few `Object/*` + `Server.ini.default`.
- `db-tools/` — scripts that generate the OpenKO-db `ManualSetup` SQL:
  - `gen_item.py`   — builds `6_InsertData_ITEM.sql` from V1279 Item_Org/Item_Ext.
  - `gen_magic.py`  — builds `6_InsertData_MAGIC*.sql` from `__TABLE_UPC_SKILL`.
  - `gen_sql.py`, `gen_sql_merged.py`, `create_magic.sql` — helpers/schema.
  - `runq.sh`, `run_sql.sh` — run a `.sql` against the DB container via sqlcmd
    (SA password is a placeholder `PUT_SA_PASSWORD_HERE`; set your own).
  - `apply_magic.sql`, `backup_magic.sql`.
- `client-tools/` — `tbl.py` (parse/decrypt) and `merge250.py` (encrypt + in-place edit).
- `analysis/` — ad-hoc analysis scripts (`an_*.py`, `corr*.py`, `cmp_magic.py`, ...).
- `sql/` — ad-hoc queries used while investigating the DB.
- `archive-scripts/` — helper scripts/notes from the V1279 extraction workspace
  (`build_v1279.py`, `parse_tbl.py`, `tbldec.py`, `configs/`, ...).

## Client table (.tbl) crypto

Tables are XOR-encrypted with a rolling 16-bit key. Plain ⇄ ciphertext:

```
key = 0x0816; c1 = 0x6081; c2 = 0x1608
decrypt: plain[i] = cipher[i] ^ ((key >> 8) & 0xff); key = ((cipher[i] + key) * c1 + c2) & 0xffff
encrypt: cipher[i] = plain[i] ^ ((key >> 8) & 0xff); key = ((cipher[i] + key) * c1 + c2) & 0xffff
```

`Item_Org_us.tbl` in the installed client has **37 columns** (extra `byGrade`, idx 36);
`bySellGroup` is **index 35**. The V1279 archive copy has 36 columns.

## Shop group merge (250 → 254)

Group 250 held 9 "Blessed" scrolls with no NPC assigned to it, so they never showed
in any shop. Merged into group 254 (Rodion / Charon scroll vendors):

- Client: `assets/Client/Data/Item_Org_us.tbl` (+ `assets/Client2/Data/...`, hard-linked
  — same inode) — set `bySellGroup 250 → 254` for the 9 rows.
- DB: `UPDATE ITEM SET SellingGroup = 254 WHERE SellingGroup = 250;` (9 rows).
- Repo: `6_InsertData_ITEM.sql` updated (only those 9 rows change; md5 `cd970c18...`).

Rollback: `UPDATE ITEM SET SellingGroup = 250 WHERE Num IN
(379021000,379022000,379023000,379024000,379025000,379030000,379031000,379032000,379033000);`

## DB alignment to V1279 (ITEM + MAGIC)

- `ITEM` regenerated from V1279 `Item_Org`/`Item_Ext`:
  `Num = dwID_base + dwID_ext`, ext match `E.dwBaseID==0 || E.dwBaseID==O.dwID`,
  `SellingGroup = O.bySellGroup`, `MAX_ITEM_EXTENSION = 24`.
  **ReqStr/Sta/Dex/Intel/Cha use a conditional sum** (`o + (e if o!=0 else 0)`, not plain SUM).
  Droprate=0, SellPrice=0. Result: `ITEM` = 66316 rows.
- `MAGIC` (+ `TYPE1/2/3/4/6/7/9`) regenerated from `__TABLE_UPC_SKILL` (30 cols).
  T5/T8 left unchanged (no V1279 source). Result: `MAGIC` = 1434 rows.
- Applied to `KN_online` and verified; committed to `stnq3r/OpenKO-db`.

## Environment facts

- Server host: `blitzarc` @ `192.168.0.114` (ssh key `~/.ssh/openko_deploy`).
- DB: MS SQL in container `openko-db-sqlserver-1`, db `KN_online`, user `sa`.
- Client runs on Mac via Wine (`run-mac-wine-dxvk.sh`), server on 15001/15100.
- No secrets are committed here (SA password is a placeholder; `knight/knight` is the
  OpenKO-documented default DB user).

## Not backed up here (intentionally)

- The ~613 MB V1279 retail client (`.../T/opencode/v1279b`) — third-party retail game
  assets; too large and not our IP. Only its helper scripts/notes were kept
  (`archive-scripts/`). Its temp-dir copy may be cleaned by macOS.
- A full `KN_online` DB dump — the schema + setup data is reproducible from
  `stnq3r/OpenKO-db` (`ManualSetup/*.sql`).
