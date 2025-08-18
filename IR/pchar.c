#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h> // for copy_to_user, copy_from_user

#define DEV_NAME    "pchar"
#define CLASS_NAME  "pchar_class"
#define SIZE        32

static char buffer[SIZE];
static dev_t devno;
static struct class *pclass;
static struct cdev pchar_cdev;

/* Function Prototypes */
static int pchar_open(struct inode *inode, struct file *file);
static int pchar_release(struct inode *inode, struct file *file);
static ssize_t pchar_read(struct file *file, char __user *ubuf, size_t count, loff_t *offset);
static ssize_t pchar_write(struct file *file, const char __user *ubuf, size_t count, loff_t *offset);

/* File Operations Structure */
static struct file_operations pchar_fops = 
{
    .owner = THIS_MODULE,
    .open = pchar_open,
    .release = pchar_release,
    .read = pchar_read,
    .write = pchar_write,
};

/* Open - just a stub */
static int pchar_open(struct inode *inode, struct file *file) 
{
    pr_info("pchar: open()\n");
    return 0;
}

/* Release - just a stub */
static int pchar_release(struct inode *inode, struct file *file) 
{
    pr_info("pchar: release()\n");
    return 0;
}

/* Read */
static ssize_t pchar_read(struct file *file, char __user *ubuf, size_t count, loff_t *offset) 
{
    ssize_t bytes_to_read, bytes_read;
    pr_info("pchar: read() at offset %lld\n", *offset);

    if (*offset >= SIZE)
        return 0; // End of buffer

    bytes_to_read = min(count, (size_t)(SIZE - *offset));
    if (bytes_to_read == 0)
        return 0;

    bytes_read = bytes_to_read - copy_to_user(ubuf, buffer + *offset, bytes_to_read);
    *offset += bytes_read;

    pr_info("pchar: read() %zd bytes at offset %lld\n", bytes_read, *offset - bytes_read);
    return bytes_read;
}

/* Write */
static ssize_t pchar_write(struct file *file, const char __user *ubuf, size_t count, loff_t *offset) 
{
    ssize_t bytes_to_write, bytes_written;
    pr_info("pchar: write() at offset %lld\n", *offset);

    if (*offset >= SIZE)
        return -ENOSPC; // No space left

    bytes_to_write = min(count, (size_t)(SIZE - *offset));
    if (bytes_to_write == 0)
        return -ENOSPC;

    bytes_written = bytes_to_write - copy_from_user(buffer + *offset, ubuf, bytes_to_write);
    *offset += bytes_written;

    pr_info("pchar: write() %zd bytes at offset %lld\n", bytes_written, *offset - bytes_written);
    return bytes_written;
}

/* Module Init */
static int __init pchar_init(void) 
{
    int ret;
    struct device *pdev;

    pr_info("pchar: loading\n");

    ret = alloc_chrdev_region(&devno, 0, 1, DEV_NAME);
    if (ret < 0)
        return ret;

    cdev_init(&pchar_cdev, &pchar_fops);
    pchar_cdev.owner = THIS_MODULE;

    ret = cdev_add(&pchar_cdev, devno, 1);
    if (ret)
        goto fail_cdev_add;

    pclass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(pclass)) {
        ret = PTR_ERR(pclass);
        goto fail_class_create;
    }

    pdev = device_create(pclass, NULL, devno, NULL, DEV_NAME);
    if (IS_ERR(pdev)) {
        ret = PTR_ERR(pdev);
        goto fail_device_create;
    }

    pr_info("pchar: module loaded. Major: %d Minor: %d\n", MAJOR(devno), MINOR(devno));
    return 0;

fail_device_create:
    class_destroy(pclass);
fail_class_create:
    cdev_del(&pchar_cdev);
fail_cdev_add:
    unregister_chrdev_region(devno, 1);
    return ret;
}

/* Module Exit */
static void __exit pchar_exit(void) 
{
    device_destroy(pclass, devno);
    class_destroy(pclass);
    cdev_del(&pchar_cdev);
    unregister_chrdev_region(devno, 1);
    pr_info("pchar: unloaded\n");
}

module_init(pchar_init);
module_exit(pchar_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SSK");
MODULE_DESCRIPTION("IR LM393 Module ");
