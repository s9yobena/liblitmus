#ifndef LITMUS_H
#define LITMUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stdint.h>
#include <setjmp.h>

/* Include kernel header.
 * This is required for the rt_param
 * and control_page structures.
 */
#include "litmus/rt_param.h"
#include "litmus/signal.h"

#include "asm/cycles.h" /* for null_call() */

typedef int pid_t;	 /* PID of a task */

/* obtain the PID of a thread */
pid_t gettid(void);

/* migrate to partition */
int be_migrate_to(int target_cpu);

int set_rt_task_param(pid_t pid, struct rt_task* param);
int get_rt_task_param(pid_t pid, struct rt_task* param);

/* setup helper */

/* times are given in ms */
int sporadic_task(
		lt_t e, lt_t p, lt_t phase,
		int partition, task_class_t cls,
		budget_policy_t budget_policy, int set_cpu_set);

/* times are given in ns */
int sporadic_task_ns(
		lt_t e, lt_t p, lt_t phase,
		int cpu, task_class_t cls,
		budget_policy_t budget_policy, int set_cpu_set);

/* budget enforcement off by default in these macros */
#define sporadic_global(e, p) \
	sporadic_task(e, p, 0, 0, RT_CLASS_SOFT, NO_ENFORCEMENT, 0)
#define sporadic_partitioned(e, p, cpu) \
	sporadic_task(e, p, 0, cpu, RT_CLASS_SOFT, NO_ENFORCEMENT, 1)

/* file descriptor attached shared objects support */
typedef enum  {
	FMLP_SEM	= 0,
	SRP_SEM		= 1,
} obj_type_t;

int od_openx(int fd, obj_type_t type, int obj_id, void* config);
int od_close(int od);

static inline int od_open(int fd, obj_type_t type, int obj_id)
{
	return od_openx(fd, type, obj_id, 0);
}

/* real-time locking protocol support */
int litmus_lock(int od);
int litmus_unlock(int od);

/* job control*/
int get_job_no(unsigned int* job_no);
int wait_for_job_release(unsigned int job_no);
int sleep_next_period(void);

/*  library functions */
int  init_litmus(void);
int  init_rt_thread(void);
void exit_litmus(void);

/* A real-time program. */
typedef int (*rt_fn_t)(void*);

/* These two functions configure the RT task to use enforced exe budgets */
int create_rt_task(rt_fn_t rt_prog, void *arg, int cpu, int wcet, int period);
int __create_rt_task(rt_fn_t rt_prog, void *arg, int cpu, int wcet,
		     int period, task_class_t cls);

/*	per-task modes */
enum rt_task_mode_t {
	BACKGROUND_TASK = 0,
	LITMUS_RT_TASK  = 1
};
int task_mode(int target_mode);

void show_rt_param(struct rt_task* tp);
task_class_t str2class(const char* str);

/* non-preemptive section support */
void enter_np(void);
void exit_np(void);
int  requested_to_preempt(void);

/* task system support */
int wait_for_ts_release(void);
int release_ts(lt_t *delay);

#define __NS_PER_MS 1000000

static inline lt_t ms2lt(unsigned long milliseconds)
{
	return __NS_PER_MS * milliseconds;
}

/* CPU time consumed so far in seconds */
double cputime(void);

/* wall-clock time in seconds */
double wctime(void);

/* semaphore allocation */

static inline int open_fmlp_sem(int fd, int name)
{
	return od_open(fd, FMLP_SEM, name);
}

static inline int open_srp_sem(int fd, int name)
{
	return od_open(fd, SRP_SEM, name);
}


/* syscall overhead measuring */
int null_call(cycles_t *timestamp);

/*
 * get control page:
 * atm it is used only by preemption migration overhead code
 * but it is very general and can be used for different purposes
 */
struct control_page* get_ctrl_page(void);

typedef struct litmus_sigjmp
{
	sigjmp_buf env;
	struct litmus_sigjmp *prev;
} litmus_sigjmp_t;

void push_sigjmp(litmus_sigjmp_t* buf);
litmus_sigjmp_t* pop_sigjmp(void);

typedef void (*litmus_sig_handler_t)(int);
typedef void (*litmus_sig_actions_t)(int, siginfo_t *, void *);

void ignore_litmus_signals(unsigned long litmus_sig_mask);
void activate_litmus_signals(unsigned long litmus_sig_mask,
				litmus_sig_handler_t handler);
void activate_litmus_signal_actions(unsigned long litmus_sig_mask,
				litmus_sig_actions_t handler);
void block_litmus_signals(unsigned long litmus_sig_mask);
void unblock_litmus_signals(unsigned long litmus_sig_mask);

#define SIG_BUDGET_MASK			0x00000001
/* more ... */
#define ALL_LITMUS_SIGS_MASK	(SIG_BUDGET_MASK)

#define LITMUS_TRY \
do { \
	litmus_sigjmp_t lit_env_##__FUNCTION__##__LINE__; \
	push_sigjmp(&lit_env_##__FUNCTION__##__LINE__); \
	switch( sigsetjmp(lit_env_##__FUNCTION__##__LINE__.env, 1) ) { \
		case 0:

#define LITMUS_CATCH(x) break; case (x):

#define END_LITMUS_TRY \
	} /* end switch */ \
}

#ifdef __cplusplus
}
#endif




#ifdef __cplusplus
/* Expose litmus exceptions if C++.
 *
 * KLUDGE: We define everything in the header since liblitmus is a C-only
 * library, but this header could be included in C++ code.
 */

#include <exception>

namespace litmus
{
	class litmus_exception: public std::exception
	{
	public:
		litmus_exception() throw() {}
		virtual ~litmus_exception() throw() {}
		virtual const char* what() const throw() { return "litmus_exception";}
	};

	class sigbudget: public litmus_exception
	{
	public:
		sigbudget() throw() {}
		virtual ~sigbudget() throw() {}
		virtual const char* what() const throw() { return "sigbudget"; }
	};

	/* Must compile your program with "non-call-exception". */
	static void throw_on_litmus_signal(int signum) __used__
	{
		printf("WE GET SIGNAL! %d\n", signum);
		/* We have to unblock the received signal to get more in the future
		 * because we are not calling siglongjmp(), which normally restores
		 * the mask for us.
		 */
		switch(signum)
		{
		case SIG_BUDGET:
			unblock_litmus_signals(SIG_BUDGET_MASK);
			throw sigbudget();
		default:
			; /* silently ignore */
		}
	}

}; /* end namespace 'litmus' */

#endif /* end __cplusplus */

#endif
