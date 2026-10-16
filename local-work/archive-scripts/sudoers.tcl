#!/usr/bin/expect -f
set timeout 60
log_user 1
set pass $env(EXPECT_PASS)
set dest $env(EXPECT_DEST)

set cmd "echo '$pass' | sudo -S sh -c 'echo \"berkgulher ALL=(ALL) NOPASSWD:ALL\" > /etc/sudoers.d/openko && chmod 440 /etc/sudoers.d/openko && visudo -c -f /etc/sudoers.d/openko && mkdir -p /opt/openko/build && chown -R berkgulher:berkgulher /opt/openko && ls -ld /opt/openko/build'"

spawn ssh $dest $cmd
expect {
    -re "assword:" { send -- "$pass\r"; exp_continue }
    -re "\[sudo\].*password" { send -- "$pass\r"; exp_continue }
    -re "(yes/no|fingerprint)" { send -- "yes\r"; exp_continue }
    eof
}
catch wait result
exit [lindex $result 3]