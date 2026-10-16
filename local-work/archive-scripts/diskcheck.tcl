#!/usr/bin/expect -f
set timeout 60
log_user 1
set pass $env(EXPECT_PASS)
set dest $env(EXPECT_DEST)

set cmd "echo '$pass' | sudo -S sh -c 'lsblk -o NAME,SIZE,FSTYPE,MOUNTPOINT; echo ===VALUABLE===; du -x -h -d2 /var/lib 2>/dev/null | sort -rh | head -15; echo ===DOCKERDF===; docker system df 2>&1; echo ===PS===; docker ps -a 2>&1 | head'"

spawn ssh $dest $cmd
expect {
    -re "assword:" { send -- "$pass\r"; exp_continue }
    -re "\[sudo\].*password" { send -- "$pass\r"; exp_continue }
    -re "(yes/no|fingerprint)" { send -- "yes\r"; exp_continue }
    eof
}
catch wait result
exit [lindex $result 3]