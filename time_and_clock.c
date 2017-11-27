#include<stdio.h>
#include<time.h>

//https://linux.die.net/man/3/clock_gettime
int main(int argc,char *argv[])
{        
    struct timespec tspec = {0, 0};   

    clock_getres(CLOCK_REALTIME, &tspec);
    printf("\nResolution of CLOCK_REALTIME: %d seconds, %d nanoseconds\n", tspec.tv_sec, tspec.tv_nsec);  

    clock_gettime(CLOCK_REALTIME, &tspec);        
    printf("Clock real time: %d seconds, %d nanoseconds\n\n", tspec.tv_sec, tspec.tv_nsec);  

    clock_getres(CLOCK_MONOTONIC, &tspec);
    printf("Resolution of CLOCK_MONOTONIC: %d seconds, %d nanoseconds\n", tspec.tv_sec, tspec.tv_nsec);  

    clock_gettime(CLOCK_MONOTONIC, &tspec);        
    printf("CLOCK_MONOTONIC (time since system start): %d seoncds, %d nanoseconds\n\n", tspec.tv_sec, tspec.tv_nsec);    

    clock_getres(CLOCK_PROCESS_CPUTIME_ID, &tspec);
    printf("Resoltuon of CLOCK_PROCESS_CPUTIME_ID : %d seoncds, %d nanoseconds\n", tspec.tv_sec, tspec.tv_nsec);    

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tspec);        
    printf("CLOCK_PROCESS_CPUTIME_ID: %d seconds, %d nanoseconds\n\n", tspec.tv_sec, tspec.tv_nsec);   

    clock_getres(CLOCK_THREAD_CPUTIME_ID, &tspec);
    printf("Resoltuon of CLOCK_THREAD_CPUTIME_ID: %d seoncds, %d nanoseconds\n", tspec.tv_sec, tspec.tv_nsec);    

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tspec);        
    printf("CLOCK_THREAD_CPUTIME_ID: %d seconds, %d nanoseconds\n\n", tspec.tv_sec, tspec.tv_nsec);   

    printf("time(NULL): %d seconds\n", time(NULL)); 

    return 0;
}
