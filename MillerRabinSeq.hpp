#ifndef PP_TP1_MILLERRABINSEQ_HPP
#define PP_TP1_MILLERRABINSEQ_HPP

#include <string>
#include <vector>
#include <gmpxx.h>

using namespace std;

class MillerRabinSeq {
public:
    static vector<mpz_class> computePrime(const vector<tuple<mpz_class, mpz_class>> &intervals);

    static mpz_class &checkOddity(mpz_class &borneMin);

    static mpz_class &checkStart(vector<mpz_class> &primeNumbers, mpz_class &borneMin);
};

#endif //PP_TP1_MILLERRABINSEQ_HPP
