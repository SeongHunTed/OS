#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

#define TIME_SLICE 10000000       // time slice
#define NULL ((void *)0)          // null

int weight = 1;                   // process 가중치

// process들을 관리하는 process table
// 스케쥴링을 위한 long long type의 min_priority
// 여러개의 process들 중에서 가장 작은 우선순위를 가지는 값을 저장한다.
// 해당 min_priority는 차후 Process가 새로 생기거나 wake up 될 때, 우선순위를 독점하지 않기위해
// ptable의 min_priority 값을 자신의 priority 값을 갖게된다.
struct {
  struct spinlock lock;
  struct proc proc[NPROC];
  long long min_priority;         // ptable에 있는 process중 가장 작은 prioirty
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

// 새로 생긴 프로세스가 제외되지 않도록
// 큰 가중치를 부여하는 스케쥴러
// 해당 함수는 schedule()함수에서 호출된다
// 해당 함수가 가장 우선적으로 수행되어야할 프로세스를 선택하여 return한다
// return한 프로세스를 받아 schedule()에서 향후 context switching 및 실행
struct proc *ssu_schedule(void)
{
  struct proc *p;               // runnable한 process 중 하나
  struct proc *ret = NULL;      // return 해줄 process

  // NPROC : Max값이 64, 즉 xv6에서 수행할 수 있는 프로세스는 한번에 64개이다.
  // ptable에 존재하는 프로세스를 순회하면서 RUNNABLE한 상태의 process를 찾는다
  // RUNNABLE한 process들을 순회하고 비교하면서 가장 낮은 priority 값을 가진 프로세스 갱신
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if (p->state == RUNNABLE) {
      if (ret == NULL || (ret->priority > p->priority)){
        ret = p;
      }
    }
  }

// 3번과제를 위한 DEBUG 옵션
// DEBUG 수행시 PID, NAME, WEIGHT, PRIORITY 값 print
#ifdef DEBUG
  if(ret)
    cprintf("PID: %d, NAME: %s, WEIGHT: %d, PRIORITY: %d\n", ret->pid, ret->name, ret->weight, ret->priority);

#endif
  return ret;   // priority 값이 가장 낮은 프로세스 return
}

// scheduler() 함수가 호출될 때 마다 수행
// 1tick 동안 수행을 마친 프로세스의 priority를 업데이트 한다.
void update_priority(struct proc *proc)
{
  // process의 priority 값을 갱신함
  // 가장 최근에 생성된 프로세스일 수록 가중치가 높다 -> priority 값이 작아진다
  // -> 스케쥴러에게 선택될 우선순위가 높아진다
  proc->priority = proc->priority + (TIME_SLICE/proc->weight);
}

// update_prioirty()와 마찬가지로 스케쥴러 함수가 호출될 때 마다 함께 호출됨
// 1tick 동안 수행을 마치고 나서 프로세스 우선순위가 업데이트가 됨
// 프로세스 우선순위가 업데이트 되었음으로, 가장 작은 priority 값도 함께 업데이트 되어야함
void update_min_priority()
{
  struct proc *min = NULL;      // priority 값이 가장 낮은 process
  struct proc *p;               // ptable 순회하는 process
  
  // ptable을 순회하며 RUNNABLE 한 것 중 가장 작은 것을 갱신함
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if(p->state == RUNNABLE) {
      if (min == NULL || (min->priority > p->priority))
        min = p;
    }
  }

  // priority 값이 가장 작은 process의 priority를 ptable의 min_priority에 할당
  if(min != NULL)
    ptable.min_priority = min->priority;
}

// process가 새롭게 생성되거나 Ready에서 wake되는 상태에서 사용
// priority를 0부터 부여하게 되면, 프로세스 독점 실행될 수 있는 문제가 있음
// 따라서 입력으로 들어온 process의 priority 값을 ptable의 min_priority로 부여
void assign_min_priority(struct proc *proc)
{
  // process의 우선순위를 ptable의 가장 작은 우선 순위로 부여
  proc->priority = ptable.min_priority;
}


void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  
  // 전역변수의 weight 가중치를 process 가중치에 할당
  // process가 생성될 때 마다 weight 값 증가
  // weight값이 증가하면 priority 값이 작아짐 -> 즉 스케쥴러에 의해 빠르게 선택됨
  p->weight = weight;
  weight++;

  // ptable에 process들의 priority 중 가장 작은 priority를 부여해준다
  assign_min_priority(p);

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  // initcode, init, sh은 xv6가 부팅되며 자동으로 생성되는 프로세스
  // 따라서 최소 priority 값을 3으로 처음에 설정해줌
  ptable.min_priority = 3;

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.

// main.c에서 호출한 해당 스케쥴러는 return값이 없다
// 따라서 loop하며 계속해서 스케쥴링을 수행한다.
void
scheduler(void)
{
  // ssu_schudule()가 가져오는 process를 cpu의 프로세스에 할당
  // 스케쥴러가 호출되면 cpu의 process를 비워준다.
  struct proc *p;             // 수행 되어야 할 프로세스
  struct cpu *c = mycpu();    // cpu 구조체
  c->proc = 0;                // cpu 구조체의 프로세스를 비워줌

  for(;;){
    // Enable interrupts on this processor.
    sti();
    acquire(&ptable.lock);      // context switcing의 시작, interrupt disable
                                // 해당 영역부터 release 전까지 interrupt가 불가능하며, 이를 critical area라 칭함
    
    p = ssu_schedule();         // ssu_schdule로 가장 priority 값이 작은 프로세스 선택
    if(p == NULL){              // 선택된 프로세스가 없는 경우
      release(&ptable.lock);    // lock 해제
      continue;
    }

    c->proc = p;                // cpu -> proc을 선택된 가장 우선순위가 높은 프로세스로 교체

    switchuvm(p);               // p로 교체
    p->state = RUNNING;         // 해당 프로세스 state RUNNING

    swtch(&(c->scheduler), p->context); // context switching
    switchkvm();

    update_priority(p);         // 실행 후 해당 프로세스의 priority값 업데이트
    update_min_priority();      // 실행 후 해당 ptable의 min_priority값 업데이트

    // Process is done running for now.
    // It should have changed its p->state before coming back.
    c->proc = 0;

    release(&ptable.lock);      // context switcing의 끝, lock 해제 부분

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan){
      p->state = RUNNABLE;
      // wake up된 프로세스의 우선 순위를 min_priority로 부여
      // 위에서 설명하였듯, 독점적인 프로세스를 방지하기 위하여
      assign_min_priority(p); 
    }

}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

void do_weightset(int weight)
{
  acquire(&ptable.lock);          // lock수행, interrupt disable
  myproc()->weight = weight;      // 현재 프로세스 가중치를 전달받은 값으로 설정
  release(&ptable.lock);          // lock해제, interrupt enable
}
