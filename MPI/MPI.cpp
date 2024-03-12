#include <stdio.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "mpi.h"

using namespace std;
typedef long long ll;


void calculateSequential(ll number);
void calculateParallel(int world_rank, int world_size);
bool isPrimeOptimized(ll num);
void printPrimes();



ll M, PRIMES_SEQUENTIAL_SIZE, SQRT_M;
ll k = 0;

double SEQUENTIAL_TIME, PARALLEL_TIME, ParallelStartTime;


ll *PRIMES_SEQUENTIAL;  // calculated primes until SQRT(M)
ll *curPrimes;          // primes calculated by one thread
ll *recvPrimes;         // Temporary array to recive one thread's primes.
ll recvSize;

vector <ll> parallelPrimes; 

int main(int argc, char** argv) {

	if (argc!=2){
		cout <<"Usage: mpiexec --use-hwthread-cpus -np <NUMBER_OF_THREADS> ./<COMPILED_FILE_NAME> <M>\n"<<endl;
		return 0;
	}
	int world_rank;
	int world_size;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	M = atoll(argv[1]);
	SQRT_M = (ll) sqrt(M);
	PRIMES_SEQUENTIAL = new ll[M/2];

	if (world_rank==0){

		double SequentialStartTime = MPI_Wtime();
		calculateSequential(SQRT_M);                                      // Calculate numbers up to SQRT(M) sequentially
		double SequentialEndTime = MPI_Wtime();
		SEQUENTIAL_TIME = SequentialEndTime - SequentialStartTime;
	}

	MPI_Barrier(MPI_COMM_WORLD);                                          // Wait for master thread to complete calculating PRIMES_SEQUENTIAL array,
	MPI_Bcast(PRIMES_SEQUENTIAL,SQRT_M,MPI_LONG_LONG,0,MPI_COMM_WORLD);   // then Broadcast it to every other thread

	calculateParallel(world_rank, world_size);                            // Calculate numbers from SQRT(M) to M in parallel

	if (world_rank==0){  											   // If we are in the main process, recieve all the primes from other processes and add them to our parallelPrimes vector
		ll i;
		for (i = 0; i < k; i++)    	
			parallelPrimes.push_back(curPrimes[i]);
		delete curPrimes;
		
		for (i = 1; i < world_size; i++){                          //go through all the recieved primes and add them to our parallelPrimes vector
			MPI_Recv(&recvSize, 1, MPI_LONG_LONG, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			recvPrimes = new ll[recvSize];
			MPI_Recv(recvPrimes, recvSize, MPI_LONG_LONG, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			for (ll j = 0; j < recvSize; j++)    	
				parallelPrimes.push_back(recvPrimes[j]);
			delete recvPrimes;
		}
		double ParallelEndTime = MPI_Wtime();
		PARALLEL_TIME = ParallelEndTime - ParallelStartTime;
		cout << SEQUENTIAL_TIME + PARALLEL_TIME << endl;

		// printPrimes();                                           // Uncomment to print the primes in increasing order in the terminal.
	}
	MPI_Finalize();
    return 0;
}



void calculateSequential(ll number){                   // Calculate primes until sqrt(M) sequentially
    ll K, J, QUO, REM, N;
    PRIMES_SEQUENTIAL_SIZE = 0;
    PRIMES_SEQUENTIAL[0] =  2;
    PRIMES_SEQUENTIAL[1] =  3;
    PRIMES_SEQUENTIAL_SIZE += 2;
    J = 1;
    for (N = 3; N <= number; N+=2){
        K = 1;
        while (1){
            QUO = N / PRIMES_SEQUENTIAL[K];
            REM = N % PRIMES_SEQUENTIAL[K];
            if (REM == 0){
                break;
            }
            if (QUO <= PRIMES_SEQUENTIAL[K] || PRIMES_SEQUENTIAL[K+1] == 0){
                PRIMES_SEQUENTIAL[J+1] = N;
                PRIMES_SEQUENTIAL_SIZE += 1;
                J += 1;
                break;
            }
            K += 1;
        }
    }
}
void calculateParallel(int world_rank, int world_size){              // Calculate primes from SQRT(M) to M in parallel
	ParallelStartTime = MPI_Wtime();

	curPrimes = new ll[M/2];
    ll x,i;
	x = (SQRT_M % 2 == 1) ? (SQRT_M + 2) : (SQRT_M + 1);                  // Start index (Odd number just after the SQRT(M))
	
	
	for (i = x + (M - x) *world_rank / world_size; i < x + (M -x)*(world_rank + 1) / world_size; i+=2){  // Divide numbers equally to the threads.
		if (i % 2 == 0)            i+=1;                           // In some cases, one thread's last num is odd, So next thread's first num is even. Just increment it by 1.
		if (isPrimeOptimized(i))   curPrimes[k++]=i;
	}

	if (world_rank!=0){                                            //if we are not in the main process(process with rank 0) send our prime numbers to main process
		MPI_Send(&k, 1, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD);
		MPI_Send(curPrimes, k, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD);
	}
	
}


bool isPrimeOptimized(ll num){    // Using the SQRT(M) primes calculated sequentially, check the number is prime or not.
	ll K, QUO, REM, N;
	K = 1;
	while (1){
		QUO = num / PRIMES_SEQUENTIAL[K];
		REM = num % PRIMES_SEQUENTIAL[K];
		if (REM == 0){
			return false;
			break;
		}
		if (QUO <= PRIMES_SEQUENTIAL[K] || PRIMES_SEQUENTIAL[K+1] == 0){
			return true;
			break;
		}
		K += 1;
	}
	return true;
}

void printPrimes(){
	vector<ll> primes_vector = {};
	ll i;
	for (i=0;i<PRIMES_SEQUENTIAL_SIZE;i++)
		primes_vector.push_back(PRIMES_SEQUENTIAL[i]);
	sort(parallelPrimes.begin(), parallelPrimes.end());
	for (i=0;i<parallelPrimes.size();i++)
		primes_vector.push_back(parallelPrimes[i]);
	
	for (i=0;i<primes_vector.size();i++)
		cout << primes_vector[i] << " ";
	cout << endl;
}