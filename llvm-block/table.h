#include <iostream>
#include <string>
#include <list>
#include <vector>
#include "llvm/IR/Instruction.h"

using namespace llvm;
class inst {
    public:
        int getidx() {
            return idx;
        }
        std::string gethead(){
            return head;
        }
        std::string getfunc(){
            return func;
        }
        inst(int i, std::string label, std::string f) 
        : idx(i), head(label), func(f) {}
      //  ~inst();
    private:
        int idx;
        std::string head;
        std::string func;
      ;
};

class col {
    public:
        int getcolnum() {
            return colnum;
        }
        std::vector<inst>::iterator getend(){
            return insts.end();
        }
        int printinsts() {
            std::vector<inst>::iterator it = insts.begin();
            while(it!=insts.end()){
                std::cout<<(*it).gethead()<<"\t"<<(*it).getidx()<<"\n";
                it++;}
                return 0;
        }
        void push(inst);
        std::list<std::vector<inst>::iterator> searchidx(int n); //n is idx in rightIR block
        col(int n)
        : colnum(n){}
     //   ~col();
    private:
        std::vector<inst> insts;
        int colnum;
};



class collist {
    public:
        void push(col *c); // cols에 col pointer push. 중복된건 안넣어야함
        col* searchcol(int colnum);
        int printcols(){
            std::list<col*>::iterator it = cols.begin();
            while(it!=cols.end()){
                std::cout<<(*it)->getcolnum()<<"\t";
                it++;
            }
            std::cout<<"\n";
            return 0;
        }
        collist() {}
      //  ~collist();
    private:
        std::list<col*> cols;
};
