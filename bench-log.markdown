測資  gaisq - /data/workspace/youtube_data/data/gais_rec20110324 (~ 26G)

#Config 1:

Brief: 對三個 field 做 sort，最大 buffer 400mb，預期每個分割至少有 400(1 - 0.75) = 100 mb

	time ../psort.exe -f ../gais_rec20110324 -M 400m -r 75 -k '@id:' STR -k '@title:' STR -k '@favoriteCount:' UINT '>'

	PIVOTS_CNT: 263 MAXMEM: 419430400       RESERVE: 75     STREAM SIZE: 104857600
	266 pivots generated
	Max record size: 45727

	partition: 	852.273676 		14.6541%
	sample: 	114.430618 		1.96754%
	sort: 		4849.224317 	83.3783%
	overall: 	5815.928627

	Time spent in user mode   (CPU seconds) : 5097.380s
	Time spent in kernel mode (CPU seconds) : 268.521s
	Total time                              : 1:36:55.97s
	CPU utilisation (percentage)            : 92.2%
	Times the process was swapped           : 0
	Times of major page faults              : 35
	Times of minor page faults              : 6728766

Problem: Sort 跟 partition 比例不太對稱，開始 debug/tune

#Config 2 (Improvement v1 Failure):

Brief: 改變 field 比較的順序，把 favoriteCount 調到 title 之前，取得較好的大小分佈。Optmize record 複製成本。

	time ../psort-s.exe -f ../gais_rec20110324 -M 400m -r 75 -k '@id:' STR -k '@favoriteCount:' UINT -k '@title:' STR

	PIVOTS_CNT: 263 MAXMEM: 419430400       RESERVE: 75     STREAM SIZE: 104857600
	266 pivots generated
	Max record size: 45727

	partition: 	914.687872 		29.9752%
	sample: 	146.356407 		4.79624%
	sort: 		1990.437809 	65.2286%
	overall: 	3051.482118

	Time spent in user mode   (CPU seconds) : 2201.537s
	Time spent in kernel mode (CPU seconds) : 277.179s
	Total time                              : 50:51.52s
	CPU utilisation (percentage)            : 81.2%
	Times the process was swapped           : 0
	Times of major page faults              : 3
	Times of minor page faults              : 10767414

Problem: 

- 比例雖然降低，但仍有差距。Tune 過後發生 memory leak (sovled in the next version)

- std::sort 不知為何比 std::stable_sort 慢，之後都改用 stable_sort

- Page fault 數量。
	
#Config 3: (Improvement v2)

Brief: 解決 memory leak 並再降低 sort 成本 (record::compare tuned)

	time ../psort-ng.exe -f ../gais_rec20110324 -M 400m -r 75 -k '@id:' STR -k '@favoriteCount:' UINT -k '@title:' STR

	PIVOTS_CNT: 263 MAXMEM: 419430400       RESERVE: 75     STREAM SIZE: 104857600
	266 pivots generated
	Max record size: 45727

	partition: 	1083.875904 	40.4165%
	sample: 	114.759175 		4.27924%
	sort: 		1483.132882 	55.3043%
	overall: 	2681.767978

	Time spent in user mode   (CPU seconds) : 2004.340s
	Time spent in kernel mode (CPU seconds) : 244.940s
	Total time                              : 44:41.82s
	CPU utilisation (percentage)            : 83.8%
	Times the process was swapped           : 0
	Times of major page faults              : 45
	Times of minor page faults              : 5291356

Problem: 

- 再度降低比例不對稱情形，然而仍花了太多時間在 user mode。

- Minor page fault 下降，Major 增加。

#Config 4:

Brief: 增加記憶體使用量至 800mb ，預期的分割大小為 180mb，比較新舊版 record comparator 的效能差異。

##舊版 comparator
	time ../psort-ocmp.exe -f ../gais_rec20110324 -M 800m -r 88 -k '@id:' STR -k '@favoriteCount:' UINT -k '@title:' STR
	
	PIVOTS_CNT: 274 MAXMEM: 838860800       RESERVE: 88     STREAM SIZE: 100663296
	278 pivots generated
	Max record size: 45727
	
	partition: 1115.840558 37.3184%
	sample: 106.620841 3.56585%
	sort: 1767.593378 59.1158%
	overall: 2990.054793

	Time spent in user mode   (CPU seconds) : 2312.095s
	Time spent in kernel mode (CPU seconds) : 258.689s
	Total time                              : 49:50.13s
	CPU utilisation (percentage)            : 85.9%
	Times the process was swapped           : 0
	Times of major page faults              : 61
	Times of minor page faults              : 5497791
	
##新版 comparator
	time ../psort-fcmp.exe -f ../gais_rec20110324 -M 800m -r 88 -k '@id:' STR -k '@favoriteCount:' UINT -k '@title:' STR

	PIVOTS_CNT: 274 MAXMEM: 838860800       RESERVE: 88     STREAM SIZE: 100663296
	278 pivots generated
	Max record size: 45727

	sort: 1495.188266
	partition: 972.824988 37.7694%
	sample: 107.663347 4.17997%
	sort: 1495.207545 58.0506%
	overall: 2575.695900

	Time spent in user mode   (CPU seconds) : 1854.316s
	Time spent in kernel mode (CPU seconds) : 253.025s
	Total time                              : 42:55.75s
	CPU utilisation (percentage)            : 81.8%
	Times the process was swapped           : 0
	Times of major page faults              : 48
	Times of minor page faults              : 5618849
