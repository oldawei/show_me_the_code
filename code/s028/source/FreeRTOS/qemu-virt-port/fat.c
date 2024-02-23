#include "ff_virtio_blk_disk.h"
#include "ff_stdio.h"

#define SECTORS  (2048*64) // 64MB
void fat_init(void) // init only can be called in freertos task
{
    FF_Disk_t *disk = FF_VirtioDiskInit("/oldawei", SECTORS, 1024);
	FF_VirtioDiskShowPartition(disk);

	ff_mkdir( "/oldawei/test" );
}

u32 fat_read_counter(void)
{
    FF_Stat_t stat;
	const char filename[] = "/oldawei/test/counter.txt";
	int is_exist = ff_stat(filename, &stat) != -1;
	FF_FILE *pxFile = ff_fopen( filename,  is_exist ? "r+" : "w" );

	u32 my_counter = 0;
	ff_rewind(pxFile);
	u32 flen = ff_filelength(pxFile);
	printf("flen: %d\n", flen);
	if (flen != 0) {
		ff_fread( &my_counter, sizeof( my_counter ), 1, pxFile );
		printf("old counter: 0x%08x\n", my_counter);
		my_counter += 1;
	}
	ff_rewind(pxFile);
	ff_fwrite( &my_counter, sizeof(my_counter), 1, pxFile );
	ff_fclose( pxFile );
	printf("new counter: 0x%08x\n", my_counter);

    return my_counter;
}