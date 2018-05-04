#include "mysync.h"‪

struct event global_event;
rwlock_t eventID_list_lock;
bool event_initialized;

struct event *get_event(int eventID){
  struct event *pos;

  list_for_each_entry(pos, &global_event.eventID_list, eventID_list){
    if(pos->eventID == eventID){
      return pos;
    }
  }
  return (struct event *) NULL;
}

void initiate_global(void){
  eventID_list_lock = RW_LOCK_UNLOCKED;
  INIT_LIST_HEAD(&global_event.eventID_list);
  global_event.eventID = 0;
  init_waitqueue_head(&global_event.waitQ);
  event_initialized = true;
}

asmlinkage int sys_mysync_make_event(void) {
    unsigned long flags;
    struct event *new_event;
    int maxID;

    new_event = kmalloc(sizeof(struct event), GFP_KERNEL);
    INIT_LIST_HEAD(&(new_event->eventID_list));
    write_lock_irqsave(&eventID_list_lock, flags);
    list_add_tail(&(new_event->eventID_list), &global_event.eventID_list);
    maxID = list_entry((new_event->eventID_list).prev, struct event, eventID_list)->eventID;
    new_event->eventID = maxID + 1;
    init_waitqueue_head(&(new_event->waitQ));
    write_unlock_irqrestore(&eventID_list_lock, flags);
    return new_event->eventID;
}

asmlinkage int sys_mysync_destroy_event(int event_id) {
  unsigned long flags;
  struct event * this_event;
  int result;

  if(eventID == 0)
    return -1;
  read_lock_irqsave(&eventID_list_lock, flags);
  this_event = get_event(eventID);
  read_unlock_irqrestore(&eventID_list_lock, flags);
  if(this_event == NULL)
    return -1;
  result = sys_mysync_sig_event(eventID);
  write_lock_irqsave(&eventID_list_lock, flags);
  list_del(&(this_event->eventID_list));
  write_unlock_irqrestore(&eventID_list_lock, flags);
  kfree(this_event);
  return result;
}

asmlinkage int sys_mysync_wait_event(int event_id) {
  unsigned long flags;
  struct event *this_event;
  int x;

  if(eventID == 0)
    return -1;
  read_lock_irqsave(&eventID_list_lock, flags);
  this_event = get_event(eventID);
  x = this_event->go_aheads;
  read_unlock_irqrestore(&eventID_list_lock, flags);
  // The go_aheads prevents this from going to sleep
  // if the process has just been signalled!
  while(x == this_event->go_aheads)
    interruptible_sleep_on(&(this_event->waitQ));
  return 1;
}

asmlinkage int sys_mysync_sig_event(int event_id) {
  unsigned long flags;
  struct event * this_event;
  int result;

  if(eventID == 0)
    return -1;
  write_lock_irqsave(&eventID_list_lock, flags);
  this_event = get_event(eventID);
  if(this_event == NULL)
    return -1;
  // Right here, the event could be deleted by another process.
  // But not with this handy write lock!
  this_event->go_aheads++;
  // Same as above.
  result = kernel_list_length(&(this_event->waitQ.task_list));
  wake_up_interruptible(&(this_event->waitQ));
  write_unlock_irqrestore(&eventID_list_lock, flags);
  return result;
}
