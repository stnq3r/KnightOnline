#!/bin/bash
set -e
SQL=$1
docker cp "$SQL" openko-db-sqlserver-1:/tmp/q.sql
docker exec openko-db-sqlserver-1 sh -c '/opt/mssql-tools*/bin/sqlcmd -S localhost -U sa -P "PUT_SA_PASSWORD_HERE" -C -d KN_online -i /tmp/q.sql -h -1 -W -s"|"'
