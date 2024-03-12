#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/sort.h>
#include <thrust/transform.h>
#include <thrust/sequence.h>
#include <thrust/copy.h>
#include <thrust/count.h>


using namespace std;


void calculateSQRTM(int number);
void calculateParallel();
void printPrimes(thrust::device_vector<int> parallelPrimes);


int M, PRIMES_SQRTM_SIZE, SQRT_M;
int k = 0;
int PARALLEL_PRIMES_SIZE = 0;
double SEQUENTIAL_TIME, PARALLEL_TIME;

int *PRIMES_SQRTM;  // calculated primes until SQRT(M)
int *d_PRIMES_SQRTM;


struct isPrimeFunctor{
    int *d_PRIMES_SQRTM;
    isPrimeFunctor(int* d_PRIMES_SQRTM) : d_PRIMES_SQRTM(d_PRIMES_SQRTM) {}

    __host__ __device__
    int operator()(int num) const{    // Using the SQRT(M) primes calculated sequentially, check the number is prime or not.
    int K, QUO, REM;
    K = 1;
    while (1){
        QUO = num / d_PRIMES_SQRTM[K];
        REM = num % d_PRIMES_SQRTM[K];
        if (REM == 0){
            return 0;
            break;
        }
        if (QUO <= d_PRIMES_SQRTM[K] || d_PRIMES_SQRTM[K+1] == 0){
            return num;
            break;
        }
        K += 1;
    }
    return 0;
    }
};

struct isNotZeroFunctor{
    __host__ __device__
    bool operator()(int num) const{
        return num != 0;
    }
};


int main(int argc, char** argv) {
	if (argc!=2){
		cout <<"Usage: ./<COMPILED_FILE_NAME> <M>\n"<<endl;
		return 0;
	}
    auto start = chrono::high_resolution_clock::now();

	M = atoi(argv[1]);
	SQRT_M = (int) sqrt(M);
    cudaMallocHost((void**)&PRIMES_SQRTM, M/2 * sizeof(int));
    calculateSQRTM(SQRT_M);

    cudaMalloc((void**)&d_PRIMES_SQRTM, M /2 * sizeof(int));
    cudaMemcpy(d_PRIMES_SQRTM, PRIMES_SQRTM, M/2 * sizeof(int), cudaMemcpyHostToDevice);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    SEQUENTIAL_TIME = elapsed.count();

    start = chrono::high_resolution_clock::now();
    calculateParallel();
    end = chrono::high_resolution_clock::now();
    elapsed = end - start;
    PARALLEL_TIME = elapsed.count();

    cout << SEQUENTIAL_TIME + PARALLEL_TIME << endl;

    return 0;
}



void calculateSQRTM(int number){                   // Calculate primes until sqrt(M) sequentially
    int K, J, QUO, REM, N;
    PRIMES_SQRTM_SIZE = 0;
    PRIMES_SQRTM[0] =  2;
    PRIMES_SQRTM[1] =  3;
    PRIMES_SQRTM_SIZE += 2;
    J = 1;
    for (N = 3; N <= number; N+=2){
        K = 1;
        while (1){
            QUO = N / PRIMES_SQRTM[K];
            REM = N % PRIMES_SQRTM[K];
            if (REM == 0){
                break;
            }
            if (QUO <= PRIMES_SQRTM[K] || PRIMES_SQRTM[K+1] == 0){
                PRIMES_SQRTM[J+1] = N;
                PRIMES_SQRTM_SIZE += 1;
                J += 1;
                break;
            }
            K += 1;
        }
    }
}
void calculateParallel(){

    int start_index = (SQRT_M % 2 == 0) ? (SQRT_M + 1) : SQRT_M + 2;
    int end_index = M;  
    int size = end_index % 2 == 0 ? (end_index - start_index) / 2 + 1 : (end_index - start_index) / 2 + 2;

    thrust::device_vector<int> device_numbers(size);
    thrust::device_vector<int> out_device_numbers(size);


    thrust::sequence(device_numbers.begin(), device_numbers.end(), start_index, 2);
    thrust::transform(device_numbers.begin(), device_numbers.end(), out_device_numbers.begin(), isPrimeFunctor(d_PRIMES_SQRTM));
    PARALLEL_PRIMES_SIZE = thrust::count_if(out_device_numbers.begin(), out_device_numbers.end(), isNotZeroFunctor());
    

    thrust::device_vector<int> parallelPrimes(PARALLEL_PRIMES_SIZE);
    thrust::copy_if(out_device_numbers.begin(), out_device_numbers.end(), parallelPrimes.begin(), isNotZeroFunctor());

    // printPrimes(parallelPrimes);         // Uncomment this line to print the primes calculated in parallel
}



void printPrimes(thrust::device_vector<int> parallelPrimes){
	vector<int> primes_vector = {};
	int i;
	for (i=0;i<PRIMES_SQRTM_SIZE;i++)
		primes_vector.push_back(PRIMES_SQRTM[i]);

	thrust::sort(parallelPrimes.begin(), parallelPrimes.end());

	for (i=0;i<parallelPrimes.size();i++){
        primes_vector.push_back(parallelPrimes[i]);
    }
	
	for (i=0;i<primes_vector.size();i++)
		cout << primes_vector[i] << " ";
	cout << endl;
}