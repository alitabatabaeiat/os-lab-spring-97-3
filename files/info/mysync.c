#include "mysync.h"‪

struct idr id_event_map;
DEFINE_RWLOCK(mysync_lock);
bool mysync_initialized;

void initiate_global(void){
  idr_init(&id_event_map);
  mysync_initialized = true;
}
int list_length(struct list_head * lh){
  int result = 0;
  struct list_head * pos;
  list_for_each(pos, lh) {
    result++;
  }
  return result;
}
asmlinkage int sys_mysync_make_event(void) {
  unsigned long flags;
  struct mysync *new_mysync;
  int ret;

  new_mysync = kmalloc(sizeof(struct mysync), GFP_KERNEL);
    
  write_lock_irqsave(&mysync_lock, flags);

    ret = idr_alloc(&id_event_map, new_mysync, 1,INT_MAX,GFP_KERNEL);
    if (ret == -ENOMEM || ret == -ENOSPC) {
      write_unlock_irqrestore(&mysync_lock, flags);
      kfree(new_mysync);
      return -1;
    }
    new_mysync->event_id=ret;
    init_waitqueue_head(&(new_mysync->wait_queue));
  
  write_unlock_irqrestore(&mysync_lock, flags);
    
  return new_mysync->event_id;
}

asmlinkage int sys_mysync_destroy_event(int event_id) {
  unsigned long flags;
  struct mysync *event;
  int result;

  read_lock_irqsave(&mysync_lock, flags);
    event = (struct mysync *)idr_find(&id_event_map,event_id);
  read_unlock_irqrestore(&mysync_lock, flags);

  if(event == NULL)
    return -1;

  result = sys_mysync_sig_event(event_id);

  write_lock_irqsave(&mysync_lock, flags);
    idr_remove(&id_event_map,event_id);
  write_unlock_irqrestore(&mysync_lock, flags);

  kfree(event);
  return result;
}

asmlinkage int sys_mysync_wait_event(int event_id) {
  unsigned long flags;
  struct mysync *event;
  int x;

  read_lock_irqsave(&mysync_lock, flags);
    event = (struct mysync *)idr_find(&id_event_map,event_id);
    if(event == NULL){
      read_unlock_irqrestore(&mysync_lock, flags);
      return -1;
    }
    x = event->go_aheads;
  read_unlock_irqrestore(&mysync_lock, flags);
      
  wait_event_interruptible(event->wait_queue, x != event->go_aheads);
  return 1;
}

asmlinkage int sys_mysync_sig_event(int event_id) {
  unsigned long flags;
  struct mysync *event;
  int result;

  write_lock_irqsave(&mysync_lock, flags);
    
    event = (struct mysync *)idr_find(&id_event_map,event_id);
    if(event == NULL){
      write_unlock_irqrestore(&mysync_lock, flags);
      return -1;
    }  
    event->go_aheads++;
    result = list_length(&(event->wait_queue.task_list));
    wake_up_interruptible(&(event->wait_queue));

  write_unlock_irqrestore(&mysync_lock, flags);

  return result;
}
