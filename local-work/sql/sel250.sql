SET NOCOUNT ON;
SELECT '==count 250==';
SELECT CAST(COUNT(*) AS varchar) FROM ITEM WHERE SellingGroup=250;
SELECT '==rows==';
SELECT CAST(Num AS varchar)+'|'+strName+'|Kind='+CAST(Kind AS varchar)+'|SellingGroup='+CAST(SellingGroup AS varchar) FROM ITEM WHERE SellingGroup=250 ORDER BY Num;
GO
