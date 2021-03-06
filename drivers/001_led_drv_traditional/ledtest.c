
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/* ledtest on
  * ledtest off
  */
int main(int argc, char **argv)
{
	int fd;
	unsigned char val = 1;
	fd = open("/dev/led", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
		return 0;
	}
	if (argc != 2)
	{
		printf("Usage :\n");
		printf("%s <on|off>\n", argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "on") == 0)
	{
		val  = 1;
	}
	else
	{
		val = 0;
	}
	
	write(fd, &val, 1);

    close(fd);
	return 0;
}
