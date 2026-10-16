SET NOCOUNT ON;
SELECT '==RENTAL(231)==';
SELECT CAST(sSid AS varchar)+'|'+strName+'|'+CAST(sPid AS varchar)+'|'+CAST(iSellingGroup AS varchar) FROM K_NPC WHERE iSellingGroup/1000=231;
SELECT '==NAME SCAN scroll/bless/upgrade/premium/merchant==';
SELECT CAST(sSid AS varchar)+'|'+strName+'|'+CAST(iSellingGroup AS varchar) FROM K_NPC WHERE (strName LIKE '%croll%' OR strName LIKE '%less%' OR strName LIKE '%pgrade%' OR strName LIKE '%remium%' OR strName LIKE '%erchant%' OR strName LIKE '%Shop%' OR strName LIKE '%ale%');
SELECT '==HAS 254/255==';
SELECT CAST(sSid AS varchar)+'|'+strName+'|'+CAST(iSellingGroup AS varchar) FROM K_NPC WHERE iSellingGroup/1000 IN (254,255);
GO
