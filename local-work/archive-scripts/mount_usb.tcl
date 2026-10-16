#!/usr/bin/expect -f
set timeout 60
log_user 1
set pass $env(EXPECT_PASS)
set dest $env(EXPECT_DEST)

set cmd "echo '$pass' | sudo -S sh -c 'set -e; id -u berkgulher; UUID=\$(blkid -s UUID -o value /dev/sdb1); echo UUID=\$UUID; mkdir -p /mnt/openko; mount -t exfat -o defaults,uid=1000,gid=1000,umask=022,noatime /dev/sdb1 /mnt/openko; grep -q /mnt/openko /etc/fstab || echo \"UUID=\$UUID /mnt/openko exfat defaults,uid=1000,gid=1000,umask=022,noatime,nofail,x-systemd.device-timeout=5 0 0\" >> /etc/fstab; df -h /mnt/openko; ls -la /mnt/openko | head -8'"

spawn ssh $dest $cmd
expect {
    -re "assword:" { send -- "$pass\r"; exp_continue }
    -re "\[sudo\].*password" { send -- "$pass\r"; exp_continue }
    -re "(yes/no|fingerprint)" { send -- "yes\r"; exp_continue }
    eof
}
catch wait result
exit [lindex $result 3]