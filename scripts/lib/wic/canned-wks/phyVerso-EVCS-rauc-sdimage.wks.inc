part --source bootimg-partition --fstype=vfat --label boot0 --ondisk mmc --active --align 4096 --offset 8192 --fixed-size 128
part --source bootimg-partition --fstype=vfat --label boot1 --ondisk mmc --active --fixed-size 128
part --source archive --sourceparams="file=config-partition.tar.gz" --fstype=ext4 --label config --ondisk mmc --fixed-size 32
part / --source rootfs --fstype=ext4 --label root0 --ondisk mmc --use-uuid --fixed-size 512
part / --source rootfs --fstype=ext4 --label root1 --ondisk mmc --use-uuid --fixed-size 512
part / --fstype=ext4 --label appfs0 --ondisk mmc --use-uuid --fixed-size 256
part / --fstype=ext4 --label appfs1 --ondisk mmc --use-uuid --fixed-size 256

