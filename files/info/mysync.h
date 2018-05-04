#ifndef _PROCESS_SORT_H
#define _PROCESS_SORT_H

#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/pid_namespace.h>
#include <linux/notifier.h>
#include <linux/thread_info.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/posix-timers.h>
#include <linux/cpu.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/tick.h>
#include <linux/kallsyms.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/div64.h>
#include <asm/timex.h>
#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/spinlock.h>

struct event *get_event(int eventID);
void initiate_global(void);

asmlinkage int sys_mysync_make_event(void);
asmlinkage int sys_mysync_destroy_event(int event_id);
asmlinkage int sys_mysync_wait_event(int event_id);
asmlinkage int sys_mysync_sig_event(int event_id);

struct event {
  int eventID;
  struct list_head eventID_list;
  wait_queue_head_t waitQ;
  int go_aheads;
};

extern rwlock_t eventID_list_lock;
extern struct event global_event;
extern bool event_initialized;

#endif
