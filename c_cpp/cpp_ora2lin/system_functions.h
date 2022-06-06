#ifndef SYSTEM_FUNCTIONS_H
#define SYSTEM_FUNCTIONS_H

#include <vector>
#include <list>
#include <string>
#include <map>
#include "smartptr.h"
#include "hash_table.h"
#include "model_head.h"

class ModelContext;
namespace Sm {


using namespace BackportHashMap;
using namespace smart;

class ResolvedEntity;
class Id;
class Function;
class Datatype;
class SqlExpr;
class SemanticTree;
using namespace std;

class FunctionCall;
struct FunCallArg;


template <typename T>
class List;

template <typename T>
class Vector;

typedef Vector<FunCallArg> CallArgList;

class Codestream;

typedef vector<Ptr<FunCallArg> > TemplateArglist;

class CallarglistTranslator;

class CreateSubstr;

//typedef Ptr<Function> (*CreateFunctionByArglist)(const string & name, vector<Ptr<Function> > &createdFunctions, TemplateArglist *arglist);
class OraTemplateData : public Smart {
public:
  typedef function<Datatype*(int, Datatype*, int, Datatype*, bool)> ConflictSelector;

  const string            funname;
  vector<Ptr<Function> >  createdFunctions;
  virtual Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) = 0;

  OraTemplateData(const string &_funname);

  Ptr<Function> addSysFun(Function *f, Sm::CallarglistTranslator *tr = 0);
  virtual ~OraTemplateData();
protected:
  void updateTemplateNamespace(Ptr<Function> fun);

  Ptr<Datatype> getMaximalArgumentDatatype(TemplateArglist &callArgList, bool pl, const ConflictSelector &conflictSelector = 0);
public:
  static void updateDatatypeIfGreather(int &dstPos, Datatype **dst, int &callArgPos, FunCallArg &callArg, bool pl, const ConflictSelector &conflictSelector = 0);
  virtual CreateSubstr *toSelfCreateSubstr() { return 0; }
protected:


  Ptr<Function> findCreatedFunByRettype(Datatype *t, bool pl, size_t arglistSize);


  Ptr<Function> createStandardSysFunction(TemplateArglist &callArgList, bool isPlContext, const std::initializer_list<string> &argnames, CallarglistTranslator *tr = 0);
  Ptr<Function> createStandardSysFunction(Ptr<Datatype> rettype, size_t callArglistSize, bool isPlContext, const std::initializer_list<string> &argnames, CallarglistTranslator *tr = 0);
  bool callArgListUnresolved(TemplateArglist &callArgList);
  bool callArglistLenNEQ (TemplateArglist &callArgList, unsigned int len);
  bool callArglistLenZGT (TemplateArglist &callArgList, unsigned int len);
  bool callArglistLenGTLT(TemplateArglist &callArgList, unsigned int len, unsigned int lowLen);
  Ptr<Datatype> getImplicitCastedOracleRettype(Ptr<Datatype> t1, Ptr<Datatype> t2, bool isPlContext, Ptr<CallarglistTranslator> &tr);

  bool isAdjustNum(TemplateArglist &callArgList);
  bool isAdjustVarchar(TemplateArglist &callArgList);
  bool isAdjustChar(TemplateArglist &callArgList);
  bool isAdjustDate(TemplateArglist &callArgList);

  bool isAdjustVarchar(TemplateArglist &callArgList, int pos);
  bool isAdjustCharVarchar(TemplateArglist &callArgList, int pos);
  bool isAdjustChar   (TemplateArglist &callArgList, int pos);
  bool isAdjustDate   (TemplateArglist &callArgList, int pos);
  bool isAdjustNumber (TemplateArglist &callArgList, int pos);
  bool isAdjustNum    (TemplateArglist &callArgList, int pos);
  bool isAdjustNull   (TemplateArglist &callArgList, int pos);
  bool isAdjustAnydata(TemplateArglist &callArgList, int pos);
  bool isAdjustBool   (TemplateArglist &callArgList, int pos);
  bool isAdjustClob   (TemplateArglist &callArgList, int pos);
  bool isAdjustBlob   (TemplateArglist &callArgList, int pos);

  typedef bool (Datatype::*Mf)() const;
  bool checkAdjust(TemplateArglist &callArgList, Mf mf);
  bool checkAdjust(TemplateArglist &callArgList, Mf mf, int pos);

  Ptr<Datatype> makeSubstrRettypeByFirstArg(Datatype* t, TemplateArglist &callArgList, unsigned int len);

};

class GlobalFunctions
{
public:
  typedef map<string, vector<Ptr<Function> > > SystemFunctionsMap;

  typedef map<string, Ptr<OraTemplateData> > OraTemplatesFunMap;
  typedef std::vector<Sm::SemanticTree*> SemanticNodes;

  SystemFunctionsMap systemFunctionsMap;
  OraTemplatesFunMap oraTemplatesFunMap;

  SemanticNodes semanticNodes;

  GlobalFunctions();
  ~GlobalFunctions();

  void addSpecialFun();

  Ptr<Function> getSubstr(bool pl, unsigned int len);

//  Function * substr_varchar = 0;
//  Function * substr_clob    = 0;
//  Function * substr_number  = 0;

  Function *add(Ptr<Function> fn);
//  void addNvl2(Ptr<Datatype> t);
//  void addRPad(string name, Ptr<Datatype> t);

  Ptr<Function> linterLenblob;
  Ptr<Function> linterGettext;
  Ptr<Function> linterToDate;
  string defaultDateFormat = "dd.mm.yyyy";

  void addLinterFunctions();

};


void trDbmsSessionSetContextName(Sm::Codestream &str, Ptr<Sm::CallArgList> args);
void trDbmsOperlogSetOperlogContext(Sm::Codestream &str, Ptr<Sm::CallArgList> args);
void trDbmsOperlogSetOperlogContextCallarglist(Ptr<Id> &argl, Codestream &str);

};

#endif // SYSTEM_FUNCTIONS_H
