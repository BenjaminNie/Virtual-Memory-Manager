#define NUMFRAMES 256
#define NUMPAGES 256
#define NUMTRANS 16
#define	PAGESIZE 256
//TODO:ADD CONSTANTS FOR DIFFERENT NUMBERS ie maxpagesize....

#include "stdlib.h"
#include "stdio.h"
#include <pthread.h>
#include <math.h>
//
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

struct addr	//structure to hold each address
{
	int logAddr;	//logical address found in text file
	int pagenum;	//page number (bits 8-15)
	int offset;	//offset (bits 0-7)
	int physAddr;
	int value;
};

// No need to allocate memory since we use global variables
int pageTable[NUMPAGES];
int transTable[NUMTRANS][2];  //[][0] = page number, [][1] = frame number
int physMem[NUMFRAMES][PAGESIZE];
int pageFault=0;
int frameNum=0;
int transCount=0;   //value must be from 0-15
int tlbHit=0;
//int physAdd=0;
//int value=0;

struct addr a = {0x00000000,0,0,0,0};

void *backingStore(void *vp);
void *pageTableLookup(void * vp);
void *transTableLookup(void * vp);
void *extractor(void * vp);
void *initPageTable(void * vp);
void *initTransTable(void * vp);
void *getValue(void *vp);

