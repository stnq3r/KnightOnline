SET NOCOUNT ON;
SELECT '==K_NPC 12301/22301==';
SELECT CAST(sSid AS varchar)+'|'+strName+'|sPid='+CAST(sPid AS varchar)+'|sSize='+CAST(sSize AS varchar)+'|grp='+CAST(iSellingGroup AS varchar)+'|type='+CAST(byType AS varchar)+'|act='+CAST(byActType AS varchar)+'|fam='+CAST(byFamily AS varchar)
FROM K_NPC WHERE sSid IN (12301,22301,7002);
SELECT '==K_NPCPOS row 22301 & 13007==';
SELECT CAST(NpcID AS varchar)+'|z='+CAST(ZoneID AS varchar)+'|ActType='+CAST(ActType AS varchar)+'|Regen='+CAST(RegenType AS varchar)+'|L'+CAST(LeftX AS varchar)+','+CAST(TopZ AS varchar)+',R'+CAST(RightX AS varchar)+','+CAST(BottomZ AS varchar)+'|minZ='+CAST(LimitMinZ AS varchar)+'|minX='+CAST(LimitMinX AS varchar)+'|maxX='+CAST(LimitMaxX AS varchar)+'|maxZ='+CAST(LimitMaxZ AS varchar)+'|num='+CAST(NumNPC AS varchar)+'|reg='+CAST(RegTime AS varchar)+'|dir='+CAST(byDirection AS varchar)+'|dot='+CAST(DotCnt AS varchar)+'|path='+CAST(path AS varchar)
FROM K_NPCPOS WHERE NpcID IN (22301,13007,13001);
SELECT '==full unique npcpos for zone21 merchants==';
SELECT CAST(NpcID AS varchar)+'|'+CAST(ZoneID AS varchar)+'|'+CAST(LeftX AS varchar)+'|'+CAST(TopZ AS varchar)+'|'+CAST(RightX AS varchar)+'|'+CAST(BottomZ AS varchar)
FROM K_NPCPOS WHERE ZoneID=21 AND NpcID IN (22301,13007,13001,13002);
GO
