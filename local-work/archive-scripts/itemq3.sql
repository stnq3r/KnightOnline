SET NOCOUNT ON;

PRINT '=== count with any (+N) ===';
SELECT COUNT(*) FROM ITEM WHERE strName LIKE '%(+%';

PRINT '=== first 30 by Num ===';
SELECT TOP 30 Num, strName, Kind, Class FROM ITEM ORDER BY Num;

PRINT '=== Kind=255 sample ===';
SELECT TOP 30 Num, strName, Kind FROM ITEM WHERE Kind=255 ORDER BY Num;

PRINT '=== Kind=210 sample ===';
SELECT TOP 20 Num, strName, Kind FROM ITEM WHERE Kind=210 ORDER BY Num;

PRINT '=== Kind=60 sample ===';
SELECT TOP 20 Num, strName, Kind FROM ITEM WHERE Kind=60 ORDER BY Num;

PRINT '=== distinct base name count (strip (+N)) ===';
SELECT COUNT(DISTINCT REPLACE(strName, ' (+0)', '')) FROM ITEM;
