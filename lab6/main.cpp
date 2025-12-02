#include <iostream>
#include <omp.h>
#include <cmath>
using namespace std;

int main() {
    int N = 300000000;
    double pi = 0.0;
    double start = omp_get_wtime();

    for (int n = 0; n < N; n++) {
        pi += pow(-1, n) / (2.0 * n + 1.0);
    }

    double end = omp_get_wtime();
    cout << "PI = " << 4 * pi << endl;
    cout << "Time: " << end - start << " sec" << endl;
    return 0;
}
