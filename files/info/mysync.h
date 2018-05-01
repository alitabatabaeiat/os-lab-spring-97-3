#ifndef _PROCESS_SORT_H
#define _PROCESS_SORT_H

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/fdtable.h>
#include <linux/list_sort.h>

asmlinkage long sys_mysync_make_event();
asmlinkage long sys_mysync_destroy_event(int event_id);
asmlinkage long sys_mysync_wait_event(int event_id);
asmlinkage long sys_mysync_sig_event(int event_id);
#endif
