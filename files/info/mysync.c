#include "mysync.h"‪

struct mysync global_event;
DEFINE_RWLOCK(mysync_lock);
bool mysync_initialized;

int list_length(struct list_head * head){
  int result = 0;
  struct list_head *pos;
  list_for_each(pos, head)
    result++;
  return result;
}

struct mysync *get_mysync(int event_ID){
  struct mysync *pos;

  list_for_each_entry(pos, &global_event.event_ID_list, event_ID_list){
    if(pos->event_ID == event_ID){
      return pos;
    }
  }
  return (struct mysync *) NULL;
}

void initiate_global(void){
  INIT_LIST_HEAD(&global_event.event_ID_list);
  global_event.event_ID = 0;
  init_waitqueue_head(&global_event.wait_queue);
  mysync_initialized = true;
}

asmlinkage int sys_mysync_make_event(void) {
    unsigned long flags;
    struct mysync *new_mysync;
    int maxID;

    new_mysync = kmalloc(sizeof(struct mysync), GFP_KERNEL);
    INIT_LIST_HEAD(&(new_mysync->event_ID_list));
    
    write_lock_irqsave(&mysync_lock, flags);
    list_add_tail(&(new_mysync->event_ID_list), &global_event.event_ID_list);
    maxID = list_entry((new_mysync->event_ID_list).prev, struct mysync, event_ID_list)->event_ID;
    new_mysync->event_ID = maxID + 1;
    init_waitqueue_head(&(new_mysync->wait_queue));
    write_unlock_irqrestore(&mysync_lock, flags);
    
    return new_mysync->event_ID;
}

asmlinkage int sys_mysync_destroy_event(int event_ID) {
  unsigned long flags;
  struct mysync *event;
  int result;

  if(event_ID == 0)
    return -1;

  read_lock_irqsave(&mysync_lock, flags);
  event = get_mysync(event_ID);
  read_unlock_irqrestore(&mysync_lock, flags);

  if(event == NULL)
    return -1;
  result = sys_mysync_sig_event(event_ID);

  write_lock_irqsave(&mysync_lock, flags);
  list_del(&(event->event_ID_list));
  write_unlock_irqrestore(&mysync_lock, flags);

  kfree(e);
  return result;
}

asmlinkage int sys_mysync_wait_event(int event_ID) {
  unsigned long flags;
  struct mysync *event;
  int x;

  if(event_ID == 0)
    return -1;

  read_lock_irqsave(&mysync_lock, flags);
  event = get_mysync(event_ID);
  x = event->go_aheads;
  read_unlock_irqrestore(&mysync_lock, flags);
  
  wait_event_interruptible(event->wait_queue, x != event->go_aheads);
  return 1;
}

asmlinkage int sys_mysync_sig_event(int event_ID) {
  unsigned long flags;
  struct mysync *event;
  int result;

  if(event_ID == 0)
    return -1;

  write_lock_irqsave(&mysync_lock, flags);
  event = get_mysync(event_ID);
  if(event == NULL)
    return -1;
    
  event->go_aheads++;
  result = list_length(&(event->wait_queue.task_list));
  wake_up_interruptible(&(event->wait_queue));
  write_unlock_irqrestore(&mysync_lock, flags);

  return result;
}
