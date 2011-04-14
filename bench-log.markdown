����  gaisq - /data/workspace/youtube_data/data/gais_rec20110324 (~ 26G)

#Config 1:

Brief: ��T�� field �� sort�A�̤j buffer 400mb�A�w���C�Ӥ��Φܤ֦� 400(1 - 0.75) = 100 mb

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

Problem: Sort �� partition ��Ҥ��ӹ�١A�}�l debug/tune

#Config 2 (Improvement v1 Failure):

Brief: ���� field ��������ǡA�� favoriteCount �ը� title ���e�A���o���n���j�p���G�COptmize record �ƻs�����C

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

- ������M���C�A�������t�Z�CTune �L��o�� memory leak (sovled in the next version)

- std::sort ��������� std::stable_sort �C�A���᳣��� stable_sort

- Page fault �ƶq�C
	
#Config 3: (Improvement v2)

Brief: �ѨM memory leak �æA���C sort ���� (record::compare tuned)

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

- �A�׭��C��Ҥ���ٱ��ΡA�M�Ӥ���F�Ӧh�ɶ��b user mode�C

- Minor page fault �U���AMajor �W�[�C

#Config 4:

Brief: �W�[�O����ϥζq�� 800mb �A�w�������Τj�p�� 180mb�A����s�ª� record comparator ���į�t���C

##�ª� comparator
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
	
##�s�� comparator
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
