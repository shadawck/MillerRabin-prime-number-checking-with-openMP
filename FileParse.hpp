#ifndef PP_TP1_FILEPARSE_HPP
#define PP_TP1_FILEPARSE_HPP

#include <tuple>
#include <vector>
#include <gmpxx.h>

using namespace std;

class FileParse {
public :
    static vector<tuple<mpz_class, mpz_class>> readFile(char *);

    static vector<tuple<mpz_class, mpz_class>>
    intervalsOptimisation(vector<tuple<mpz_class, mpz_class>> intervals, size_t THREAD_NUMBER);

    static void printTupleVector(const vector<tuple<mpz_class, mpz_class>> &intervals);

private :
    static vector<tuple<mpz_class, mpz_class>>
    adaptInterval(const vector<tuple<mpz_class, mpz_class>> &intervals, size_t THREAD_NUMBER, size_t intervalSize);


};

#endif //PP_TP1_FILEPARSE_HPP
