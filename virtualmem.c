#include"virtualmem.h"

int main (int argc, char* argv[])
{
	FILE *filePointer;
	pthread_t thread_id;
	int i=0;
	int numRead=0;
	char *map;
	int size,digits =5;
	struct stat sb;
	
	//map file
	filePointer = open(argv[1],O_RDONLY);
	fstat(filePointer, &sb);
	map = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, filePointer, 0);
	close(filePointer);

	//Initialize the transTable
	pthread_create(&thread_id, NULL, initTransTable, (void *)NULL);
	pthread_join(thread_id,NULL);
	
	//Initialize the pagetable
	pthread_create(&thread_id, NULL, initPageTable, (void *)NULL);
	pthread_join(thread_id,NULL);

	for(i=0;i<sb.st_size;i++)
	{
		while(map[i] != 10)
		{
			a.logAddr += pow(10,digits)*(map[i]-48);
			digits--;
			i++;
		}
		while(a.logAddr%10 == 0)
			a.logAddr/=10;
		digits = 5;
		pthread_create(&thread_id, NULL, extractor, (void *)NULL);
		pthread_join(thread_id,NULL);
		pthread_create(&thread_id, NULL, transTableLookup, (void *)NULL);
		pthread_join(thread_id,NULL);		
		printf("Virtual Address: %i,  Physical Address: %i,  Value : %i\n",a.logAddr,a.physAddr,a.value);
		numRead++;
		a.logAddr = 0;
	}
	printf("Number of translations: %d\n",numRead);
	printf("Page Faults: %d\n",pageFault);
	printf("Page Fault Rate: %.4f\n",(float)pageFault/(float)numRead);
	printf("TLB Hits: %d\n",tlbHit);
	printf("TLB Hit Rate: %.4f\n",(float)tlbHit/(float)numRead);

	munmap(map,sb.st_size);	//unmap mapped file

	return 0;
}

void *initTransTable(void * vp){
	int i;
	for(i=0; i<NUMTRANS; i++)
		transTable[i][0] = -1;

	pthread_exit(NULL);
}

void *initPageTable(void * vp){
	int i;
	for(i=0; i<NUMPAGES; i++)
		pageTable[i]=-1;

	pthread_exit(NULL);
}

//This function obtains the pagenum and the offset from the logical address
void *extractor(void * vp)
{
	a.pagenum = (a.logAddr & 0x0000FF00) >>(8);
	a.offset = a.logAddr & 0x000000FF;
	pthread_exit(NULL);
}

void *transTableLookup( void * vp){
	pthread_t thread_id;
	int i;
	int flag = 0;

	for ( i=0 ; i < NUMTRANS ; i++ ) 
	{
		if ( transTable[i][0] == a.pagenum )
		{
			a.physAddr=transTable[i][1]*PAGESIZE+a.offset;	  //transTable[i][1] stores FN associated with [i][0] PN
			flag = 1;
			tlbHit++;	
			break;
		}
	}
	
	//If pageNum not found inside transTable, look inside pageTable	
	if (flag == 0)
	{
		pthread_create(&thread_id, NULL, pageTableLookup, (void *)NULL);
		pthread_join(thread_id,NULL);

		//Update transTable
		transTable[transCount][0] = a.pagenum;
		transTable[transCount][1] = frameNum;
		transCount++;

		if (transCount == 16)
			transCount = 0;
	}

	return;
}

/* This function looks up the framenum from a pagenumbers, if the framenumber is not in the pagetable
 * we increase the pagefault flag and call the thread to bring the page from memory.
 */
void *pageTableLookup(void * vp){
	pthread_t thread_id;

	if( pageTable[a.pagenum] == -1){	
		pageTable[a.pagenum]=frameNum;
		pthread_create(&thread_id, NULL, backingStore, (void *)NULL);
		pthread_join(thread_id,NULL);
		frameNum++;
		pageFault++;
	}

	pthread_create(&thread_id, NULL, getValue, (void *)NULL);
	pthread_join(thread_id,NULL);

	a.physAddr=pageTable[a.pagenum]*PAGESIZE+a.offset;
	
	pthread_exit(NULL);
}

/*When a pagefault is generated we will bring the page that caused the pagefault from
 *the backing store to the physical memory
 */
void *backingStore(void *vp){
	//If page x caused a fault bring in that page to memory at the frameNum location
	char temp[NUMFRAMES];
	int i;

	FILE *filePointer;
	filePointer=fopen("BACKING_STORE.bin","rb");

   	 if( filePointer == NULL){
    		printf("\n fopen failed\n");
      	 	pthread_exit(NULL);
   	 }
	
	if( 0!=fseek(filePointer,a.pagenum*PAGESIZE,SEEK_SET) ){
		printf("\nfseek failed\n");
		pthread_exit(NULL);	
	
	}

	if(PAGESIZE!=fread(temp,1,PAGESIZE,filePointer) ){
		printf("\nfread failed\n");
		pthread_exit(NULL);
	}

	fclose(filePointer);

	for(i=0; i<PAGESIZE; i++)
		physMem[frameNum][i]=temp[i];

	pthread_exit(NULL);
}

// Reads the value from the physical memory that corresponds to the logical address
void *getValue(void *vp){
	a.value=physMem[pageTable[a.pagenum]][ a.offset ];//Read the value from physical memory
	pthread_exit(NULL);
}






