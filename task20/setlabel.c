#include <stdio.h>
#include <string.h>
#include <stropts.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <linux/msdos_fs.h>
#include <sys/ioctl.h>

#define FAT_IOCTL_SET_VOLUME_LABEL	_IOW('r', 0x14, char[MSDOS_NAME])

int main(int argc, char **argv)
{
	char label[MSDOS_NAME];
	int fd;
	size_t len;
	int err;

	if (argc != 3) {
		fprintf(stderr, "usage: %s FS VOLUME_LABEL\n", argv[0]);
		return 1;
	}

	fd = open(argv[1], 0, O_RDWR);
	if (fd < 0) {
		perror("open");
		return 2;
	}

	memset(label, ' ', MSDOS_NAME);

	len = strlen(argv[2]);
	if (len > MSDOS_NAME)
		len = MSDOS_NAME;

	memcpy(label, argv[2], len);

	err = ioctl(fd, FAT_IOCTL_SET_VOLUME_LABEL, label);
	if (err) {
		perror("ioctl");
		return 3;
	}

	close(fd);

	return 0;
}
