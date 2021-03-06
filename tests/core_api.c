#include <sys/wait.h> /* for waitpid() */
#include <unistd.h>
#include <stdio.h>

#include "tests.h"
#include "litmus.h"


TESTCASE(set_rt_task_param_invalid_pointer, ALL,
	 "reject invalid rt_task pointers")
{
	SYSCALL_FAILS( EINVAL, set_rt_task_param(gettid(), NULL));

	SYSCALL_FAILS( EFAULT, set_rt_task_param(gettid(), (void*) 0x123 ));
}

TESTCASE(set_rt_task_param_invalid_params, ALL,
	 "reject invalid rt_task values")
{
	struct rt_task params;
	params.cpu        = 0;
	params.period     = 100;
	params.relative_deadline = params.period;
	params.phase      = 0;
	params.priority	  = LITMUS_LOWEST_PRIORITY;
	params.cls        = RT_CLASS_HARD;
	params.budget_policy = NO_ENFORCEMENT;

	/* over utilize */
	params.exec_cost  = 110;
	SYSCALL_FAILS( EINVAL, set_rt_task_param(gettid(), &params) );

	/* bad CPU */
	params.exec_cost = 90;
	params.cpu       = -1;
	SYSCALL_FAILS( EINVAL, set_rt_task_param(gettid(), &params) );

	/* infeasible density */
	params.cpu  = 0;
	params.relative_deadline = 30;
	SYSCALL_FAILS( EINVAL, set_rt_task_param(gettid(), &params) );

	/* bad task */
	params.relative_deadline = params.period;
	SYSCALL_FAILS( EINVAL, set_rt_task_param(-1, &params) );


	/* now try correct params */
	SYSCALL( set_rt_task_param(gettid(), &params) );
}

TESTCASE(reject_bad_priorities, P_FP,
	 "reject invalid priorities")
{
	struct rt_task params;
	params.cpu        = 0;
	params.exec_cost  =  10;
	params.period     = 100;
	params.relative_deadline = params.period;
	params.phase      = 0;
	params.cls        = RT_CLASS_HARD;
	params.budget_policy = NO_ENFORCEMENT;

	SYSCALL( be_migrate_to(params.cpu) );

	/* too high */
	params.priority	  = 0;
	SYSCALL( set_rt_task_param(gettid(), &params) );
	SYSCALL_FAILS( EINVAL, task_mode(LITMUS_RT_TASK) );

	/* too low */
	params.priority   = LITMUS_MAX_PRIORITY;
	SYSCALL( set_rt_task_param(gettid(), &params) );
	SYSCALL_FAILS( EINVAL, task_mode(LITMUS_RT_TASK) );

}

TESTCASE(accept_valid_priorities, P_FP,
	 "accept lowest and highest valid priorities")
{
	struct rt_task params;
	params.cpu        = 0;
	params.exec_cost  =  10;
	params.period     = 100;
	params.relative_deadline = params.period;
	params.phase      = 0;
	params.cls        = RT_CLASS_HARD;
	params.budget_policy = NO_ENFORCEMENT;

	SYSCALL( be_migrate_to(params.cpu) );

	/* acceptable */
	params.priority   = LITMUS_LOWEST_PRIORITY;
	SYSCALL( set_rt_task_param(gettid(), &params) );
	SYSCALL( task_mode(LITMUS_RT_TASK) );
	SYSCALL( task_mode(BACKGROUND_TASK) );

	params.priority   = LITMUS_HIGHEST_PRIORITY;
	SYSCALL( set_rt_task_param(gettid(), &params) );
	SYSCALL( task_mode(LITMUS_RT_TASK) );
	SYSCALL( task_mode(BACKGROUND_TASK) );
}

TESTCASE(job_control_non_rt, ALL,
	 "reject job control for non-rt tasks")
{
	unsigned int job_no;

	SYSCALL_FAILS( EINVAL, sleep_next_period() );

	SYSCALL_FAILS( EINVAL, wait_for_job_release(0) );

	SYSCALL_FAILS( EPERM, get_job_no(&job_no) );
}


TESTCASE(rt_fork_non_rt, LITMUS,
	 "children of RT tasks are not automatically RT tasks")
{
	unsigned int pid, job_no;
	int status;

	SYSCALL( sporadic_partitioned(10, 100, 0) );
	SYSCALL( task_mode(LITMUS_RT_TASK) );

	pid = fork();

	ASSERT( pid != -1 );

	if (pid == 0) {
		/* child */

		SYSCALL_FAILS( EINVAL, sleep_next_period() );
		SYSCALL_FAILS( EINVAL, wait_for_job_release(0) );
		SYSCALL_FAILS( EPERM, get_job_no(&job_no) );

		exit(0);
	} else {
		/* parent */

		SYSCALL( sleep_next_period() );
		SYSCALL( wait_for_job_release(20) );
		SYSCALL( get_job_no(&job_no) );

		SYSCALL( task_mode(BACKGROUND_TASK) );

		SYSCALL( waitpid(pid, &status, 0) );

		ASSERT(WEXITSTATUS(status) == 0);
	}
}

TESTCASE(ctrl_page_writable, ALL,
	 "tasks have write access to /dev/litmus/ctrl mappings")
{
	volatile int *ctrl_page = (volatile int*) get_ctrl_page();

	/* init_litmus() should have mapped the page already  */
	ASSERT(ctrl_page != NULL);

	/* These should work without page faults. */
	enter_np();
	exit_np();

	/* Try poking the memory directly. */

	ctrl_page[32] = 0x12345678;
}
