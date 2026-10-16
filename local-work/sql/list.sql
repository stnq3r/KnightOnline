SET NOCOUNT ON;
SELECT t.name AS tbl, p.rows AS rows, SUM(a.total_pages)*8/1024 AS kb
FROM sys.tables t
JOIN sys.indexes i ON i.object_id=t.object_id AND i.index_id IN (0,1)
JOIN sys.partitions p ON p.object_id=t.object_id AND p.index_id=i.index_id
JOIN sys.allocation_units a ON a.container_id=p.partition_id
GROUP BY t.name, p.rows
ORDER BY kb DESC;
SELECT 'TOTAL_TABLES=' + CAST(COUNT(*) AS varchar) FROM sys.tables;
GO
