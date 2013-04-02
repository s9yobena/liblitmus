#include <stdio.h>
#include <stdlib.h>


#include "litmus.h"

#define MAX_RT_TASK_NR 10

int sched_test(struct rt_task rt_task_param[], int nb_rts);
int get_nbr_cpus();
int get_overhead();

int main(int argc, char** argv)
{
  int  rt_task_id[MAX_RT_TASK_NR];
  struct rt_task rt_task_param[MAX_RT_TASK_NR];
  int i;
  int nb_rts;
  nb_rts = 0;

  for (i=1;i<argc;i++){
    rt_task_id[i-1] = atoi(argv[i]);
    get_rt_task_param(rt_task_id[i-1],  &rt_task_param[i-1] ); 
    nb_rts ++;
  }
  

  for (i=0;i<nb_rts;i++){
    printf("rt_task %d: e = %d, p = %d \n",rt_task_id[i], 
	   (int)rt_task_param[i].exec_cost, 
	   (int)rt_task_param[i].period);
  }

  if (sched_test(rt_task_param,nb_rts))
    printf("Yeah, task set schedulable\n");
  else
    printf("Oh ****, task set is not schedulable\n");

  return 0;
}


/* Test if the task set is schedulabale using the density test.
 * GSN-EDF "global EDF", is used.
 * Currently overheads are not taken care of.
 * Implicit deadlines are assumed. i.e Deadline = Period.
 * 
 * TODO: retrieve number of processors
 */
int sched_test(struct rt_task rt_task_param[], int nb_rts) {

  int i;
  long double sum_density;
  long double max_density;
  long double cur_density;
  int nbr_cpu;
  nbr_cpu = get_nbr_cpus();
  sum_density = 0;
  max_density = 0;
  cur_density = 0;
  for (i=0; i<nb_rts; i++){
    cur_density =(long double)rt_task_param[i].exec_cost /(long double)rt_task_param[i].period;
    if (cur_density > max_density) {
      max_density = cur_density;
    }
    sum_density += cur_density;
    
  }
  
  printf("sum_density  ?<> nbr_cpu - (nbr_cpu-1)*max_density: %Lf ?<> %Lf",sum_density,nbr_cpu-(nbr_cpu-1)*max_density);
  
  if (sum_density <= (long double)nbr_cpu-((long double)nbr_cpu-1)*max_density) {
    return 1;
  } else {
    return 0;
  }  

}

int get_nbr_cpus(){
        FILE * fp;
        char res[128];
        fp = popen("/bin/cat /proc/cpuinfo |grep -c '^processor'","r");
        fread(res, 1, sizeof(res)-1, fp);
        pclose(fp);
	/* printf("nbr of cpus: %c \n",res[0]); */
        return atoi(&res[0]);
}
