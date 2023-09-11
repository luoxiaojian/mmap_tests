ITER=4
THREAD_NUM=20

for SZ in 10737418240
do
  # rnd
  SEQ=0
  echo 1 > /proc/sys/vm/drop_caches
  ./build/malloc_test ${THREAD_NUM} ${SEQ} ${ITER} ${SZ}
  for HINT in 0
  do
    echo 1 > /proc/sys/vm/drop_caches
    ./build/mmap_test /data/data ${THREAD_NUM} ${SEQ} ${HINT} ${ITER} ${SZ}
  done

  # seq
  SEQ=1
  for STRIDE in 1 4096
  do
    echo 1 > /proc/sys/vm/drop_caches
    ./build/malloc_test ${THREAD_NUM} ${SEQ} ${ITER} ${SZ} ${STRIDE}
    for HINT in 0
    do
      echo 1 > /proc/sys/vm/drop_caches
      ./build/mmap_test /data/data ${THREAD_NUM} ${SEQ} ${HINT} ${ITER} ${SZ} ${STRIDE}
    done
  done
done
