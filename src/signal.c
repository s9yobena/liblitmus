#include <stdio.h>
#include <string.h>

#include "litmus.h"
#include "internal.h"

static __thread litmus_sigjmp_t *g_sigjmp_tail = 0;

void throw_litmus_signal(int signum)
{
	litmus_sigjmp_t *lit_env;

	printf("WE GET SIGNAL!\n");
	lit_env = pop_sigjmp();
	if (lit_env) {
		printf("received signal %d!\n", signum);
		siglongjmp(lit_env->env, signum);
	}
	else {
		/* silently ignore the signal. */
	}
}

void push_sigjmp(litmus_sigjmp_t *buf)
{
	printf("push\n");
	buf->prev = g_sigjmp_tail;
	g_sigjmp_tail = buf;
}

litmus_sigjmp_t* pop_sigjmp(void)
{
	litmus_sigjmp_t* ret;
	printf("pop\n");
	ret = g_sigjmp_tail;
	g_sigjmp_tail = (ret) ? ret->prev : NULL;
	return ret;
}

static void reg_litmus_signals(unsigned long litmus_sig_mask,
		struct sigaction *pAction)
{
	int ret;

	if (litmus_sig_mask | SIG_BUDGET_MASK) { 
		ret = sigaction(SIG_BUDGET, pAction, NULL);
		check("SIG_BUDGET");
	}
	/* more ... */
}

void ignore_litmus_signals(unsigned long litmus_sig_mask)
{
	activate_litmus_signals(litmus_sig_mask, SIG_IGN);
}

void activate_litmus_signals(unsigned long litmus_sig_mask,
	litmus_sig_handler_t handle)
{
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = handle;

	reg_litmus_signals(litmus_sig_mask, &action);
}

void activate_litmus_signal_actions(unsigned long litmus_sig_mask,
		litmus_sig_actions_t handle)
{
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_sigaction = handle;
	action.sa_flags = SA_SIGINFO;

	reg_litmus_signals(litmus_sig_mask, &action);
}

void block_litmus_signals(unsigned long litmus_sig_mask)
{
	int ret;
	sigset_t sigs;
	sigemptyset(&sigs);

	if (litmus_sig_mask | SIG_BUDGET_MASK) {
		sigaddset(&sigs, SIG_BUDGET);
	}
	/* more ... */

	ret = sigprocmask(SIG_BLOCK, &sigs, NULL);
	check("SIG_BLOCK litmus signals");
}

void unblock_litmus_signals(unsigned long litmus_sig_mask)
{
	int ret;
	sigset_t sigs;
	sigemptyset(&sigs);

	if (litmus_sig_mask | SIG_BUDGET_MASK) {
		sigaddset(&sigs, SIG_BUDGET);
	}
	/* more ... */

	ret = sigprocmask(SIG_UNBLOCK, &sigs, NULL);
	check("SIG_UNBLOCK litmus signals");
}


void longjmp_on_litmus_signal(int signum)
{
	litmus_sigjmp_t *lit_env;
	printf("WE GET SIGNAL!\n");
	lit_env = pop_sigjmp();
	if (lit_env) {
		printf("received signal %d\n", signum);
		siglongjmp(lit_env->env, signum); /* restores signal mask */
	}
	else {
		/* silently ignore the signal */
	}
}
