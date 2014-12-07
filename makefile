vmem: virtualmem.h virtualmem.c
	gcc -pthread -o vmem virtualmem.c -lm
