#ifndef PP_TP1_ARGPARSE_HPP
#define PP_TP1_ARGPARSE_HPP

class ArgParse {
public:
    ArgParse(char *arg);

    int getParseInt() const;

private:
    int parseInt;
};

#endif //PP_TP1_ARGPARSE_HPP
