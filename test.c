#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAKE 545
#define DESTROY 546
#define WAIT 547
#define SIG 548

void testA(void);
void testB(void);
void testC(void);

int main(int argc, char **argv){

  printf("\n******************************\n");
  printf("Test A - No process waiting while signaled.\n");
  testA();

  printf("\n******************************\n");
  printf("Test B - Process waiting when signal is called.\n");
  testB();

  printf("\n******************************\n");
  printf("Test C - Processes waiting when destroy is called\n");
  testC();

	return 0;
}

/* Test A - No process waiting while signaled. */
void testA() {
	int eid;
	int ret;

	eid = syscall(MAKE);
	if(eid == -1){
		printf("Error in creating event\n");
		return;
	}
	printf("Create event #%d. \n", eid);

  ret = syscall(SIG, eid);
  printf("%d processes have been signaled.\n", ret);
  if (ret != -1)
    printf("Correct\n");
  else
    printf("Incorrect\n");

  ret = syscall(DESTROY, eid);
  if (ret != -1)
    printf("Event #%d is Destroyed.\n", eid);
  else
    printf("Event #%d is not Destroyed.\n", eid);

  return;
}

/* Test B - Process waiting when signal is called. */
void testB(){
  int pid;
  int eid;
  int ret_child, ret_parent;

  eid = syscall(MAKE);
	if(eid == -1) {
  	printf("Error in creating event.\n");
  	return;
	}
	printf("Event #%d is created.\n", eid);

  pid = fork();
  if (pid == -1) {
    printf("Error in fork.\n");
  } else if (pid == 0) {
    ret_child = syscall(WAIT, eid);
    if(ret_child == -1)
      printf("Error in wait.\n");
    else
      printf("New process waiting on event #%d. \n", eid);
    exit(0); // important!! Otherwise, the children will implement all the rest codes.
   }
   else {
    sleep(1);
    ret_parent = syscall(SIG, eid);
    if(ret_parent == -1){
    	printf("Error in signal. \n");
      return;
    }
    printf("Unblock %d process.\n", ret_parent);
    ret_parent = syscall(DESTROY, eid);
    if (ret_parent != -1)
      printf("Event #%d is Destroyed.\n", eid);
    else
      printf("Event #%d is not Destroyed.\n", eid);
	}

}

/* Test C - Processes waiting when destroy is called */
void testC() {
  int pid;
  int eid1, eid2;
  int ret_parent, ret_eid1, ret_eid2;

  eid1 = syscall(MAKE);
  eid2 = syscall(MAKE);
  if(eid1 == -1) {
    printf("Error in creating event 1.\n");
    return;
  }
  printf("Event #%d is created.\n", eid1);

  if(eid2 == -1) {
    printf("Error in creating event 1.\n");
    return;
  }
  printf("Event #%d is created.\n", eid2);

  pid = fork();
  if (pid == -1) {
   printf("Error in fork.\n");
  } else if (pid == 0) {
    ret_eid1 = syscall(WAIT, eid1);
    if(ret_eid1 == -1)
      printf("Error in wait.\n");
    else
      printf("New process waiting on event #%d. \n", eid1);
    exit(0);
  } else {
    pid = fork();
    if (pid == -1) {
      printf("Error in fork.\n");
    } if (pid == 0) {
      ret_eid2 = syscall(WAIT, eid2);
      if(ret_eid2 == -1)
        printf("Error in wait.\n");
      else
        printf("New process waiting on event #%d. \n", eid2);
      exit(0);
    } else { // main process
      sleep(1);
      ret_parent = syscall(DESTROY, eid1);
      if (ret_parent != -1)
        printf("Event #%d is Destroyed.\n", eid1);
      else
        printf("Event #%d is not Destroyed.\n", eid1);

      ret_parent = syscall(DESTROY, eid2);
      if (ret_parent != -1)
        printf("Event #%d is Destroyed.\n", eid2);
      else
        printf("Event #%d is not Destroyed.\n", eid2);
    }
  }
}
