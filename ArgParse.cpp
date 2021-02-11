#include <iostream>

#include "ArgParse.hpp"

ArgParse::ArgParse(char *arg) {
    char *endPtr;
    int res;

    res = strtol(arg, &endPtr, 10);

    this->parseInt = res;
}

int ArgParse::getParseInt() const {
    return parseInt;
}
