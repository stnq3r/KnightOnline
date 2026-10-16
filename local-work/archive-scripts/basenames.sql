SET NOCOUNT ON;
SELECT DISTINCT
  CASE WHEN CHARINDEX(' (+', strName) > 0
       THEN LTRIM(RTRIM(LEFT(strName, CHARINDEX(' (+', strName) - 1)))
       ELSE strName END AS base
FROM ITEM
ORDER BY base;
