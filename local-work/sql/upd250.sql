SET NOCOUNT ON;
SELECT '==BEFORE==';
SELECT CAST(Num AS varchar)+'|'+CAST(SellingGroup AS varchar) FROM ITEM WHERE SellingGroup=250 ORDER BY Num;
UPDATE ITEM SET SellingGroup=254 WHERE SellingGroup=250;
SELECT '==rows affected=='+CAST(@@ROWCOUNT AS varchar);
SELECT '==count 250 after=='+CAST(COUNT(*) AS varchar) FROM ITEM WHERE SellingGroup=250;
SELECT '==count 254 after=='+CAST(COUNT(*) AS varchar) FROM ITEM WHERE SellingGroup=254;
GO
