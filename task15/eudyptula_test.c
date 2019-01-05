#include <stdio.h>
#include <unistd.h>

#define SYS_eudyptula 335

int main()
{
	int h = 0xcaffe;
	int l = 0x0;
	printf("sys_eudyptula(0x%x, 0x%x) = %d\n", h, l,
			syscall(SYS_eudyptula, h, l));

	h = 0xc001;
	l = 0xdeadbeef;
	printf("sys_eudyptula(0x%x, 0x%x) = %d\n", h, l,
			syscall(SYS_eudyptula, h, l));

	return 0;
}
