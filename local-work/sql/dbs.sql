SET NOCOUNT ON;
SELECT d.name + ' | ' + CAST(SUM(CASE WHEN df.type_desc='LOG' THEN 0 ELSE df.size END)*8/1024 AS varchar)+'MB data'
FROM sys.databases d
JOIN sys.master_files df ON df.database_id=d.database_id
GROUP BY d.name ORDER BY d.name;
GO
