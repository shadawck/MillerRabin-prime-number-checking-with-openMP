# GIF-7104-TP-2

## Build

```sh
cmake /path/to/pp_tp1
make
```

## Run

```sh
./pp_tp2 -t <threadNumber> -f <file_with_interval>
```

Par exemple :

```sh
./pp_tp2 -t 8 -f ../sample/7_long.txt
```

## Nota Bene

Notre programme exécute les méthodes séquentielle ET parallèle donc dans le cas du fichier de test 8_test.txt le
programme va mettre un peu de temps à tourner : Environ 5 min pour le séquentielle et 80 secondes pour le parallèle (
avec 8 threads). Si vous voulez éviter d'attendre 5 min, il suffit juste de commenté le calcul du programme séquentiel;

```c++
/**
 * main.cpp ligne 176
* MillerRabin Sequential
*/
auto chSeq = Chrono(true);
vector<mpz_class> primeNumbersSeq = MillerRabinSeq::computePrime(INTERVALS);
chSeq.pause();
```

devient :

```c++
/**
 * main.cpp ligne 176
* MillerRabin Sequential
*/
auto chSeq = Chrono(true);
vector<mpz_class> primeNumbersSeq; //= MillerRabinSeq::computePrime(INTERVALS);
chSeq.pause();
```