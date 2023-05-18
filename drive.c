#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/kfifo.h>
#include <linux/vmalloc.h>
#include <linux/kern_levels.h>
#include <linux/wait.h>

#define BUFF_SIZE 32
#define MY_IOCTL_CALC 1
DECLARE_WAIT_QUEUE_HEAD(write_queue); 
DECLARE_WAIT_QUEUE_HEAD(read_queue); 
DEFINE_KFIFO(myfifo, int, 8);

dev_t devno;
int major = 255;
const char DEVNAME[] = "calcdev";
int write_num[1];
int ioctl_num[3];
int flag=0;
static int output_data[8]={0};//output区，便于查看
static int input_data[8]={0};//input区，便于查看
static int read_wait = 0;
static int write_wait =0;
static int read_condition =0;
static int write_condition = 0;

int hust_open(struct inode * ip, struct file * fp)
{
    printk("%s : %d\n", __func__, __LINE__);
    
    /* 一般用来做初始化设备的操作 */
    return 0;
}

int hust_close(struct inode * ip, struct file * fp)
{
    printk("%s : %d\n", __func__, __LINE__);
    
    /* 一般用来做和open相反的操作，open申请资源，close释放资源 */
    return 0;
}

ssize_t hust_read(struct file * fp, char __user * buf, size_t count, loff_t * loff)
{
 
//    printk("%s : %d\n", __func__, __LINE__);
    if (kfifo_len(&myfifo)<count)
    {
//阻塞
    read_wait = count;
    read_condition =1;
    printk("Read进程被阻塞\n");
    wait_event_interruptible(read_queue, kfifo_len(&myfifo)>=count);
    
        return 1;
    }
    else{
    flag=kfifo_out(&myfifo, output_data, count); 
    copy_to_user(buf,output_data,4*count);   
    printk(KERN_ALERT"read   %d",flag); 

    if(kfifo_avail(&myfifo)>=write_wait&&write_condition==1){
    wake_up_interruptible(&write_queue);     //唤醒被挂起的进程
    write_condition=0;
    printk("唤醒Write进程\n");

}

    return count;
    }
}

ssize_t hust_write(struct file * fp, const char __user * buf, size_t count, loff_t * loff)
{
   
//    printk("%s : %d\n", __func__, __LINE__);
    if (  kfifo_avail(&myfifo)<count)
    {
//将程序阻塞，加入阻塞队列
    write_wait=count;
    write_condition =1;
    printk("Write进程被阻塞\n");
    wait_event_interruptible(write_queue, kfifo_avail(&myfifo)>=write_wait);
    
        return -1;
    }
    else{
//先写，写完之后判断能不能唤醒读的程序
    copy_from_user(input_data,buf,4*count);
    flag=kfifo_in(&myfifo, input_data,count);
    printk(KERN_ALERT"write   %d",flag); 
    printk(KERN_ALERT"缓冲区目前的长度是    %d\n",kfifo_len(&myfifo));
if(kfifo_len(&myfifo)>=read_wait&&read_condition==1){
    wake_up_interruptible(&read_queue);     //唤醒被挂起的进程
    read_condition=0;
    printk("唤醒Read进程\n");
}


    return count;
    }
    
}

long hust_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    switch(cmd){
        case MY_IOCTL_CALC:
            if (copy_from_user(ioctl_num, (int __user *)arg, sizeof(int[3]))) {
                return -1;
            }
            switch(ioctl_num[2]){
                case 1: 
                return ioctl_num[0]+ioctl_num[1];
                break;

                case 2: 
                return ioctl_num[0]-ioctl_num[1];
                break;

                case 3: 
                return (ioctl_num[0]>ioctl_num[1])?ioctl_num[0]:ioctl_num[1];
                break;
            }
        
    }
    return -1;
}

//分配file_operations结构体
struct file_operations hust_fops = {
    .owner = THIS_MODULE,
    .open  = hust_open,
    .release = hust_close,
    .read = hust_read,
    .write = hust_write,
    .unlocked_ioctl = hust_ioctl,
};
struct cdev cdev;

//开始
static int hust_init(void)
{
    int ret;
    printk("%s : %d\n", __func__, __LINE__);
    
    //生成并注册设备号
    devno = MKDEV(major, 0);
    ret  = register_chrdev_region(devno, 1, DEVNAME);
    if (ret != 0)
    {
        printk("%s : %d fail to register_chrdev_region\n", __func__, __LINE__);
        return -1;
    }
    printk(KERN_ALERT "zhe shi ce shi ri zhi ");   
//zhe shi yi ge ce shi !!!
 
    //分配、设置、注册cdev结构体 */
    cdev.owner = THIS_MODULE;
    ret = cdev_add(&cdev, devno, 1);
    cdev_init(&cdev, &hust_fops);
    if (ret < 0)
    {
        printk("%s : %d fail to cdev_add\n", __func__, __LINE__);
        return -1;
    }
    printk("register successfully!\n");
    return 0;
}

//结束
static void hust_exit(void)
{
    printk("%s : %d\n", __func__, __LINE__);
    //释放资源
    cdev_del(&cdev);
    unregister_chrdev_region(devno, 1);
}

MODULE_LICENSE("GPL");
module_init(hust_init);
module_exit(hust_exit);
