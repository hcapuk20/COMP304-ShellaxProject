#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/slab.h>


// Meta Information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("A module that knows how to greet");

//char name;
int given_pid;

/*
* module_param(foo, int, 0000)
* The first param is the parameters name
* The second param is it's data type
* The final argument is the permissions bits,
* for exposing parameters in sysfs (if non-zero) at a later stage.
*/

//module_param(name, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
//MODULE_PARM_DESC(name, "name of the caller");

module_param(given_pid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(given_pid, "pid of root");


void build_tree(struct task_struct *task, int p_pid) {
    struct task_struct *child;
    struct list_head *task_list;
    int first = 1;
    if (p_pid != 0){
        printk("\"%d, %lld\"->\"%d, %lld\"\n", p_pid,task->parent->start_time, task->pid,task->start_time);
    }
    list_for_each(task_list, &task->children) {
        child = list_entry(task_list, struct task_struct, sibling);
        if (first == 1){
            printk("\"%d, %lld\" [color=\"green\"]",child->pid,child->start_time);
            first = 0;
        }
        build_tree(child, task->pid);
    }
}


// A function that runs when the module is first loaded
int simple_init(void) {
    struct task_struct *ts;

    ts = get_pid_task(find_get_pid(given_pid), PIDTYPE_PID);

    build_tree(ts, 0);
    return 0;
}

// A function that runs when the module is removed
void simple_exit(void) {

}

module_init(simple_init);
module_exit(simple_exit);