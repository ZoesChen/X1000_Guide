#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <common.h>

#define PROC_NAME "/proc/mtd"
#define PART_NUM	12
static struct partition parti[PART_NUM];
static unsigned int partition_num;

int get_blocksize()
{
	return parti[0].erasesize;
}
struct partition *get_partition_info(char *name)
{
	int i;

	for (i = 0; i < partition_num; i++) {
		if(!strncmp(parti[i].name, name, strlen(name))) {
			break;
		}
	}

	if(i == partition_num)
		return NULL;
	return &parti[i];
}

int get_partitions()
{
	FILE *fp;
	char line[1024];
	char *tmp;
	char par[PART_NUM][32];
	int devices_num, i;

	fp = fopen(PROC_NAME, "r");
	if(fp < 0) {
		printf("open MTD %s failed\n", PROC_NAME);
		return -1;
	}

	devices_num = 0;
	while(!feof(fp)) {
		memset(line, 0, 1024);
		if(fgets(line, 1024, fp) == NULL)
			continue;
		if(strstr(line, "mtd")) {
			i = 0;
			tmp = strtok(line, " ");
			while(tmp != NULL) {
				strncpy(par[i], tmp, strlen(tmp));
				tmp = strtok(NULL, " ");
				i ++;
			}
			tmp = strtok(par[0], ":");
			strncpy(parti[devices_num].charname, tmp, strlen(tmp));
			sprintf(parti[devices_num].blockname, "mtdblock%d", devices_num);
			parti[devices_num].size = strtoul(par[1], NULL, 16);
			parti[devices_num].erasesize = strtoul(par[2], NULL, 16);
			if(devices_num > 0)
				parti[devices_num].offset = parti[devices_num - 1].size + parti[devices_num - 1].offset;
			tmp = strtok(par[3], "\"");
			strncpy(parti[devices_num].name, tmp, strlen(tmp));
			devices_num ++;
		}
	}
	partition_num  = devices_num;

	fclose(fp);
	return 0;
}
