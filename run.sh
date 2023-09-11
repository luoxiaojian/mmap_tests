
echo 1 > /proc/sys/vm/drop_caches
./build/mmap_test /data/data 20 0 100000000 10737418240 4096
