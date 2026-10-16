#!/bin/sh
docker cp /tmp/chk.sql openko-db-sqlserver-1:/tmp/chk.sql
docker exec openko-db-sqlserver-1 sh -c "/opt/mssql-tools*/bin/sqlcmd -S localhost -U sa -P PUT_SA_PASSWORD_HERE -C -d KN_online -i /tmp/chk.sql -h -1 -W -s|"
