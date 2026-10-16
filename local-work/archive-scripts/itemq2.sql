SET NOCOUNT ON;

PRINT '=== rows with (+0) ===';
SELECT COUNT(*) FROM ITEM WHERE strName LIKE '%(+0)%';

PRINT '=== rows WITHOUT (+N) ===';
SELECT COUNT(*) FROM ITEM WHERE strName NOT LIKE '%(+)%';

PRINT '=== specific V1264 ids ===';
SELECT Num, strName, Kind, Class FROM ITEM WHERE Num IN (110110000,120110000,379016000,800029000,389050000) ORDER BY Num;

PRINT '=== distinct suffixes sample ===';
SELECT TOP 20 strName FROM ITEM WHERE strName LIKE '%(+)%' ORDER BY NEWID();

PRINT '=== count by trailing (+N) ===';
SELECT RIGHT(strName,3) AS suf, COUNT(*) AS cnt FROM ITEM WHERE strName LIKE '%(+)%' GROUP BY RIGHT(strName,3) ORDER BY suf;

PRINT '=== max Num ===';
SELECT MIN(Num), MAX(Num) FROM ITEM;
