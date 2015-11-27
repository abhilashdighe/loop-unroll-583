#include <stdio.h>
#include <time.h>

int main ()
{
    time_t secondsBefore, secondsAfter ;

    int a = 0;
    secondsBefore = time(NULL);
    for(int i = 0  ; i < 10 ; i++){
        a +=1;
    }
    secondsAfter = time(NULL);
    printf("Time elapsed: %ld" , secondsAfter - secondsBefore);
    return(0);
}
