SET NOCOUNT ON;
SELECT 'OLD_BAK_GROUP=' + CAST(SellingGroup AS varchar) + ' -> ' + CAST(COUNT(*) AS varchar) FROM ITEM_BAK_20261015 WHERE SellingGroup<>0 GROUP BY SellingGroup ORDER BY SellingGroup;
GO
