SET NOCOUNT ON;
SELECT 'DANGLING_grp1000=' + CAST(n.iSellingGroup/1000 AS varchar) + ' (' + CAST(COUNT(*) AS varchar) + ' npc)'
FROM K_NPC n WHERE n.iSellingGroup<>0 AND NOT EXISTS(SELECT 1 FROM ITEM i WHERE i.SellingGroup=n.iSellingGroup/1000)
GROUP BY n.iSellingGroup/1000 ORDER BY 1;
SELECT 'NPCS_WITH_SHOP=' + CAST(COUNT(*) AS varchar) FROM K_NPC WHERE iSellingGroup<>0;
SELECT 'NPCS_EMPTY_SHOP=' + CAST(COUNT(*) AS varchar) FROM K_NPC WHERE iSellingGroup=0;
GO
