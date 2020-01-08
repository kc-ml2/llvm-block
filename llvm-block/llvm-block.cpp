#include <string>
#include <vector>
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

std::list<std::vector<inst>::iterator>::iterator findstring(
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
    last1=a.end(), last2=b.end();
    while (first1 != last1 && first2 != last2) {
      s1 = (**first1).gethead();
      s2 = (**first2).gethead();
      if(s1.at(0)!=c) {
        if (findstring(first2,last2,s1)!=last2) o.push_back(*first1);
        first1++;
        continue;
      }

      else if(s2.at(0)!=c) {
        if (findstring(first1,last1,s2)!=last1) o.push_back(*first2);
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

std::vector<collist*> CreateTable(std::string inputfile) {
  int n,m,colnum;
  char c('!');
  std::string line, word;
  std::ifstream input(inputfile);
  //  ofstream output(argv[2]);
  std::vector<collist*> table;

  while(getline(input, line)) { 
    std::stringstream iss(line);
    iss >> word;
    if (word.at(0) != c) continue;
    //first char: !
    iss >> word >> word;
    if ( word != "!DILocation(line:" ) continue;
    iss >> word; //linenum,
    n = atoi(word.substr(0,word.size()-1).c_str()); //linenum
    iss >> word >> word; //colnum,
    colnum = atoi(word.substr(0,word.size()-1).c_str()); //colnum

    if(table.size() <= n) {
      table.resize(n+1);
      table.at(n) = new collist();
      }
    else if(table.at(n) == NULL) table.at(n) = new collist();
    table.at(n) -> push(new col(colnum));
   
    
  }
  return table;
}

std::vector<collist*> InsertTable(std::vector<collist*> table
                                  , Module &M) {
  int i;
  DebugLoc DL;
  std::string label,func;
  for( auto &F : M) {
    func = F.getName().str();
    for ( auto &BB : F){
      label = getSimpleNodeLabel(&BB);
      i=0;
      for ( auto &I : BB) {
        i++;
        DL = I.getDebugLoc();
        inst objinst(i,label,func);
        if(!DL) break;
        table.at(DL.getLine())->searchcol(DL.getCol())->push(objinst); 
      }
    }
  }

  return table;
}


void CompareLR(std::vector<collist*> table, Module &M){
  std::string label1, label2,func;
  DebugLoc DL;
  int f=0, i;
  col* objcol;
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
        DL = I.getDebugLoc();
        if(!DL || DL.getLine()==0) break;
        objcol = table.at(DL.getLine())->searchcol(DL.getCol());
        itsa = objcol->searchidx(++i);
        if (itsa.empty()) continue;  
        if (!itsb.empty()) o = intersection(itsa,itsb);
        else o = itsa;
        itsb = o;
        if (o.empty()) continue;
        if (I.isTerminator() ) {
          it = o.front();
          while( !o.empty() &&
          heads.end()!=std::find(heads.begin(), heads.end(), (*it).gethead())) {
            o.pop_front();
            it = o.front();
          }
          if(o.empty()) continue;
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

  std::vector<collist*> table = CreateTable(LeftFilename);
  table = InsertTable(table, *LModule);
  CompareLR(table, *RModule);
  return 0;
}