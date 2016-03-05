#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
  
  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(proc->killed)
      exit();
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpu->id == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();

    //check if an alarm is set, increase ticks if it is, 

    if (proc && proc->alarmset > 0)
    {
      proc->alarmticks += 1;

      if (proc->alarmticks > proc->alarmreqticks)   //enough time has passed to run the handler
      {
        //reset tick things for future alarm calls
        proc->alarmticks = 0;
        proc->alarmreqticks = 0;
        proc->alarmset = 0;

        //things we need to do for stage2:
        //    push proc->tf->eax/ecx/edx onto the stack
        //    build our call to the handler as we did before
        //    add our popregs asm function to be called just AFTER the handler to restore the registers -> make this location the return address of the handler?
        //    function to pop registers is stored at proc->procfunc, stored here in our call to register_signal_handler so the user never sees it
        //    flow should be: 1) push register values 2) run handler, which returns to popfunction? 3) run function which restores the registers, which in turn calls ret which sets the ip next on the stack (the bad instruction)
        *(int*) (proc->tf->esp+4) = proc->tf->eip;  
        proc->tf->esp += 4;
        proc->tf->eip = (uint) proc->handlers[SIGALRM];

        //cprintf("stage2 setup done, current esp is: %p\n", proc->tf->esp);

        //push registers below return address and argument for handler
        *(int*) (proc->tf->esp+4) = proc->tf->edx;
        *(int*) (proc->tf->esp+8) = proc->tf->ecx;
        *(int*) (proc->tf->esp+12) = proc->tf->eax;

        cprintf("val at ecx %d\n", proc->tf->ecx);

        //create frame for our handler function, with its return address being the asm function to pop registers -> think this is wrong, popfunc should be below the things for handler???
        proc->tf->esp += 12;  
        *(int*) (proc->tf->esp) = (uint) proc->popfunc;
        siginfo_t *info = (siginfo_t*) (proc->tf->esp + 4);
        info->signum = SIGALRM;
      }
    }
    

    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpu->id, tf->cs, tf->eip);
    lapiceoi();
    break;
   
  //PAGEBREAK: 13
  default:
    if(proc == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpu->id, tf->eip, rcr2());
      panic("trap");
    }
    else if (tf->trapno == T_DIVIDE){

      //dividing by zero -> trigger SIGFPE handler or kill the process if no handler is set

     if (proc->handlers[SIGFPE] == (sighandler_t*) 1)
     {
      cprintf("No handler assigned for SIGFPE, exiting. Current pid is %d\n", proc->pid);
      proc->killed = 1;
     }
     else
     {
      //things we need to do for stage2:
      //    push proc->tf->eax/ecx/edx onto the stack
      //    build our call to the handler as we did before
      //    add our popregs asm function to be called just AFTER the handler to restore the registers -> make this location the return address of the handler?
      //    function to pop registers is stored at proc->procfunc, stored here in our call to register_signal_handler so the user never sees it
      //    flow should be: 1) push register values 2) run handler, which returns to popfunction? 3) run function which restores the registers, which in turn calls ret which sets the ip next on the stack (the bad instruction)
      
      //works for stage1
      cprintf("ebp: %p, esp: %p\n", proc->tf->ebp, proc->tf->esp);
      
      *(int*) (proc->tf->esp-4) = proc->tf->eip;
      proc->tf->esp -= 4;
      proc->tf->eip = (uint) proc->handlers[SIGFPE];

      //current stack pointer + 4 = first argument of the handler (the siginfo struct)
      cprintf("proc->tf->ebp: %d, proc->tf->esp: %d\n", proc->tf->ebp, proc->tf->esp);

      siginfo_t *info = (siginfo_t*) (proc->tf->esp + 4);
      info->signum = SIGFPE;
      

     }
    }
    // In user space, assume process misbehaved.
    else {
      cprintf("pid %d %s: trap %d err %d on cpu %d "
              "eip 0x%x addr 0x%x--kill proc\n",
              proc->pid, proc->name, tf->trapno, tf->err, cpu->id, tf->eip, 
              rcr2());
      proc->killed = 1;
    }
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running 
  // until it gets to the regular system call return.)
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
  	 exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
   	 yield();

  // Check if the process has been killed since we yielded
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
   	 exit();
}
