/*
 * Lab problem set for UNIX programming course
 * by Cheng Han Lee
 * License: GPLv2
 */
#include <linux/module.h>	// included for all kernel modules
#include <linux/kernel.h>	// included for KERN_INFO
#include <linux/init.h>		// included for __init and __exit macros
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/errno.h>
#include <linux/sched.h>	// task_struct requried for current_uid()
#include <linux/cred.h>		// for current_uid();
#include <linux/slab.h>		// for kmalloc/kfree
#include <linux/uaccess.h>	// copy_to_user
#include <linux/string.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include "kshram.h"

#define NUMBER_OF_DEVICE 8
static dev_t devnum;
static struct cdev c_dev;
static struct class *clazz;

struct kshram_device {
    void *mem;
    size_t size;
	struct mutex lock;
};

static struct kshram_device kshram_devices[NUMBER_OF_DEVICE];


static int kshram_dev_open(struct inode *i, struct file *f) {
	printk(KERN_INFO "kshram: device opened.\n");
	return 0;
}

static int kshram_dev_close(struct inode *i, struct file *f) {
	printk(KERN_INFO "kshram: device closed.\n");
	return 0;
}

static ssize_t kshram_dev_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
    int dev_idx = MINOR(f->f_path.dentry->d_inode->i_rdev);
    size_t size_to_copy = min((size_t)len, (size_t)(kshram_devices[dev_idx].size - *off));

    if (copy_to_user(buf, kshram_devices[dev_idx].mem + *off, size_to_copy)) {
        return -EFAULT;
    }
    
	*off += size_to_copy;
	printk(KERN_INFO "kshram%d: read %zu bytes @ %llu, current device memory size: %zu.\n", dev_idx, size_to_copy, *off, kshram_devices[dev_idx].size);
    
	return size_to_copy;
}

static ssize_t kshram_dev_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
    int dev_idx = MINOR(f->f_path.dentry->d_inode->i_rdev);
    size_t size_to_copy = min((size_t)len, (size_t)(kshram_devices[dev_idx].size - *off));

    if (copy_from_user(kshram_devices[dev_idx].mem + *off, buf, size_to_copy)) {
        return -EFAULT;
    }

    *off += size_to_copy;
	printk(KERN_INFO "kshram%d: write %zu bytes @ %llu, current device memory size: %zu.\n", dev_idx, size_to_copy, *off, kshram_devices[dev_idx].size);
    
	return size_to_copy;
}

static long kshram_dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) {
    int dev_idx = MINOR(fp->f_path.dentry->d_inode->i_rdev);
    size_t new_size;

    switch (cmd) {
        case KSHRAM_GETSLOTS:
            printk(KERN_INFO "kshram: ioctl KSHRAM_GETSLOTS - %d slots available.\n", NUMBER_OF_DEVICE);
            return NUMBER_OF_DEVICE;

        case KSHRAM_GETSIZE:
            printk(KERN_INFO "kshram: ioctl KSHRAM_GETSIZE for device %d - size: %zu.\n", dev_idx, kshram_devices[dev_idx].size);
            return kshram_devices[dev_idx].size;

        case KSHRAM_SETSIZE:
            new_size = (size_t)arg;
            kshram_devices[dev_idx].mem = krealloc(kshram_devices[dev_idx].mem, new_size, GFP_KERNEL);
            if (kshram_devices[dev_idx].mem) {
                kshram_devices[dev_idx].size = new_size;
                printk(KERN_INFO "kshram: ioctl KSHRAM_SETSIZE for device %d - new size: %zu.\n", dev_idx, kshram_devices[dev_idx].size);
            } else {
                printk(KERN_ERR "kshram: ioctl KSHRAM_SETSIZE for device %d - memory allocation failed.\n", dev_idx);
                return -ENOMEM;
            }
            break;

        default:
            printk(KERN_WARNING "kshram: ioctl unknown cmd=%u arg=%lu.\n", cmd, arg);
            return -EINVAL;
    }

    return 0;
}

// Implement the kshram_dev_mmap() function
static int kshram_dev_mmap(struct file *filp, struct vm_area_struct *vma) {
    int dev_idx = MINOR(filp->f_path.dentry->d_inode->i_rdev);
    struct kshram_device *dev = &kshram_devices[dev_idx];
	unsigned long pfn = vmalloc_to_pfn(dev->mem);
    unsigned long size = vma->vm_end - vma->vm_start;

	printk(KERN_INFO "kshram/mmap: idx %d size %zu\n", dev_idx, dev->size);

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    if (size > dev->size) {
        mutex_unlock(&dev->lock);
        return -EINVAL;
    }

    if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot)) {
        mutex_unlock(&dev->lock);
        return -EAGAIN;
    }

    mutex_unlock(&dev->lock);

    return 0;
}

static const struct file_operations kshram_dev_fops = {
	.owner = THIS_MODULE,
	.open = kshram_dev_open,
	.read = kshram_dev_read,
	.write = kshram_dev_write,
	.unlocked_ioctl = kshram_dev_ioctl,
	.release = kshram_dev_close,
	.mmap = kshram_dev_mmap
};

static int kshram_proc_read(struct seq_file *m, void *v) {
	char buf[100];
	memset(buf, 0, sizeof buf);
	for(size_t i = 0; i < NUMBER_OF_DEVICE; i++){
		char tmp[20];
		snprintf(tmp, sizeof tmp, "0%lu: %lu\n", i, kshram_devices[i].size);
		strncat(buf, tmp, sizeof(buf) - strlen(buf) - 1);
	}
	seq_printf(m, buf);
	return 0;
}

static int kshram_proc_open(struct inode *inode, struct file *file) {
	return single_open(file, kshram_proc_read, NULL);
}

static const struct proc_ops kshram_proc_fops = {
	.proc_open = kshram_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static char *kshram_devnode(const struct device *dev, umode_t *mode) {
	if(mode == NULL) return NULL;
	*mode = 0666;
	return NULL;
}


static int __init kshram_init(void)
{
	for (size_t i = 0; i < NUMBER_OF_DEVICE; i++) {
		kshram_devices[i].mem = kzalloc(4096, GFP_KERNEL);
		kshram_devices[i].size = 4096;
		mutex_init(&kshram_devices[i].lock);
	}

	// create char dev
	if(alloc_chrdev_region(&devnum, 0, NUMBER_OF_DEVICE, "updev") < 0)
		return -1;
	if((clazz = class_create(THIS_MODULE, "upclass")) == NULL)
		goto release_region;
	clazz->devnode = kshram_devnode;
	for(size_t i = 0; i < NUMBER_OF_DEVICE; i++){
		char device_name[10];
		snprintf(device_name, sizeof(device_name), "kshram%lu", i);

		printk(KERN_INFO "kshram%lu: %lu bytes allocated @ %llx\n", i, kshram_devices[i].size, (unsigned long long)kshram_devices[i].mem);
		if(device_create(clazz, NULL, devnum + i, NULL, device_name) == NULL)
			goto release_class;
	}
	cdev_init(&c_dev, &kshram_dev_fops);
	if(cdev_add(&c_dev, devnum, NUMBER_OF_DEVICE) == -1)
		goto release_device;

	// create proc
	proc_create("kshram", 0, NULL, &kshram_proc_fops);

	printk(KERN_INFO "kshram: initialized.\n");
	return 0;    // Non-zero return means that the module couldn't be loaded.

release_device:
	device_destroy(clazz, devnum);
release_class:
	class_destroy(clazz);
release_region:
	unregister_chrdev_region(devnum, 1);
	return -1;
}

static void __exit kshram_cleanup(void)
{
	remove_proc_entry("kshram", NULL);

	cdev_del(&c_dev);
	device_destroy(clazz, devnum);

	// Destroy all the devices created during module initialization
    for (size_t i = 0; i < NUMBER_OF_DEVICE; i++) {
        device_destroy(clazz, devnum + i);
    }

	for (size_t i = 0; i < NUMBER_OF_DEVICE; i++) {
		kfree(kshram_devices[i].mem);
	}

	class_destroy(clazz);
	unregister_chrdev_region(devnum, 8);

	printk(KERN_INFO "kshram: cleaned up.\n");
}

module_init(kshram_init);
module_exit(kshram_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cheng Han Lee");
MODULE_DESCRIPTION("The unix programming Lab05 kernel module.");
