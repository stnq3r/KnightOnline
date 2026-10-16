#!/usr/bin/expect -f
set timeout 900
log_user 1
set pass $env(EXPECT_PASS)
set dest $env(EXPECT_DEST)

set cmd "echo '$pass' | sudo -S sh -c 'set -e; export DEBIAN_FRONTEND=noninteractive; apt-get update -qq; apt-get install -y -qq build-essential cmake ninja-build git pkg-config unixodbc unixodbc-dev curl ca-certificates gnupg lsb-release; echo ===DONE===; gcc --version | head -1; cmake --version | head -1; git --version'"

spawn ssh $dest $cmd
expect {
    -re "assword:" { send -- "$pass\r"; exp_continue }
    -re "\[sudo\].*password" { send -- "$pass\r"; exp_continue }
    -re "(yes/no|fingerprint)" { send -- "yes\r"; exp_continue }
    eof
}
catch wait result
exit [lindex $result 3]