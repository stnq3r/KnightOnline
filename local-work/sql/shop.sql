SET NOCOUNT ON;
SELECT '==NPC_SELLING_GROUPS==';
SELECT 'grp=' + CAST(iSellingGroup AS varchar) + ' -> ' + CAST(COUNT(*) AS varchar) + ' npc' FROM K_NPC GROUP BY iSellingGroup ORDER BY iSellingGroup;
SELECT '==ITEM_GROUPS==';
SELECT 'grp=' + CAST(SellingGroup AS varchar) + ' -> ' + CAST(COUNT(*) AS varchar) + ' items' FROM ITEM WHERE SellingGroup<>0 GROUP BY SellingGroup ORDER BY SellingGroup;
SELECT '==DANGLING_NPCGROUPS==';
SELECT 'grp=' + CAST(n.iSellingGroup AS varchar) FROM K_NPC n
 WHERE n.iSellingGroup<>0 AND NOT EXISTS(SELECT 1 FROM ITEM i WHERE i.SellingGroup=n.iSellingGroup)
 GROUP BY n.iSellingGroup ORDER BY 1;
GO
