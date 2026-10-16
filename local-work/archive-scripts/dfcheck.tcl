#!/usr/bin/expect -f
set timeout 45
log_user 1
set pass $env(EXPECT_PASS)
set dest $env(EXPECT_DEST)

spawn ssh $dest "df -h; echo ===; df -i | head -15; echo ===; ls -la ~ | head; echo ===; du -sh ~ 2>/dev/null"

expect {
    -re "assword:" { send -- "$pass\r"; exp_continue }
    -re "(yes/no|fingerprint)" { send -- "yes\r"; exp_continue }
    eof
}
catch wait result
exit [lindex $result 3]