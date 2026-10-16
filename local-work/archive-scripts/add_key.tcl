#!/usr/bin/expect -f
set timeout 45
log_user 0
set pass $env(EXPECT_PASS)
set dest $env(EXPECT_DEST)
set pubfile $env(EXPECT_PUBFILE)

set fh [open $pubfile r]
set pub [string trim [read -nonewline $fh]]
close $fh

set remote "mkdir -p ~/.ssh && chmod 700 ~/.ssh && touch ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys && (grep -qs openko-deploy ~/.ssh/authorized_keys || printf '%s\\n' '$pub' >> ~/.ssh/authorized_keys)"

spawn ssh -o StrictHostKeyChecking=accept-new $dest $remote

expect {
    -re "assword:" { send -- "$pass\r"; exp_continue }
    -re "(yes/no|fingerprint)" { send -- "yes\r"; exp_continue }
    eof
}
catch wait result
exit [lindex $result 3]