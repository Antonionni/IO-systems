#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>

static dev_t first;
static struct cdev c_dev; 
static struct class *cl;
static char* WORK_FILE = "work_file";
static struct file * file;
static bool starts_with(const char *, const char *);
static char* count_symbols(const char*);

static int my_open(struct inode *i, struct file *f) {
  printk(KERN_INFO "Driver: open()\n");
  return 0;
}

static int my_close(struct inode *i, struct file *f) {
  printk(KERN_INFO "Driver: close()\n");
  return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
	if (file == NULL) {
		  printk(KERN_INFO "Driver: cannot read from not opened file()\n");
		  return -1;
	}
  char* data = kcalloc(len, sizeof(char), GFP_USER);

  set_fs(KERNEL_DS);
  if (off != NULL) {
  	size_t wlen = vfs_read(file, data, len, off);
  }
  set_fs(USER_DS);

  if (strlen(data) == 0) {
    kfree(data);
    return 0;
  }

  printk(KERN_INFO "Driver: read()\n");

  if(copy_to_user(buf, data, len) != 0) {
    kfree(data);
    return -EFAULT;
  }

  kfree(data);
  return len;
}

static ssize_t my_write(struct file *f, const char __user *buf,  size_t len, loff_t *off) {
  char *data = kcalloc(len, sizeof(char), GFP_USER);
  if (copy_from_user(data, buf, len) != 0) {
    kfree(data);
    return -EFAULT;
  }
  if (starts_with(data, "open ")) { 
    if (file != NULL) {
      filp_close(file, NULL);
    }
    int fileNameLen = strlen(data) - 5;
    char subbuff[fileNameLen];
    memcpy( subbuff, &data[5], fileNameLen);
    subbuff[fileNameLen] = '\0';
    WORK_FILE = subbuff;

    file = filp_open(WORK_FILE, O_RDWR|O_CREAT, 0644);
    kfree(data);
    return len;
  } else if (starts_with(data, "close")) { 
    if (file != NULL) {
        filp_close(file, NULL);
        file = NULL;
    } else {
		  printk(KERN_INFO "Driver: cannot close file which is not opened()\n");
	  }
    kfree(data);
	  return len;
  } else if (file == NULL) {
	  printk(KERN_INFO "Driver: cannot write to not opened file()\n");
    kfree(data);
	  return -1;
  }
  printk(KERN_INFO "string = %s", data);
  set_fs(KERNEL_DS);
  char * newData = count_symbols(data);
  size_t wlen = vfs_write(file, newData, len, &file->f_pos);
  
  set_fs(USER_DS);

  printk(KERN_INFO "Driver: write() len = %ld, %lld\n", len, file->f_pos);
  
  kfree(data);
  return len;
}

static struct file_operations mychdev_fops = {
  .owner = THIS_MODULE,
  .open = my_open,
  .release = my_close,
  .read = my_read,
  .write = my_write
};
 
static bool starts_with(const char *a, const char *b) {
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

int countDigit(long long n) { 
    int count = 0; 
    while (n != 0) { 
        n = n / 10; 
        ++count; 
    } 
    return count; 
}

static char* count_symbols(const char* string) {
  int len = strlen(string);
  char* result = kcalloc(countDigit(len - 1) + 1, sizeof(char), GFP_USER);
  sprintf(result, "%d\n", len - 1);

  printk(KERN_INFO "result: %s", result);
  return result;
}

static int __init ch_drv_init(void) {
  printk(KERN_INFO "Hello!\n");
  if (alloc_chrdev_region(&first, 0, 1, "ch_dev") < 0) {
    return -1;
  }
  if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL) {
	  unregister_chrdev_region(first, 1);
	  return -1;
  }
  if (device_create(cl, NULL, first, NULL, "var2") == NULL) {
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
  }
  cdev_init(&c_dev, &mychdev_fops);
    
  if (cdev_add(&c_dev, first, 1) == -1) {
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
  }
  return 0;
}
 
static void __exit ch_drv_exit(void) {
  cdev_del(&c_dev);
  device_destroy(cl, first);
  class_destroy(cl);
  unregister_chrdev_region(first, 1);
  printk(KERN_INFO "Bye!!!\n");
}
 
module_init(ch_drv_init);
module_exit(ch_drv_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("The first kernel module");

