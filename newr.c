#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#define MY_IOCTL_CALC 1

int main(char argc, char * argv[])
{

    
    int fd;
    int ret;
    int state[1];
    int state_result;
    int buf[3] = {128,64,1};
    int p = 0;
    int num =0;
    int array[8] ={8,7,6,5,4,3,2,1};
    int out[8] = {8,8,8,8,8,9,9,9};
    int mid[3]={3,3,3};
    int i = 0;
int n =0;

     //将要打开的文件的路径通过main函数的参数传入
    
    fd = open("/dev/calcdev", O_RDWR);
    if (fd < 0)
    {
        perror("fail to open file\n");
        return -1;
    }

    printf("输入任意数开始程序:\n");
    scanf("%d",&n);
    
    for(int i=1;i>0;i++){
    read(fd,out,3);
   }

    close(fd);
    return 0;
}
