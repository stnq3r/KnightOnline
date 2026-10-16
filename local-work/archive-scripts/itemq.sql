SET NOCOUNT ON;

PRINT '=== Class distribution ===';
SELECT Class, COUNT(*) AS cnt FROM ITEM GROUP BY Class ORDER BY Class;

PRINT '=== Kind distribution ===';
SELECT Kind, COUNT(*) AS cnt FROM ITEM GROUP BY Kind ORDER BY Kind;

PRINT '=== names containing Class ===';
SELECT TOP 40 Num, strName, Kind, Class FROM ITEM WHERE strName LIKE '%Class%' ORDER BY Num;

PRINT '=== names containing Class Up ===';
SELECT TOP 40 Num, strName, Kind, Class FROM ITEM WHERE strName LIKE '%Upgrade%' ORDER BY Num;

PRINT '=== distinct scroll-ish names ===';
SELECT TOP 60 Num, strName, Kind, Class FROM ITEM WHERE strName LIKE '%Scroll%' ORDER BY Num;
