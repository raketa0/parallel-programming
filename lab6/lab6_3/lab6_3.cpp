#include <iostream>
#include <omp.h>
#include <cmath>
using namespace std;

int main() {
    int N = 100000000;
    double pi = 0.0;
    double start = omp_get_wtime();

    #pragma omp parallel for
    for (long long n = 0; n < N; n++) 
    {
        double term = pow(-1, n) / (2.0 * n + 1.0);
        #pragma omp atomic
        pi += term;
    }

    double end = omp_get_wtime();
    cout << "PI = " << 4 * pi << endl;
    cout << "Time: " << end - start << " sec" << endl;
    return 0;
}
