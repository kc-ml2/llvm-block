#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "table.h"

using namespace llvm;
using TableMap = std::unordered_map<int, collist*>;

static std::unique_ptr<Module> readModule(LLVMContext &Context,
                                          StringRef Name) {
  SMDiagnostic Diag;
  std::unique_ptr<Module> M = parseIRFile(Name, Diag, Context);
  if (!M)
    Diag.print("llvm-fixblock", errs());
  return M;
}

static cl::opt<std::string> LeftFilename(cl::Positional,
                                         cl::desc("<first file>"),
                                         cl::Required);
static cl::opt<std::string> RightFilename(cl::Positional,
                                          cl::desc("<second file>"),
                                          cl::Required);


static std::string getSimpleNodeLabel(const BasicBlock *Node) {
  if (!Node->getName().empty())
       return Node->getName().str();
 
     std::string Str;
     raw_string_ostream OS(Str);
 
     Node->printAsOperand(OS, false);
     return OS.str();
}

std::list<std::vector<inst>::iterator>::iterator findString(
            std::list<std::vector<inst>::iterator>::iterator first,
            std::list<std::vector<inst>::iterator>::iterator last,
            std::string s){
  while (first!=last) {
    if ((**first).gethead()==s) return first;
    ++first;
  }
  return last;
}

std::list<std::vector<inst>::iterator> intersection(
        std::list<std::vector<inst>::iterator> a,
        std::list<std::vector<inst>::iterator> b ) {
  std::string s1,s2;
  int n1,n2;
  char c('%');
  std::list<std::vector<inst>::iterator> o;
  std::list<std::vector<inst>::iterator>::iterator first1 = a.begin(),
                                                   first2 = b.begin(),
                                                   last1=a.end(),
                                                   last2=b.end();
  while (first1 != last1 && first2 != last2) {
    s1 = (**first1).gethead();
    s2 = (**first2).gethead();
    if(s1.at(0)!=c) {
      if (findString(first2,last2,s1)!=last2) o.push_back(*first1);
      first1++;
      continue;
    }
    
    else if(s2.at(0)!=c) {
      if (findString(first1,last1,s2)!=last1) o.push_back(*first2);
      first2++;
      continue;
    }
    
    if (s1 < s2) ++first1;
    else  {
      if (!(s2 < s1)) o.push_back(*first1++);
      ++first2;
    } 
  }
  return o;
}

TableMap CreateTable(std::string inputfile) {
  char c('!');
  std::string line, word;
  std::ifstream input(inputfile);

  TableMap table;

  while(getline(input, line)) { 
    std::stringstream iss(line);
    iss >> word;
    if (word.empty() || word.at(0) != c) continue;
    //first char: !
    iss >> word >> word;
    if ( word != "!DILocation(line:" ) continue;
    iss >> word; //linenum,
    int n = atoi(word.substr(0,word.size()-1).c_str()); //linenum
    iss >> word >> word; //colnum,
    int colnum = atoi(word.substr(0,word.size()-1).c_str()); //colnum
    
    // generate new collist if key does not exist in the map
    if(table.find(n) == table.end()) {
      table[n] = new collist();
    }
    table[n] -> push(new col(colnum));
  }
  return table;
}

TableMap InsertTable(TableMap table, Module &M) {
  col* c;
  DebugLoc DL;
  std::string label,func;
  for( auto &F : M) {
    func = F.getName().str();
    for ( auto &BB : F){
      label = getSimpleNodeLabel(&BB);
      int i=0;
      for ( auto &I : BB) {
        i++;
        DL = I.getDebugLoc();
        if (!DL) break;

        int line = DL.getLine();
        int col = DL.getCol();
        auto it = table.find(line);
        if (it == table.end()) continue;

        c = it->second->searchcol(col);
        if (!c) continue;

        c->push(inst(i, label, func));
      }
    }
  }

  return table;
}


void CompareLR(TableMap table, Module &M){
  col* objcol;
  DebugLoc DL;
  int f = 0;
  for( auto &F: M){
    errs() << "Function "<< ++f << ": " << F.getName() << "\n";
    std::list<std::string> heads;

    for( auto &BB : F){
      int i=0;
      std::list<std::vector<inst>::iterator> itsa, itsb, o;
      for( auto &I :BB){
        DL = I.getDebugLoc();
        if(!DL || DL.getLine()==0) break;
        int line = DL.getLine(), col = DL.getCol();
        auto it_map = table.find(line);
        if (it_map == table.end()) continue;
        objcol = it_map->second->searchcol(col);
        if (!objcol) continue;
        itsa = objcol->searchidx(++i);
        if (itsa.empty()) continue;  
        o = itsb.empty() ? itsa : intersection(itsa, itsb);
        itsb = o;
        if (o.empty()) continue;
        if (I.isTerminator() ) {
          auto it = o.front();
          while( !o.empty() &&
          heads.end()!=std::find(heads.begin(), heads.end(), (*it).gethead())) {
            o.pop_front();
            if (!o.empty()) it = o.front();
          }
          if(o.empty()) continue;
          if ((*it).getfunc() != F.getName()) continue;
          errs() << (*it).gethead() << " " << getSimpleNodeLabel(&BB) << "\n";
          heads.push_back((*it).gethead());
        }
      }
    }
    errs() <<"\n";
  }
  return;
}


int main(int argc, char **argv) {
  cl::ParseCommandLineOptions(argc, argv);
  LLVMContext Context;

  // Load both modules.  Die if that fails.
  auto LModule = readModule(Context, LeftFilename);
  auto RModule = readModule(Context, RightFilename);

  TableMap table = CreateTable(LeftFilename);
  table = InsertTable(table, *LModule);
  CompareLR(table, *RModule);
  return 0;
}