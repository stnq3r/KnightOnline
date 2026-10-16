SET NOCOUNT ON;
SELECT '==254 SCROLL NPCs==';
SELECT CAST(n.sSid AS varchar)+'|'+n.strName+'|grp='+CAST(n.iSellingGroup AS varchar)+'|zone='+CAST(p.ZoneID AS varchar)+'|pos='+CAST(p.NumNPC AS varchar)
FROM K_NPC n LEFT JOIN K_NPCPOS p ON n.sSid=p.NpcID
WHERE n.iSellingGroup/1000=254;
SELECT '==231 RENTAL NPCs==';
SELECT CAST(n.sSid AS varchar)+'|'+n.strName+'|grp='+CAST(n.iSellingGroup AS varchar)+'|zone='+CAST(p.ZoneID AS varchar)+'|pos='+CAST(p.NumNPC AS varchar)
FROM K_NPC n LEFT JOIN K_NPCPOS p ON n.sSid=p.NpcID
WHERE n.iSellingGroup/1000=231;
SELECT '==241 skill/other unspawned merchants grp=0 with shop names==';
SELECT CAST(sSid AS varchar)+'|'+strName FROM K_NPC WHERE iSellingGroup=0 AND (strName LIKE '%croll%' OR strName LIKE '%Bless%' OR strName LIKE '%Remium%' OR strName LIKE '%rchand%' OR strName LIKE '%Potioneer%' OR strName LIKE '%Trader%');
GO
