#include <iostream>
#include <string>
#include <unordered_map>
#include "llvm/IR/Instruction.h"

using namespace llvm;
class inst {
    public:
        int getidx() const {
            return idx;
        }
        std::string gethead() const {
            return head;
        }
        std::string getfunc() const {
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
        int getcolnum() const {
            return colnum;
        }
        std::vector<inst>::iterator getend(){
            return insts.end();
        }
        int printinsts() const {
            for (auto &i : insts) std::cout << i.gethead() << "\t" << i.getidx() << "\n";
            return 0;
        }
        void push(const inst &i) { insts.push_back(i); }
        std::list<std::vector<inst>::iterator> searchidx(int n); //n is idx in rightIR block
        col(int n)
        : colnum(n){}

    private:
        std::vector<inst> insts;
        int colnum;
};



class collist {
    public:
        void push(col *c) {
            int key = c->getcolnum();
            if (cols.find(key) == cols.end()) {
                cols[key] = c;
            }
        }
        col* searchcol(int colnum) {
            auto it = cols.find(colnum);
            return (it != cols.end()) ? it->second : nullptr;
        }
        int printcols() const {
            for (const auto &p : cols) std::cout << p.first << "\t";
            std::cout << "\n";
            return 0;
        }
        collist() = default;
 
    private:
        std::unordered_map<int, col*> cols;
};
