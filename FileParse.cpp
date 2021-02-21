#include <fstream>
#include <tuple>
#include <iostream>

#include "FileParse.hpp"

using namespace std;

/***
 * Read file and store value as mpz_class type in a vector
 * @param FILENAME
 * @return vector of tuple
 */
vector<tuple<mpz_class, mpz_class>> FileParse::readFile(char *FILENAME) {
    vector<tuple<mpz_class, mpz_class>> intervalsMpz;
    fstream file(FILENAME);
    if(file.fail()){
        cout << "Couldn't open the file!" << endl;
        exit(0);
    }

    mpz_class xMpz, yMpz;
    while (file >> xMpz >> yMpz) {
        intervalsMpz.emplace_back(xMpz, yMpz);
    }
    fstream close(FILENAME);

    return intervalsMpz;
}

/***
 * Split Tuple in n even tuple (modulo remain)
 * @param tup
 * @param n
 * @return vector of tuple
 */
vector<tuple<mpz_class, mpz_class>> SplitTuple(tuple<mpz_class, mpz_class> tup, size_t n) {
    vector<tuple<mpz_class, mpz_class>> outTup;

    mpz_class tupSize = get<1>(tup) - get<0>(tup);
    size_t s = tupSize.get_ui();

    size_t length = s / n;
    size_t remain = s % n;

    size_t begin = 0;
    size_t end = 0;

    for (size_t i = 0; i < min(n, s); ++i) {
        end += (remain > 0) ? (length + !!(remain--)) : length;
        outTup.emplace_back(get<0>(tup) + begin, get<0>(tup) + end);
        begin = end + 1;
    }

    return outTup;
}

/***
 * Optimize interval by removing overlapping intervals
 * @param intervals
 * @param THREAD_NUMBER
 * @return vector of tuple
 */
vector<tuple<mpz_class, mpz_class>>
FileParse::intervalsOptimisation(vector<tuple<mpz_class, mpz_class>> intervals, size_t THREAD_NUMBER) {
    sort(intervals.begin(), intervals.end());
    int index = 0;
    for (size_t i = 0; i < intervals.size(); i++) {
        if (get<1>(intervals[index]) >= get<0>(intervals[i])) {
            get<1>(intervals[index]) = max(get<1>(intervals[index]), get<1>(intervals[i]));
            get<0>(intervals[index]) = min(get<0>(intervals[index]), get<0>(intervals[i]));
        } else {
            index++;
        }
    }
    intervals.erase(intervals.begin() + index + 1, intervals.end());

    if (intervals.size() < THREAD_NUMBER) {
        return adaptInterval(intervals, THREAD_NUMBER, intervals.size());
    }

    return intervals;
}

/***
 * Split Intervals according to number of thread
 * @param intervals
 * @param THREAD_NUMBER
 * @param intervalSize
 * @return vector of tuple
 */
vector<tuple<mpz_class, mpz_class>>
FileParse::adaptInterval(const vector<tuple<mpz_class, mpz_class>> &intervals, size_t THREAD_NUMBER,
                         size_t intervalSize) {
    vector<tuple<mpz_class, mpz_class>> adaptedInterval;

    for (size_t i = 0; i < intervalSize; i++) {
        for (const tuple<mpz_class, mpz_class> &tup : SplitTuple(intervals[i], THREAD_NUMBER))
            adaptedInterval.emplace_back(tup);
    }

    return adaptedInterval;
}

void FileParse::printTupleVector(const vector<tuple<mpz_class, mpz_class>> &intervals) {
    int ind = 0;
    for (const auto &i : intervals) {
        cout << "Interval " << ind << " : [" << get<0>(i) << ", " << get<1>(i) << "]" << endl;
        ind++;
    }
}