#include <list>
#include <vector>
#include <string>
#include "table.h"

std::list<std::vector<inst>::iterator> col::searchidx(int n) {
    std::list<std::vector<inst>::iterator> its;
    for (auto it = insts.begin(); it != insts.end(); ++it) {
        if (it->getidx() == n) its.push_back(it);
    }
    return its;
}