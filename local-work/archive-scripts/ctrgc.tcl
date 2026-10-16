#!/usr/bin/expect -f
set timeout 120
log_user 1
set pass $env(EXPECT_PASS)
set dest $env(EXPECT_DEST)

set cmd "echo '$pass' | sudo -S sh -c 'du -x -h -d1 /var/lib/containerd 2>/dev/null | sort -rh | head; echo ===; cat /etc/docker/daemon.json 2>/dev/null; echo ===CTR===; which ctr; sudo ctr content prune 2>&1 | tail -3'"

spawn ssh $dest $cmd
expect {
    -re "assword:" { send -- "$pass\r"; exp_continue }
    -re "\[sudo\].*password" { send -- "$pass\r"; exp_continue }
    -re "(yes/no|fingerprint)" { send -- "yes\r"; exp_continue }
    eof
}
catch wait result
exit [lindex $result 3]