#!/usr/bin/expect -f
set timeout 60
log_user 1
set pass $env(EXPECT_PASS)
set dest $env(EXPECT_DEST)

set cmd "echo '$pass' | sudo -S sh -c 'du -x -h -d2 / 2>/dev/null | sort -rh | head -30' ; echo ===DOCKER===; which docker; docker ps -a --format '{{.Names}} | {{.Image}} | {{.Status}}' 2>&1 | head -20; echo ===COMPOSE===; docker volume ls 2>&1"

spawn ssh $dest $cmd
expect {
    -re "assword:" { send -- "$pass\r"; exp_continue }
    -re "\[sudo\].*password" { send -- "$pass\r"; exp_continue }
    -re "(yes/no|fingerprint)" { send -- "yes\r"; exp_continue }
    eof
}
catch wait result
exit [lindex $result 3]