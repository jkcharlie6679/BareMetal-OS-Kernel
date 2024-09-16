#include "stdint.h"

#define THREAD_SP_SIZE 2048

enum state{WAITTING, RUNNING, EXIT};
enum mode{KERNEL, USER};

typedef struct task{
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
  uint64_t fp;  // x29
  uint64_t lr;  // x30
  uint64_t sp;
  uint64_t target_func;
  uint64_t user_sp;
  uint64_t sp_addr;
  uint32_t pid;
  enum mode mode;
  void (*handler)();
  enum state state;
  struct task *next;
} task;

typedef void (*thread_func)(void);

extern void *get_current();
extern void switch_to(task *pre, task *next);
extern void write_current(uint64_t x0);

void thread_init(void);
void idle_thread(void);
void *task_create(thread_func func, enum mode mode);
void schedule();
void kill_zombies();
void kill_thread(int pid);
void switch_to_user_space();
void fork_test();
void foo();
void remove_task(uint64_t pid);
void add_to_queue();
void *find_task(uint64_t pid);
