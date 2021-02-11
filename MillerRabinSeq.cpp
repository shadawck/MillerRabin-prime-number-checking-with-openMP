#include "MillerRabinSeq.hpp"
#include <gmpxx.h>

using namespace std;

int REPETITION_NUMBER = 10;

/***
 * Sequential computing of prime number.
 * @param intervals
 * @return
 */
vector<mpz_class> MillerRabinSeq::computePrime(const vector<tuple<mpz_class, mpz_class>> &intervals) {
    vector<mpz_class> primeNumbers;
    int primeProbability;

    for (tuple<mpz_class, mpz_class> tup : intervals) {
        mpz_class borneMin = get<0>(tup);
        mpz_class borneMax = get<1>(tup);

        mpz_class borneMinTemp = checkStart(primeNumbers, borneMin);
        borneMin = checkOddity(borneMinTemp);

        /// Prime number can't be even, so iterate just through odd number of the interval
        for (; borneMin <= borneMax; borneMin = borneMin + 2) {
            primeProbability = mpz_probab_prime_p(borneMin.get_mpz_t(), REPETITION_NUMBER);
            if (primeProbability > 0) {
                primeNumbers.emplace_back(borneMin);
            }
        }
    }

    return primeNumbers;
}

/***
 * 0 and 1 aren't prime nb, but 2 is one.
 * @param primeNumbers
 * @param borneMin
 * @return
 */
mpz_class &MillerRabinSeq::checkStart(vector<mpz_class> &primeNumbers, mpz_class &borneMin) {
    if (borneMin <= 2) {
        borneMin = 2;
        primeNumbers.emplace_back(2);
    }
    return borneMin;
}

/***
 * PrimeNumber can't be even number so we just start with and odd number and iterate 2 by 2
 * @param borneMin
 * @return
 */
mpz_class &MillerRabinSeq::checkOddity(mpz_class &borneMin) {
    if (borneMin % 2 == 0)
        borneMin++;
    return borneMin;
}
