#include <stdio.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;
typedef long long ll;

bool fileExists(string& filename);
void writeFile(string filename, string col1, string col2, string col3, string col4, string col5, string col6, string col7, string col8, string col9, string col10);
void calculateSequential(ll number, ll Primes[]);
vector<ll> calculateParallel(ll number, ll Primes[], ll PrimesSize, ll NumOfThreads, string scheduling, ll chunkSize);
void writeResults(ll i);
void printPrimes(ll Primes[], vector<ll> primeResults);

string FILE_NAME = "results.csv";
ll THREADS[4] = {1, 2, 4, 8};
ll M, CHUNK_SIZE, PRIMES_SEQUENTIAL;
string SCHEDULING[3] = {"static", "dynamic", "guided"};
double SEQUENTIAL_TIME, PARALLEL_TIME;
double TIME_RESULTS[3][4] = {0};
vector<ll> PARALLEL_PRIMES = {};


int main(int argc, char *argv[]){
    if (argc != 3){
        cout << "Usage: ./<OUTPUT_FILE> <M> <CHUNK_SIZE>\n";
        return -1;
    }

    M = atoll(argv[1]);
    CHUNK_SIZE = atoll(argv[2]);
    if (M < 1 || CHUNK_SIZE < 1){
        cout << "M and Chunk Size should be positive numbers\n";
        return -1;
    }

    if (!fileExists(FILE_NAME)){                                      // Write header to file if the file doesn't exist
        writeFile(FILE_NAME,"M","Scheduling Method","Chunk Size","T1","T2","T4","T8","S2","S4","S8");
    }

    ll SQRT_M = (ll) sqrt(M);
    ll PRIMES[SQRT_M] = {0};                                            // M = 2 is edge case assuming M is always greater than 2

    double SequentialStartTime = omp_get_wtime();
    calculateSequential(SQRT_M, PRIMES);
    double SequentialEndTime = omp_get_wtime();
    SEQUENTIAL_TIME = SequentialEndTime - SequentialStartTime;         

    for (int i = 0; i < 3; i++){
        for(int j = 0; j < 4; j++){
            string scheduling = SCHEDULING[i];
            ll NumOfThreads = THREADS[j];
            PARALLEL_PRIMES = calculateParallel(M, PRIMES, SQRT_M, NumOfThreads, scheduling, CHUNK_SIZE);
            TIME_RESULTS[i][j] = SEQUENTIAL_TIME + PARALLEL_TIME;
        }
        writeResults(i);
    }
    // printPrimes(PRIMES, PARALLEL_PRIMES);         // Uncomment to print all primes in increasing order into the terminal
    return 0;
}

bool fileExists(string& filename){
    ifstream file(filename);
    return file.good();
}

void writeFile(string filename, string col1, string col2, string col3, string col4, string col5, string col6, string col7, string col8, string col9, string col10){
    fstream file;
    file.open(filename, ios::out|ios::app);
    if (file){
        file << col1 << "," << col2 << "," << col3 << "," << col4 << "," << col5 << "," << col6 << "," << col7 << "," << col8 << "," << col9 << "," << col10 << "\n";
        file.close();
    }
}

void calculateSequential(ll number, ll Primes[]){                   // Calculate primes until sqrt(M) sequentially
    ll K, J, QUO, REM, N;
    PRIMES_SEQUENTIAL = 0;
    Primes[0] = (ll) 2;
    Primes[1] = (ll) 3;
    PRIMES_SEQUENTIAL += 2;
    J = 1;

    for (N = 3; N < number; N+=2){
        K = 1;
        while (1){
            QUO = N / Primes[K];
            REM = N % Primes[K];
            if (REM == 0){
                break;
            }
            if (QUO <= Primes[K] || Primes[K+1] == 0){
                Primes[J+1] = N;
                PRIMES_SEQUENTIAL += 1;
                J += 1;
                break;
            }
            K += 1;
        }
    }
}

vector<ll> calculateParallel(ll number, ll Primes[], ll PrimesSize, ll NumOfThreads, string scheduling, ll chunkSize){ // Calculate primes until M in parallel
    omp_lock_t lock;
    omp_init_lock(&lock);

    vector<ll> primeResults = {};
    ll K, J, QUO, REM, N;
    N = number;
    ll startIndex = (PrimesSize % 2 == 1) ? (PrimesSize + 2) : (PrimesSize + 1);

    omp_set_num_threads(NumOfThreads);
    if (scheduling == "static") omp_set_schedule(omp_sched_static, chunkSize);
    else if (scheduling == "dynamic") omp_set_schedule(omp_sched_dynamic, chunkSize);
    else if (scheduling == "guided") omp_set_schedule(omp_sched_guided, chunkSize);

    double ParallelStartTime = omp_get_wtime();
    #pragma omp parallel for schedule(runtime) private(K, QUO, REM) shared(N, Primes, primeResults, lock)
    for (J = startIndex; J <= N; J+=2){
        K = 1;
        while (1){
            QUO = J / Primes[K];
            REM = J % Primes[K];
            if (REM == 0){
                break;
            }
            if (QUO <= Primes[K] || Primes[K+1] == 0){
                omp_set_lock(&lock);
                primeResults.push_back(J);
                omp_unset_lock(&lock);
                break;
            }
            K += 1;
        }
    }
    double ParallelEndTime = omp_get_wtime();
    PARALLEL_TIME = ParallelEndTime - ParallelStartTime;
    omp_destroy_lock(&lock);
    return primeResults;
}

void writeResults(ll i){
    string scheduling = SCHEDULING[i];
    if (i != 1) scheduling += " ";                                     // To allign timings and make the output look better
    string M_str = to_string(M);
    string chunksize_str = to_string(CHUNK_SIZE);
    string T1 = to_string(TIME_RESULTS[i][0]);
    string T2 = to_string(TIME_RESULTS[i][1]);
    string T4 = to_string(TIME_RESULTS[i][2]);
    string T8 = to_string(TIME_RESULTS[i][3]);
    string S2 = to_string(TIME_RESULTS[i][0] / TIME_RESULTS[i][1]);
    string S4 = to_string(TIME_RESULTS[i][0] / TIME_RESULTS[i][2]);
    string S8 = to_string(TIME_RESULTS[i][0] / TIME_RESULTS[i][3]);
    writeFile(FILE_NAME, M_str, scheduling, chunksize_str, T1, T2, T4, T8, S2, S4, S8);
}

void printPrimes(ll Primes[], vector<ll> primeResults){
    vector <ll> allPrimes = {};
    for (ll i = 0; i < PRIMES_SEQUENTIAL; i++){
        allPrimes.push_back(Primes[i]);
    }
    for (ll i = 0; i < primeResults.size(); i++){
        allPrimes.push_back(primeResults[i]);
    }
    sort(allPrimes.begin(), allPrimes.end());
    for (ll i = 0; i < allPrimes.size(); i++){
        cout << allPrimes[i] << " ";
    }
    cout << "\n";
}
