#ifndef ZIPFIAN_H
#define ZIPFIAN_H

#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;
class Zipfian
{
public:
    double *pf;
    void InitZipfian(int A, int num)
    {
        pf = new double[num];
        double sum = 0.0;
        for (int i = 0; i < num; i++)
        {
            sum += 1 / pow((double)(i + 2), A);
        }
        for (int i = 0; i < num; i++)
        {
            if (i == 0)
                pf[i] = 1 / pow((double)(i + 2), A) / sum;
            else
                pf[i] = pf[i - 1] + 1 / pow((double)(i + 2), A) / sum;
        }
    }

    int GenerateNextIndex()
    {
        srand(time(0));
        int index = 0;
        double data = (double)rand() / RAND_MAX; // 0-1
        while (data > pf[index])
            index++;
        return index;
    }
};

#endif // !ZIPFIAN_H