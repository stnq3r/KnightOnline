SET NOCOUNT ON;
SELECT '==Moradon(zone21) NPCs==';
SELECT CAST(n.sSid AS varchar)+'|'+n.strName+'|grp='+CAST(n.iSellingGroup AS varchar)+'|pid='+CAST(n.sPid AS varchar)
FROM K_NPC n JOIN K_NPCPOS p ON n.sSid=p.NpcID
WHERE p.ZoneID=21 ORDER BY n.iSellingGroup, n.sSid;
SELECT '==Moradon merchants count by group==';
SELECT CAST(n.iSellingGroup AS varchar)+'|'+CAST(COUNT(*) AS varchar)
FROM K_NPC n JOIN K_NPCPOS p ON n.sSid=p.NpcID WHERE p.ZoneID=21 GROUP BY n.iSellingGroup;
GO
