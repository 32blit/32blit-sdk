OUT=build/x-firmware.elf
arm-none-eabi-size -Ax $OUT
arm-none-eabi-readelf -e $OUT
arm-none-eabi-nm --print-size --size-sort --radix=x $OUT
arm-none-eabi-readelf -s $OUT |perl -ne 'if(/(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/) { print $3 . " " . $8. "\n";}'|sort -n | uniq
