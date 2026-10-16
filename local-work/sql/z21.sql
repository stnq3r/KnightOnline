SET NOCOUNT ON;
SELECT CAST(NpcID AS varchar)+'|L'+CAST(LeftX AS varchar)+','+CAST(TopZ AS varchar)+' R'+CAST(RightX AS varchar)+','+CAST(BottomZ AS varchar)+'|minX'+CAST(LimitMinX AS varchar)+' maxX'+CAST(LimitMaxX AS varchar)+'|minZ'+CAST(LimitMinZ AS varchar)+' maxZ'+CAST(LimitMaxZ AS varchar)
FROM K_NPCPOS WHERE ZoneID=21 AND NpcID BETWEEN 13000 AND 13020;
SELECT '--charon22301--';
SELECT CAST(NpcID AS varchar)+'|L'+CAST(LeftX AS varchar)+','+CAST(TopZ AS varchar)+' R'+CAST(RightX AS varchar)+','+CAST(BottomZ AS varchar)
FROM K_NPCPOS WHERE NpcID=22301;
GO
