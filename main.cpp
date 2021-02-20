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

    // TODO -> TO uncomment
//    printPrimeNumber(primeNumbersPar);

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
        begin = en
#pragma omp ford;
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
    omp_set_num_threads(THREAD_NUMBER);

    auto chPar = Chrono(true);
    vector<vector<tuple<mpz_class, mpz_class>>> splitVector = SplitVector(INTERVALS, THREAD_NUMBER);

    // Master vector
    vector<mpz_class> m_primeNumbersPar;

#pragma omp parallel default(none) shared(splitVector, m_primeNumbersPar)
    {
        /// Slave vector
        vector<vector<mpz_class>> s_primeNumbersPar;

#pragma omp for nowait schedule(dynamic)
        for (size_t i = 0; i < splitVector.size(); ++i) {
            s_primeNumbersPar.emplace_back(MillerRabinSeq::computePrime(splitVector[i]));
        }

#pragma omp critical
        {
            for (vector<mpz_class> vec : s_primeNumbersPar) {
                m_primeNumbersPar.insert(m_primeNumbersPar.end(),
                                         make_move_iterator(vec.begin()),
                                         make_move_iterator(vec.end())
                );
            }
        };
    };
    chPar.pause();

/**
 * DISPLAY
 */
    primeNbDisplay(primeNumbersSeq, m_primeNumbersPar);
    inputPrint(INTERVALS, THREAD_NUMBER);
    chronoExecution(chInterval, chSeq, chPar);

    return 0;
}