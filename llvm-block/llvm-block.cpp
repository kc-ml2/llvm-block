#include <algorithm>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>
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
using Location = std::pair<unsigned, unsigned>;
using Table = std::map<Location, col>;

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

Table CreateTable(Module &M) {
  Table table;
  for( auto &F : M) {
    for ( auto &BB : F){
      int i=0;
      for ( auto &I : BB) {
        ++i;
        DebugLoc DL = I.getDebugLoc();
        if(!DL) continue;
        table[{DL.getLine(), DL.getCol()}].push(
            inst(i, getSimpleNodeLabel(&BB), F.getName().str()));
      }
    }
  }
  return table;
}

void CompareLR(Table &table, Module &M){
  std::string label1, label2,func;
  DebugLoc DL;
  int f=0, i;
  std::vector<inst>::iterator it;
  for( auto &F: M){
    f++;
    func = F.getName().str();
    errs() << "Function "<< f << ": " << func << "\n";
    std::list<std::string> heads;

    for( auto &BB : F){
      i=0;
      std::list<std::vector<inst>::iterator> itsa, itsb, o;
      for( auto &I :BB){
        ++i;
        DL = I.getDebugLoc();
        if(!DL || DL.getLine()==0) continue;
        auto location = table.find({DL.getLine(), DL.getCol()});
        if (location == table.end()) continue;
        itsa = location->second.searchidx(i);
        if (itsa.empty()) continue;  
        if (!itsb.empty()) o = intersection(itsa,itsb);
        else o = itsa;
        itsb = o;
        if (o.empty()) continue;
        if (I.isTerminator() ) {
          while (!o.empty() && heads.end() != std::find(
              heads.begin(), heads.end(), o.front()->gethead())) {
            o.pop_front();
          }
          if(o.empty()) continue;
          it = o.front();
          label1 = (*it).gethead();
          label2 = getSimpleNodeLabel(&BB);
          if ((*it).getfunc() != func) continue;
          errs() << label1 << " " << label2 << "\n";
          heads.push_back(label1);
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
  std::unique_ptr<Module> LModule = readModule(Context, LeftFilename);
  std::unique_ptr<Module> RModule = readModule(Context, RightFilename);
  if (!LModule || !RModule)
    return 1;

  Table table = CreateTable(*LModule);
  CompareLR(table, *RModule);
  return 0;
}
