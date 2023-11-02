#include "str.c"    // YES! .c

#include <math.h>
#include <stdio.h>

#define MAXSIZ (31)

//int main(void)
int fake(void)
{
    int collisions[TABSIZE] = {0};
    char buffer[MAXSIZ + 1];
    const int N = TABSIZE * 256;

    for (int n = 0; n < N; ++ n) {
        // Generates a random string
        int len = 1 + ((double) rand() / RAND_MAX) * (MAXSIZ - 2);
        // Generates the string
        for (int i = 0; i < len; ++ i)
            buffer[i] = ((double) rand() / RAND_MAX) * (127-32) + 32;
        buffer[len] = '\0';
        ++ collisions[hash(buffer, len)];
    }
    double mean = (double) N / TABSIZE;
    double min = N;
    double max = 0;
    double std = 0;
    for (int n = 1; n < TABSIZE; ++ n) {
        max = fmax(collisions[n], max);
        min = fmin(collisions[n], min);
        std += pow(collisions[n] - mean, 2);
    }
    std = sqrt(std/TABSIZE);
    printf("%i bucket, %i words (%.0f words/bucket)\n", TABSIZE, N, (double) N / TABSIZE);
    printf("Min = %g < Mean = %g < Max = %g\n", min, mean, max);
    printf("Stdev = %g\n", std);
}
