#include <stdio.h>

int a[100];

int main()
{
	for (int j = 0; j < 100; j++) {
    	a[j] = j;
    	for (int i = j; i < 5; i = i + 2) {
    		a[i] = 1;
    	}
    }
    for (int x = 0; x < 100 ; x++) {
    	printf("%d\n", a[x]);
    }
    return 0;
}
