#include <list>
#include <vector>
#include <string>
#include "table.h"

col* collist::searchcol(int colnum) {
    std::list<col*>::iterator it = cols.begin();
    int gc;
    while(it != cols.end()) {
        gc = (*it) -> getcolnum();
        if (gc > colnum) continue;
        else if (gc == colnum) return *it;
        else it++;
    }
    return NULL;
}



void collist::push(col *c) {
    int gc, colnum = c -> getcolnum();

    std::list<col*>::iterator it = cols.begin();
    while(it != cols.end()) {     
        gc = (*it) -> getcolnum();
        if (gc > colnum){
            cols.insert(it, c);
            return;
        }
        else if (gc == colnum) return;
        else it++;
    }
    cols.push_back(c);
    return;
}

void col::push(inst i) {
    insts.push_back(i);
}

std::list<std::vector<inst>::iterator> col::searchidx(int n) {
    std::list<std::vector<inst>::iterator> its;
    std::vector<inst>::iterator it = insts.begin();
    while(it != insts.end()) {
        if ((*it).getidx() == n) its.push_back(it);
        it++;
    }
    return its;
}