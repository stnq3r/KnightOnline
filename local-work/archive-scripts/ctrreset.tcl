#!/usr/bin/expect -f
set timeout 120
log_user 1
set pass $env(EXPECT_PASS)
set dest $env(EXPECT_DEST)

set cmd "echo '$pass' | sudo -S sh -c 'set -e; docker ps -a --format \"{{.Names}}\" | wc -l; docker volume ls -q | wc -l; systemctl stop docker containerd; rm -rf /var/lib/containerd; systemctl start docker; sleep 3; docker version --format \"server={{.Server.Version}}\"; df -h /'"

spawn ssh $dest $cmd
expect {
    -re "assword:" { send -- "$pass\r"; exp_continue }
    -re "\[sudo\].*password" { send -- "$pass\r"; exp_continue }
    -re "(yes/no|fingerprint)" { send -- "yes\r"; exp_continue }
    eof
}
catch wait result
exit [lindex $result 3]