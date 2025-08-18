#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h> // For copy_to_user, copy_from_user
#include <linux/gpio.h>    // For GPIO operations

// Define the GPIO pin connected to the IR sensor.
// IMPORTANT: Replace with your actual GPIO pin number (e.g., 17 for GPIO17 on Raspberry Pi)
#define IR_SENSOR_GPIO_PIN 17 // Example GPIO pin. ***CHANGE THIS TO YOUR ACTUAL PIN***

// Pseudo device buffer (for simple read/write, though IR sensor is mostly read)
#define BUFFER_SIZE 1
static char ir_sensor_buffer[BUFFER_SIZE]; // To hold '1' or '0' for sensor state

// Function declarations for file operations
static int ir_sensor_open(struct inode *pinode, struct file *pfile);
static int ir_sensor_close(struct inode *pinode, struct file *pfile);
static ssize_t ir_sensor_read(struct file *pfile, char __user *ubuf, size_t bufsize, loff_t *poffset);
static ssize_t ir_sensor_write(struct file *pfile, const char __user *ubuf, size_t bufsize, loff_t *poffset);

// Global variables for device registration
static dev_t ir_sensor_devno;
static struct class *ir_sensor_class;
static struct file_operations ir_sensor_ops = {
    .owner = THIS_MODULE,
    .open = ir_sensor_open,
    .release = ir_sensor_close,
    .read = ir_sensor_read,
    .write = ir_sensor_write // Write operation typically not used for simple IR sensors
};
static struct cdev ir_sensor_cdev;

// Module initialization function
static int __init ir_sensor_init(void) {
    int ret;
    struct device *pdevice;
    pr_info("%s: ir_sensor_init() called.\n", THIS_MODULE->name);

    // 1. Allocate device number
    ret = alloc_chrdev_region(&ir_sensor_devno, 0, 1, "ir_sensor");
    if(ret < 0) {
        pr_err("%s: alloc_chrdev_region() failed.\n", THIS_MODULE->name);
        return ret;
    }
    pr_info("%s: device number = %d/%d.\n", THIS_MODULE->name, MAJOR(ir_sensor_devno), MINOR(ir_sensor_devno));

    // 2. Create device class
    ir_sensor_class = class_create(THIS_MODULE, "ir_sensor_class");
    if(IS_ERR(ir_sensor_class)) {
        pr_err("%s: class_create() failed.\n", THIS_MODULE->name);
        unregister_chrdev_region(ir_sensor_devno, 1);
        return PTR_ERR(ir_sensor_class); // Return error from IS_ERR
    }
    pr_info("%s: device class is created.\n", THIS_MODULE->name);

    // 3. Create device file
    pdevice = device_create(ir_sensor_class, NULL, ir_sensor_devno, NULL, "ir_sensor");
    if(IS_ERR(pdevice)) {
        pr_err("%s: device_create() failed.\n", THIS_MODULE->name);
        class_destroy(ir_sensor_class);
        unregister_chrdev_region(ir_sensor_devno, 1);
        return PTR_ERR(pdevice); // Return error from IS_ERR
    }
    pr_info("%s: device file '/dev/ir_sensor' is created.\n", THIS_MODULE->name);

    // 4. Initialize cdev struct and add into kernel db
    cdev_init(&ir_sensor_cdev, &ir_sensor_ops);
    ret = cdev_add(&ir_sensor_cdev, ir_sensor_devno, 1);
    if(ret < 0) {
        pr_err("%s: cdev_add() failed.\n", THIS_MODULE->name);
        device_destroy(ir_sensor_class, ir_sensor_devno);
        class_destroy(ir_sensor_class);
        unregister_chrdev_region(ir_sensor_devno, 1);
        return ret;
    }
    pr_info("%s: device cdev is added to kernel.\n", THIS_MODULE->name);

    // 5. Request and configure GPIO pin for IR sensor
    // Check if the GPIO pin is valid
    if (!gpio_is_valid(IR_SENSOR_GPIO_PIN)) {
        pr_err("%s: Invalid GPIO pin %d\n", THIS_MODULE->name, IR_SENSOR_GPIO_PIN);
        cdev_del(&ir_sensor_cdev);
        device_destroy(ir_sensor_class, ir_sensor_devno);
        class_destroy(ir_sensor_class);
        unregister_chrdev_region(ir_sensor_devno, 1);
        return -EINVAL; // Invalid argument
    }

    // Request the GPIO pin
    ret = gpio_request(IR_SENSOR_GPIO_PIN, "ir_sensor_gpio");
    if (ret) {
        pr_err("%s: Failed to request GPIO %d\n", THIS_MODULE->name, IR_SENSOR_GPIO_PIN);
        cdev_del(&ir_sensor_cdev);
        device_destroy(ir_sensor_class, ir_sensor_devno);
        class_destroy(ir_sensor_class);
        unregister_chrdev_region(ir_sensor_devno, 1);
        return ret;
    }

    // Set GPIO direction to input
    gpio_direction_input(IR_SENSOR_GPIO_PIN);
    pr_info("%s: GPIO %d configured as input for IR sensor.\n", THIS_MODULE->name, IR_SENSOR_GPIO_PIN);

    return 0; // Module initialized successfully.
}

// Module de-initialization function
static void __exit ir_sensor_exit(void) {
    pr_info("%s: ir_sensor_exit() called.\n", THIS_MODULE->name);

    // Free the GPIO pin
    gpio_free(IR_SENSOR_GPIO_PIN);
    pr_info("%s: GPIO %d freed.\n", THIS_MODULE->name, IR_SENSOR_GPIO_PIN);

    // Remove device cdev from the kernel db.
    cdev_del(&ir_sensor_cdev);
    pr_info("%s: device cdev is removed from kernel.\n", THIS_MODULE->name);

    // Destroy device file
    device_destroy(ir_sensor_class, ir_sensor_devno);
    pr_info("%s: device file is destroyed.\n", THIS_MODULE->name);

    // Destroy device class
    class_destroy(ir_sensor_class);
    pr_info("%s: device class is destroyed.\n", THIS_MODULE->name);

    // Unallocate device number
    unregister_chrdev_region(ir_sensor_devno, 1);
    pr_info("%s: device number released.\n", THIS_MODULE->name);
}

// IR sensor file operations
static int ir_sensor_open(struct inode *pinode, struct file *pfile) {
    pr_info("%s: ir_sensor_open() called.\n", THIS_MODULE->name);
    return 0;
}

static int ir_sensor_close(struct inode *pinode, struct file *pfile) {
    pr_info("%s: ir_sensor_close() called.\n", THIS_MODULE->name);
    return 0;
}

static ssize_t ir_sensor_read(struct file *pfile, char __user *ubuf, size_t bufsize, loff_t *poffset) {
    int sensor_state;
    int bytes_copied;

    pr_info("%s: ir_sensor_read() called.\n", THIS_MODULE->name);

    // Ensure we are reading from the beginning of the "buffer" (or sensor state)
    if (*poffset >= BUFFER_SIZE) {
        pr_info("%s: pchar_read() end of device reached.\n", THIS_MODULE->name);
        return 0; // End of file
    }

    // Read the current state of the IR sensor GPIO pin
    // This assumes a simple digital IR sensor (e.g., 0 for no detection, 1 for detection)
    sensor_state = gpio_get_value(IR_SENSOR_GPIO_PIN);
    pr_info("%s: IR Sensor State: %d\n", THIS_MODULE->name, sensor_state);

    // Store the sensor state as a character ('0' or '1') in our internal buffer
    ir_sensor_buffer[0] = (sensor_state == 0) ? '0' : '1';

    // Copy the sensor state to the user buffer
    // We only copy 1 byte (the state character)
    bytes_copied = BUFFER_SIZE - copy_to_user(ubuf, ir_sensor_buffer + *poffset, BUFFER_SIZE - *poffset);

    if (bytes_copied == 0 && (BUFFER_SIZE - *poffset) > 0) {
        pr_err("%s: Failed to copy data to user space.\n", THIS_MODULE->name);
        return -EFAULT; // Bad address
    }

    // Update the file position
    *poffset += bytes_copied;

    pr_info("%s: ir_sensor_read() - bytes read: %d (state: %c).\n", THIS_MODULE->name, bytes_copied, ir_sensor_buffer[0]);

    // Return the number of bytes successfully read.
    return bytes_copied;
}

static ssize_t ir_sensor_write(struct file *pfile, const char __user *ubuf, size_t bufsize, loff_t *poffset) {
    pr_info("%s: ir_sensor_write() called. (Write operation not supported for simple IR sensor)\n", THIS_MODULE->name);
    // For a simple IR sensor, writing data is generally not applicable.
    // You might return -EPERM (Operation not permitted) or 0 bytes written.
    return -EPERM; // Operation not permitted
}

module_init(ir_sensor_init);
module_exit(ir_sensor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name/Organization"); // Update with your information
MODULE_DESCRIPTION("Conceptual IR Sensor Device Driver");
MODULE_VERSION("0.1");
