#pragma once

#include <list>
#include <string>
#include <vector>

class inst {
    public:
        int getidx() const {
            return idx;
        }
        const std::string &gethead() const {
            return head;
        }
        const std::string &getfunc() const {
            return func;
        }
        inst(int i, std::string label, std::string f) 
        : idx(i), head(label), func(f) {}

    private:
        int idx;
        std::string head;
        std::string func;
};

class col {
    public:
        void push(inst);
        std::list<std::vector<inst>::iterator> searchidx(int n); //n is idx in rightIR block

    private:
        std::vector<inst> insts;
};
