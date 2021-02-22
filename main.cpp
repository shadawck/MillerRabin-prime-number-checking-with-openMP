#include <getopt.h>
#include <iostream>
#include <gmpxx.h>
#include <chrono>
#include <omp.h>
#include <unistd.h>
#include <vector>
#include <numeric>

#include "ArgParse.hpp"
#include "FileParse.hpp"
#include "MillerRabinSeq.hpp"
#include "Chrono.hpp"

void mergeMasterAndSlaveVector(vector<mpz_class> &m_primeNumbersPar, vector<vector<mpz_class>> &s_primeNumbersPar);

void
mergeMasterAndSlaveTimeVector(vector<double> &m_chronoThread, double start, vector<double> &s_chronoThread, double end);

void threadTimeDisplay(vector<double> &m_chronoThread);

void printIntervalForEachThread(vector<vector<tuple<mpz_class, mpz_class>>> &splitVector);

void chronoExecutionDisplay(Chrono &chInterval, Chrono &chSeq, Chrono &chPara);

void primeNbDisplay(const vector<mpz_class> &primeNumbersPar);

void primeNbStatDisplay(const vector<mpz_class> &primeNumbersSeq, const vector<mpz_class> &primeNumbersPar);

void handleInputs(int argc, char **argv, size_t &THREAD_NUMBER, char *&FILEPATH);

void setupInputs(int argc, char **argv, size_t &THREAD_NUMBER, char *&FILEPATH);

void inputPrintDisplay(const vector<tuple<mpz_class, mpz_class>> &intervals, int threadNb);

template<typename T>
vector<vector<T>> SplitVector(const vector<T> &vec, size_t n);

using namespace std;

/// ./pp_tp1 -t <threadNumber> -f <file_with_interval>
int main(int argc, char **argv) {
    vector<tuple<mpz_class, mpz_class>> INTERVALS;
    vector<tuple<mpz_class, mpz_class>> readIntervals;

    size_t THREAD_NUMBER;
    char *FILEPATH;

    setupInputs(argc, argv, THREAD_NUMBER, FILEPATH);
    // Enter input with flag. Best for us. Debug
    //handleInputs(argc, argv, THREAD_NUMBER, FILEPATH);

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

    // Uncomment to see intervals in threads
    // printIntervalForEachThread(splitVector);

    /// Master vector
    vector<mpz_class> m_primeNumbersPar;
    vector<double> m_chronoThread;

#pragma omp parallel default(none) shared(splitVector, m_primeNumbersPar, m_chronoThread)
    {
        double start = omp_get_wtime();

        /// Slave vector
        vector<vector<mpz_class>> s_primeNumbersPar;
        vector<double> s_chronoThread;

#pragma omp for nowait schedule(dynamic, 1)
        for (size_t i = 0; i < splitVector.size(); ++i) {
            s_primeNumbersPar.emplace_back(MillerRabinSeq::computePrime(splitVector[i]));
        }

#pragma omp critical
        {
            mergeMasterAndSlaveVector(m_primeNumbersPar, s_primeNumbersPar);
        };

        double end = omp_get_wtime();
        mergeMasterAndSlaveTimeVector(m_chronoThread, start, s_chronoThread, end);
    };

    sort(m_primeNumbersPar.begin(), m_primeNumbersPar.end());
    chPar.pause();

    /**
    * DISPLAY
    */
    primeNbDisplay(m_primeNumbersPar);
    chronoExecutionDisplay(chInterval, chSeq, chPar);

    /// If you only want primeNumber and chrono time on stdout/stderr, comment the 3 function below
    primeNbStatDisplay(primeNumbersSeq, m_primeNumbersPar);
    inputPrintDisplay(INTERVALS, THREAD_NUMBER);
    threadTimeDisplay(m_chronoThread);

    return 0;
}

/***
 * Print chrono execution time
 * @param chInterval
 * @param chSeq
 * @param chPara
 */
void chronoExecutionDisplay(Chrono &chInterval, Chrono &chSeq, Chrono &chPara) {
    double chronoSeq = chInterval.get() + chSeq.get();
    double chronoPar = chInterval.get() + chPara.get();

    cout << "\n--- Execution Time ---" << endl;

    cout << "Interval Optimization time : " << chInterval.get() << " sec" << endl;

    fprintf(stderr, "Total Sequential execution time : %f \n", chronoSeq);
    fprintf(stderr, "Total Parallel execution time : %f \n", chronoPar);

    cout << "SPEEDUP : " << chSeq.get() / chPara.get() << endl;
}

/***
 * Print all prime number found with parallel method
 * @param primeNumbersPar
 */
void primeNbDisplay(const vector<mpz_class> &primeNumbersPar) {
    for (const mpz_class &prime : primeNumbersPar) {
        cout << prime << endl;
    }
    cout << endl;
}

/**
 * Display number of primeNumber found in Sequential and Parallel method
 * @param primeNumbersSeq
 * @param primeNumbersPar
 */
void primeNbStatDisplay(const vector<mpz_class> &primeNumbersSeq, const vector<mpz_class> &primeNumbersPar) {
    cout << "\n--- Prime Numbers Stat ---" << endl;
    cout << primeNumbersSeq.size() << " Prime number found with sequential method" << endl;
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
 * Handle user input
 * @param argc
 * @param argv
 * @param THREAD_NUMBER
 * @param FILEPATH
 */
void setupInputs(int argc, char **argv, size_t &THREAD_NUMBER, char *&FILEPATH) {
    if (argc < 3) {
        cout << "One argument is lacking" << endl;
        cout << "Please enter something like :" << endl;
        cout << "./pp_tp2 <thread_number> <filepath>" << endl;
        exit(0);
    } else {
        try {
            THREAD_NUMBER = stoi(argv[1]);
            FILEPATH = argv[2];
        } catch (invalid_argument i) {
            try {
                THREAD_NUMBER = stoi(argv[2]);
                FILEPATH = argv[1];
            } catch (invalid_argument ii) {
                cout << "Please enter something like :" << endl;
                cout << "./pp_tp2 <thread_number> <filepath>" << endl;
                exit(0);
            }
        }
    }
}

/***
 * Print user input
 * @param intervals
 * @param threadNb
 */
void inputPrintDisplay(const vector<tuple<mpz_class, mpz_class>> &intervals, int threadNb) {
    cout << "--- Optimized Interval ---" << endl;
    cout << intervals.size() << " Intervals handled during prime computing" << endl;

    cout << "\n--- Threads Number --- " << endl;
    cout << threadNb << endl << endl;
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

/***
 * Help corrector to debug and visualize threads and intervals
 * @param splitVector
 */
void printIntervalForEachThread(vector<vector<tuple<mpz_class, mpz_class>>> &splitVector) {
    int i = 0;
    for (vector<tuple<mpz_class, mpz_class>> v : splitVector) {
        cout << " --- Thread : " << i << " ---" << endl;
        FileParse::printTupleVector(v);
        i++;
    }
}

/***
 * Display Execution time for each thread
 * @param m_chronoThread
 */
void threadTimeDisplay(vector<double> &m_chronoThread) {
    cout << "--- Threads Time Detail ---" << endl;

    int ind = 0;
    double totalTime = accumulate(m_chronoThread.begin(), m_chronoThread.end(), 0.0);

    cout << "Total accumulated time : " << totalTime << endl;
    for (double d : m_chronoThread) {
        cout << "Thread " << ind << " worked for " << d << " (s) - " << (d / totalTime) * 100 << "%" << endl;
        ind++;
    }
}

/***
 * Merge MasterVectorTime and SlaveVectorTime without copy, for threadTimeDisplay()
 * @param m_chronoThread
 * @param start
 * @param s_chronoThread
 * @param end
 */
void
mergeMasterAndSlaveTimeVector(vector<double> &m_chronoThread, double start, vector<double> &s_chronoThread,
                              double end) {
    s_chronoThread.emplace_back(end - start);
    m_chronoThread.insert(m_chronoThread.end(),
                          make_move_iterator(s_chronoThread.begin()),
                          make_move_iterator(s_chronoThread.end()));
}

/***
 * Merge MasterVector and SlaveVector without copy
 * @param m_chronoThread
 * @param start
 * @param s_chronoThread
 * @param end
 */
void mergeMasterAndSlaveVector(vector<mpz_class> &m_primeNumbersPar, vector<vector<mpz_class>> &s_primeNumbersPar) {
    for (vector<mpz_class> vec : s_primeNumbersPar) {
        m_primeNumbersPar.insert(m_primeNumbersPar.end(),
                                 make_move_iterator(vec.begin()),
                                 make_move_iterator(vec.end())
        );
    }
}


