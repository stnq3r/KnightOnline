SET NOCOUNT ON;
SELECT CAST(iSellingGroup AS varchar) + '|' + CAST(COUNT(*) AS varchar) + '|' + MIN(strName)
FROM K_NPC WHERE iSellingGroup<>0 GROUP BY iSellingGroup ORDER BY iSellingGroup;
SELECT '--NPCCOUNTS--';
SELECT CAST(COUNT(*) AS varchar) FROM K_NPC WHERE iSellingGroup<>0;
GO
