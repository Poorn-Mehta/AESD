/*
*	Custom Timer Module by Poorn Mehta
*	Initialized with default values (name and delay)
*	Delay is to be given in milli seconds
*	Refernce Links:
*	http://linuxkernel51.blogspot.com/2011/03/use-of-module-parameters-in-kernel.html
*	https://blog.sourcerer.io/writing-a-simple-linux-kernel-module-d9dc3762c234
*	https://stackoverflow.com/questions/3467850/cross-compiling-a-kernel-module 
*/

// Includes

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>

// Variables with initialized value
struct timer_list custom_timer;
char *user_name = "Null by Default";
int delay_ms = 500;
int counter = 0;

// Module inline parameters with type and description to be shown under modinfo
module_param(user_name, charp, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(user_name, "User Name Parameter - String");
module_param(delay_ms, int, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(delay_ms, "Timing to set the frequency of Callback Functions - integer value representing milliseconds");

// Function that will be called after each timeout
void _Callback_Function(unsigned long data)
{
    mod_timer(&custom_timer, jiffies + msecs_to_jiffies(delay_ms));	//Formula for time calculation, used some online reference
    printk("Kernel Module Counter: %d", counter);	//Simple kernel print message
    counter += 1;	//Counter increment
}

// Timer initialization function
static int __init custom_timer_init(void)
{
    printk("Custom Kernel Module Insertion"); //Simple kernel print messages
    printk("User Name: %s", user_name);
    setup_timer(&custom_timer,_Callback_Function, 0); //Inbuilt functions for setting up timer
    mod_timer(&custom_timer, jiffies + msecs_to_jiffies(delay_ms)); 
    return 0;
}

// Timer ending function (this will delete the timer from kernel)
static void __exit custom_timer_exit(void)
{
    del_timer(&custom_timer);
    printk("Custom Kernel Module Removed");
}

// Modinfo messages
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Poorn Mehta");
MODULE_DESCRIPTION("This is a Custom Timer Kernel Module");

																																																														module_init(custom_timer_init);																																																																module_exit(custom_timer_exit);

