
cd ~/pktgen

sudo ./app/app//x86_64-native-linuxapp-gcc/pktgen -c 0x1ff -n 3 -- -P -m "[1-2:3-4].0, [5-6:7-8].1"  -f ~/scripts/pktgen-scripts/regular-with-bin-mf.lua
