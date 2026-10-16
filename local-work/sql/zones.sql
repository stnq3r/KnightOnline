SET NOCOUNT ON;
SELECT '==ZONE INFO==';
SELECT CAST(ZoneNo AS varchar)+'|'+strZoneName+'|type='+CAST(Type AS varchar) FROM ZONE_INFO WHERE ZoneNo IN (1,2,21,32) ORDER BY ZoneNo;
SELECT '==unspawned group254/231==';
SELECT 'scan';
SELECT CAST(sSid AS varchar)+'|'+strName+'|grp='+CAST(iSellingGroup AS varchar)+'|spawned='+CASE WHEN EXISTS(SELECT 1 FROM K_NPCPOS p WHERE p.NpcID=K_NPC.sSid) THEN 'Y' ELSE 'N' END
FROM K_NPC WHERE iSellingGroup/1000 IN (231,254);
SELECT '==Offering/Premium spawned?==';
SELECT CAST(sSid AS varchar)+'|'+strName+'|'+CASE WHEN EXISTS(SELECT 1 FROM K_NPCPOS p WHERE p.NpcID=K_NPC.sSid) THEN 'Y' ELSE 'N' END
FROM K_NPC WHERE sSid IN (13014,16085);
GO
