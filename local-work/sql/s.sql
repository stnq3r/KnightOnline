SET NOCOUNT ON;
SELECT strItem, strSerial FROM USERDATA WHERE strUserId='Stnq3r';
SELECT WarehouseData, strSerial FROM WAREHOUSE WHERE strAccountID=(SELECT strAccountID FROM USERDATA WHERE strUserId='Stnq3r');
GO
