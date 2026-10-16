#!/usr/bin/expect -f
set timeout 60
log_user 1
set pass $env(EXPECT_PASS)
set dest $env(EXPECT_DEST)

set cmd "echo '$pass' | sudo -S lsblk -o NAME,SIZE,FSTYPE,LABEL,MOUNTPOINT,MODEL"

spawn ssh $dest $cmd
expect {
    -re "assword:" { send -- "$pass\r"; exp_continue }
    -re "\[sudo\].*password" { send -- "$pass\r"; exp_continue }
    -re "(yes/no|fingerprint)" { send -- "yes\r"; exp_continue }
    eof
}
catch wait result
exit [lindex $result 3]