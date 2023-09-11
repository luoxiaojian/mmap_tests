
COUNT=1073741824
for THREAD_NUM in 20
do
	for STRIDE in 1 4096
	do
		for SEQ in 1 0
		do
			for SZ in 10737418240
			do
				echo 1 > /proc/sys/vm/drop_caches
				./build/malloc_test ${THREAD_NUM} ${SEQ} ${COUNT} ${SZ} ${STRIDE}

				for HINT in 0
				do
					echo 1 > /proc/sys/vm/drop_caches
					./build/mmap_test /data/data ${THREAD_NUM} ${SEQ} ${HINT} ${COUNT} ${SZ} ${STRIDE}
				done
			done
		done
	done
done
