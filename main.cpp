#include <getopt.h>
#include <iostream>
#include <gmpxx.h>
#include <chrono>
#include <omp.h>

#include "ArgParse.hpp"
#include "FileParse.hpp"
#include "MillerRabinSeq.hpp"
#include "Chrono.hpp"

using namespace std;

/***
 * Print chrono execution time
 * @param chInterval
 * @param chSeq
 * @param chPara
 */
void chronoExecution(Chrono &chInterval, Chrono &chSeq, Chrono &chPara) {
    double chronoSeq = chInterval.get() + chSeq.get();
    double chronoPar = chInterval.get() + chPara.get();

    cout << "\n--- Execution Time ---" << endl;

    cout << "Interval Optimization time : " << chInterval.get() << " sec" << endl;

    fprintf(stderr, "Total Sequential execution time : %f \n", chronoSeq);
    fprintf(stderr, "Total Parallel execution time : %f \n", chronoPar);

    cout << "SPEEDUP : " << chSeq.get() / chPara.get() << endl;
}

/***
 * Print all prime number found
 * @param primeNumbersPar
 */
void printPrimeNumber(const vector<mpz_class> &primeNumbersPar) {
    for (const mpz_class &prime : primeNumbersPar) {
        cout << prime << " ";
    }
    cout << endl;
}

/***
 * Display size and primeNumber
 * @param primeNumbersSeq
 * @param primeNumbersPar
 */
void primeNbDisplay(const vector<mpz_class> &primeNumbersSeq, const vector<mpz_class> &primeNumbersPar) {
    cout << "\n--- Prime Numbers ---" << endl;

    printPrimeNumber(primeNumbersPar);

    cout << endl << primeNumbersSeq.size() << " Prime number found with sequential method" << endl;
    cout << primeNumbersPar.size() << " Prime number found with Parallel method" << endl << endl;
}

/***
 * Handle user input
 * @param argc
 * @param argv
 * @param THREAD_NUMBER
 * @param FILEPATH
 */
void handleInputs(int argc, char **argv, size_t &THREAD_NUMBER, char *&FILEPATH) {
    THREAD_NUMBER = 0;
    int args;
    int FILE_FLAG = 0;
    int THREAD_FLAG = 0;
    while ((args = getopt(argc, argv, "t:f:")) != -1) {
        auto ap = ArgParse(optarg);

        switch (args) {
            case 't' :
                THREAD_FLAG = 1;
                THREAD_NUMBER = ap.getParseInt();
                break;
            case 'f':
                FILE_FLAG = 1;
                FILEPATH = optarg;
                break;
            default:
                abort();
        }
    }
    if (!FILE_FLAG) {
        cout << "Please use -f <file_path> to load file." << endl;
        exit(0);
    }
    if (!THREAD_FLAG) {
        cout << "Default number of flag is 1." << endl;
    }
}

/***
 * Print user input
 * @param intervals
 * @param threadNb
 */
void inputPrint(const vector<tuple<mpz_class, mpz_class>> &intervals, int threadNb) {
    cout << "\n--- Optimized Interval ---" << endl;
    cout << intervals.size() << " Intervals to handle during prime computing" << endl;

    cout << "\n--- Threads Number --- " << endl;
    cout << threadNb << endl;
}

/***
 * Split Interval Vector to divide works for thread
 * @tparam T
 * @param vec
 * @param n
 * @return vector of vector of tuple
 */
template<typename T>
vector<vector<T>> SplitVector(const vector<T> &vec, size_t n) {
    vector<vector<T>> outVec;

    size_t length = vec.size() / n;
    size_t remain = vec.size() % n;

    size_t begin = 0;
    size_t end = 0;

    for (size_t i = 0; i < min(n, vec.size()); ++i) {
        end += (remain > 0) ? (length + !!(remain--)) : length;
        outVec.push_back(vector<T>(vec.begin() + begin, vec.begin() + end));
        begin = end;
    }
    return outVec;
}

/// ./pp_tp1 -t <threadNumber> -f <file_with_interval>
int main(int argc, char **argv) {
    vector<tuple<mpz_class, mpz_class>> INTERVALS;
    vector<tuple<mpz_class, mpz_class>> readIntervals;

    size_t THREAD_NUMBER;
    char *FILEPATH;
    handleInputs(argc, argv, THREAD_NUMBER, FILEPATH);

    readIntervals = FileParse::readFile(FILEPATH);
    auto chInterval = Chrono(true);
    INTERVALS = FileParse::intervalsOptimisation(readIntervals, THREAD_NUMBER);
    chInterval.pause();

    /**
     * MillerRabin Sequential
     */
    auto chSeq = Chrono(true);
    vector<mpz_class> primeNumbersSeq = MillerRabinSeq::computePrime(INTERVALS);
    chSeq.pause();

    /**
     * SOLUTION - OpenMP solution
     */
    auto chPar = Chrono(true);
    omp_set_num_threads(THREAD_NUMBER);
    vector<vector<tuple<mpz_class, mpz_class>>> splitVector = SplitVector(INTERVALS, THREAD_NUMBER);

    #pragma omp parallel for default(none) shared(splitVector)
    for (size_t i = 0; i < splitVector.size(); ++i) {
        printf("Thread : %d \n", omp_get_thread_num());
        MillerRabinSeq::computePrime(splitVector[i]);
    }

//    int ind = 0;
//    for (const vector<tuple<mpz_class, mpz_class>>& vec : splitVector){
//        cout << "Thread " << ind << endl;
//        FileParse::printTupleVector(vec);
//        ind++;
//
//        MillerRabinSeq::computePrime(vec);
//    }


    vector<mpz_class> primeNumbersPar;
    chPar.pause();
    /**
     * DISPLAY
     */
    primeNbDisplay(primeNumbersSeq, primeNumbersPar);
    inputPrint(INTERVALS, THREAD_NUMBER);
    chronoExecution(chInterval, chSeq, chPar);

    return 0;
}