#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>


#include "lexsubtokenizer.h"
#include "sentence_transformer.h"
#include "smart_lexer.h"
#include "sql_syntaxer_bison.h"
#define  YY_HEADER_EXPORT_START_CONDITIONS
#include "sql_lexer_lex.h"
#include "semantic_statements.h"
#include "semantic_expr.h"
#include "semantic_function.h"


extern LexInputData pLexInputData;
extern lex_push_state_t lex_push_state;
YY_BUFFER_STATE get_current_buffer(yyscan_t yyscanner);
extern Sm::LexSubtokenizer lexerSubtokenizer;

using namespace Sm;

namespace trim_space {

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
static inline std::string ltrimmed(std::string s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
static inline std::string rtrimmed(std::string s) {
    rtrim(s);
    return s;
}

// trim from both ends (copying)
static inline std::string trimmed(std::string s) {
    trim(s);
    return s;
}
}


//extern int yydebug;
void SentenceTransformer::parseBuffer() {
  yyscan_t scanner = nullptr;
//  yydebug = 1;

  lexerSubtokenizer.setOracleStage();

  yylex_init_extra(cntx_, &scanner);
  LexInputData oldLexBufFun = pLexInputData; // подмена функции лексера, читающей данные из файла на функцию, читающую из строки text

  SmartLexer::StrReader strReadFun(sentence);
  pLexInputData = strReadFun;

  lex_push_state(Sql, scanner);
  lex_push_state(Sql, scanner);


  yy_delete_buffer(get_current_buffer(scanner), scanner);
  YY_BUFFER_STATE curBuf = yy_create_buffer(0, 2097152, scanner);
  yy_switch_to_buffer(curBuf, scanner);

  Ptr<Sm::String> baseName = new Sm::String("<EMPTY>");
  globalCurrentFile = baseName.object();
  cntx_->addParsedFilename("<EMPTY>", baseName);

  globalCurrentFile = baseName.object();
  cntx_->currentFileName = baseName;

  yy::parser parserItem(cntx_, scanner, cntx_->currentFileName);
  parserItem.parse();

  pLexInputData = oldLexBufFun;

  yylex_destroy(scanner);
}

namespace sentence_transformer {

struct Token : yy::parser::token {
  yytokentype      tokId;
  cl::filelocation location;
};

class VectorizeConcat;

struct Chunck : public yy::parser::token {
  typedef vector<Token> Tokens;

  Ptr<PlExpr> expr;
  bool        isToChar = false;
  bool        isLiteral = false;
  string      quotedText;
  Tokens      tokens;
  std::set<yytokentype> setTokens;

  vector<Token> *leftTokens  = 0;
  vector<Token> *rightTokens = 0;
  Chunck *leftChunck  = 0;
  Chunck *rightChunck = 0;
  
  Chunck(Ptr<PlExpr> e, SmartLexer::StringLexer *lexer);

  string toString(bool inConcstructExpr, VectorizeConcat *ctx);
  bool inSelectFieldContext();
  bool isMainSelect();
  bool containsSelectTok() const { return setTokens.count(SELECT) || setTokens.count(SELECT_HINT); }
  bool containsNonSelectOpTok() const { return setTokens.count(INSERT) || setTokens.count(UPDATE) || setTokens.count(DELETE) || setTokens.count(MERGE); }
  bool containsNonInsertOpTok() const { return setTokens.count(SELECT) || setTokens.count(UPDATE) || setTokens.count(DELETE) || setTokens.count(MERGE); }
  bool containsSqlOpTok() const { return setTokens.count(SELECT) || setTokens.count(INSERT) || setTokens.count(UPDATE) || setTokens.count(DELETE) || setTokens.count(MERGE); }
  bool containsSelectKWTok() const { return setTokens.count(FROM) || setTokens.count(GROUP) || setTokens.count(ORDER); }
  bool leftContainsSelectTok() const;
  bool leftContainsNonSelectOpTok() const;
  bool isDynTableContext() const;
  bool isInsertIntoFieldSContext() const;
  bool leftContainsInsertInto() const;
  bool insertIntoFieldsBound() const;
  bool leftIsValuesContext() const;
  bool containsInsertInto() const { return setTokens.count(INSERT) && setTokens.count(INTO); }
  bool leftIsWhere() const;

  static const std::set<yytokentype> sqlOpTok;
  static const std::set<yytokentype> selectKwTok;
};

const std::set<Chunck::yytokentype> Chunck::sqlOpTok    = { Chunck::SELECT, Chunck::INSERT, Chunck::UPDATE, Chunck::DELETE, Chunck::MERGE };
const std::set<Chunck::yytokentype> Chunck::selectKwTok = { Chunck::FROM, Chunck::GROUP, Chunck::ORDER };


bool Chunck::leftIsWhere() const {
  if (leftChunck) {
    for (Tokens::reverse_iterator it = leftChunck->tokens.rbegin(); it != leftChunck->tokens.rend(); ++it) {
      if (sqlOpTok.count(it->tokId) || selectKwTok.count(it->tokId))
        return false;
      if (it->tokId == WHERE)
        return true;
    }
    return leftChunck->leftIsWhere();
  }
  return false;
}

bool Chunck::leftIsValuesContext() const {
  return leftChunck && !leftChunck->containsNonInsertOpTok() &&
         ((leftChunck->setTokens.count((yytokentype)'(') && leftChunck->setTokens.count(VALUES)) || leftChunck->leftIsValuesContext());
}

bool Chunck::insertIntoFieldsBound() const { return setTokens.count(VALUES) || containsNonInsertOpTok(); }

bool Chunck::leftContainsInsertInto() const {
  return leftChunck && !leftChunck->insertIntoFieldsBound() &&
         ((leftChunck->containsInsertInto()) || leftChunck->leftContainsInsertInto());
}

bool Chunck::isInsertIntoFieldSContext() const {
  if (insertIntoFieldsBound())
    return false;
  if (setTokens.count((yytokentype)'(') && leftContainsInsertInto())
    return true;
  return leftChunck && leftChunck->isInsertIntoFieldSContext();
}


bool Chunck::isDynTableContext() const {
  return (leftTokens && leftTokens->back().tokId == yy::parser::token::FROM) ||
         (leftChunck && leftChunck->setTokens.count(INSERT) && leftChunck->setTokens.count(INTO));
}

bool Chunck::leftContainsNonSelectOpTok() const {
  return leftChunck && (leftChunck->containsNonSelectOpTok() || leftChunck->leftContainsNonSelectOpTok());
}

bool Chunck::leftContainsSelectTok() const {
  return leftChunck && (leftChunck->containsSelectTok() || leftChunck->leftContainsSelectTok());
}

bool Chunck::isMainSelect() {
  if (containsSelectTok()) {
    if (leftContainsSelectTok())
      return false;
    if (containsNonSelectOpTok() || leftContainsNonSelectOpTok())
      return false;
    return true;
  }
  if (leftChunck)
    return leftChunck->isMainSelect();
  return false;
}


bool Chunck::inSelectFieldContext() {
  for (Tokens::reverse_iterator it = tokens.rbegin(); it != tokens.rend(); ++it) {
    switch (it->tokId) {
      case SELECT:
      case SELECT_HINT:
        return true;
      case FROM:
      case WHERE:
      case GROUP:
      case ORDER:
        return false;
      default:
        break;
    }
  }
  if (leftChunck)
    return leftChunck->inSelectFieldContext();
  return false;
}


class VectorizeConcat {
public:
  std::vector<Chunck> v;
  string assignEntity;
  string assignEntityUp;
  Ptr<SmartLexer::StringLexer> lexer = new SmartLexer::StringLexer(Sql);
  Sm::List<Sm::RefAbstract> *intoList_;


  bool isConcat = false;
  void vectorize(PlExpr *e);
  void setReferences();

  VectorizeConcat(PlExpr *e, Sm::List<Sm::RefAbstract> *_intoList)
    : intoList_(_intoList) { vectorize(e); }

  void transformConcat();

  void joinDstSrc(string &dst, const string &res);
  void addSemicolon(string &dstCExpr);

  string intoList() const;
};


string Chunck::toString(bool inConcstructExpr, VectorizeConcat *ctx) {
  stringstream str;
  if (!isLiteral && leftTokens && !leftTokens->empty()) {
    string exprStr = expr->getLLoc().text(syntaxerContext.transformSentence);
    if (isDynTableContext()) {
      str << "_dyn_table(" << exprStr << ")";
      return str.str();
    }
    else if (inSelectFieldContext() || isInsertIntoFieldSContext() ) {
      str << "_dyn_field(" << exprStr << ")";
      return str.str();
    }
    else if (leftIsValuesContext() || leftIsWhere()) {
      RefAbstract *ra;
      if ((ra = expr->toSelfRefAbstract()) && ra->refEntity() && ra->refEntity()->callArglist)
        str << "!?_dyn_expr(" << exprStr << ")";
      else
        str << "!?_dyn_field(" << exprStr << ")";
      return str.str();
    }
  }
  if (setTokens.count(FROM) && !inConcstructExpr) {
    if (isMainSelect()) {
      for (vector<Token>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
        if (it->tokId == FROM || quotedText.empty()) {
          if (it == tokens.begin())
            break;

          cl::filelocation l1 = expr->getLLoc();
          cl::filelocation l2 = expr->getLLoc();
          l1.loc = tokens.begin()->location.loc + prev(it)->location.loc;
          l2.loc = it->location.loc + tokens.back().location.loc;

          str << l1.text(quotedText)
              << " INTO " << ctx->intoList();

          string s = l2.text(quotedText);
          if (!s.empty() && s.front() != ' ')
            str << " ";
          str << s;
          return str.str();
        }
      }
      str << " INTO " << ctx->intoList() << " ";
    }
  }
  if (!quotedText.empty())
    str << quotedText;
  else if (expr)
    str << expr->getLLoc().text(syntaxerContext.transformSentence);

  return str.str();
}



void VectorizeConcat::vectorize(PlExpr *e) {
  if (AlgebraicCompound *c = e->toSelfAlgebraicCompound())
    if (c->op == algebraic_compound::CONCAT) {
      vectorize(c->lhs);
      vectorize(c->rhs);
      return;
    }
  v.push_back(Chunck(e, lexer.object()));
}


void VectorizeConcat::setReferences() {
  vector<Token> *prev = 0;
  Chunck *prevChunck = 0;
  for (std::vector<Chunck>::iterator it = v.begin(); it != v.end(); ++it) {
    if (prev) {
      it->leftTokens = prev;
      it->leftChunck = prevChunck;
    }
    prev       = &(it->tokens);
    prevChunck = &(*it);
  }

  prev       = 0;
  prevChunck = 0;
  for (std::vector<Chunck>::reverse_iterator it = v.rbegin(); it != v.rend(); ++it) {
    if (prev) {
      it->rightTokens = prev;
      it->rightChunck = prevChunck;
    }
    prev       = &(it->tokens);
    prevChunck = &(*it);
  }
}


void VectorizeConcat::joinDstSrc(string &dst, const string &res) {
  static const set<char> nospaceSet = { ' ', '(', ')' };
  if (dst.empty() || nospaceSet.count(dst.back()) || res.empty() || nospaceSet.count(res.front()) || res.front() == ',')
    dst += res;
  else {
    dst += " ";
    dst += res;
  }
}


void VectorizeConcat::addSemicolon(string &dstCExpr)
{
  if (!dstCExpr.empty() && dstCExpr.back() != ';')
    dstCExpr += ";";
}

string VectorizeConcat::intoList() const {
  if (!intoList_)
    return "<INTO_LIST>";
  else {
    cl::filelocation frontL = intoList_->front()->getLLoc();
    cl::filelocation backL   = intoList_->back()->getLLoc();
    frontL.loc = frontL.loc + backL.loc;
    return frontL.text(syntaxerContext.transformSentence);
  }
  return "";
}

void VectorizeConcat::transformConcat() {
  if (v.empty())
    return;

  string exprVarUp = v.front().expr->getLLoc().textUp(syntaxerContext.transformSentence);

  if (assignEntityUp == exprVarUp) {
    isConcat = true;
    v.erase(v.begin());
  }

  stringstream str;
  str << "_construct_expr {" << assignEntity;
  if (isConcat)
    str << ", concat";
  str << "}";

  string dstCExpr, dstSimple;
  for (Chunck &c : v)
    joinDstSrc(dstCExpr, c.toString(/*inConcstructExpr=*/true, this));
  for (Chunck &c : v)
    joinDstSrc(dstSimple, c.toString(/*inConcstructExpr=*/false, this));

  addSemicolon(dstCExpr);
  addSemicolon(dstSimple);

  cout << "python<<<<<\n";
  cout << "construct_expr_stmt = \"\"\"" << str.str() << " " << dstCExpr << "\"\"\"\n\n";
  cout << "unquoted_expr_stmt = \"\"\""  << dstSimple << "\"\"\"\n\n";
  cout << "assign_entity = \"\"\"" << assignEntity << "\"\"\"\n\n";
  cout << ">>>>>python";
}


void transformStatement(Ptr<Sm::StatementInterface> stmt, Sm::List<Sm::RefAbstract> *intoList, bool /*semi*/) {
  if (Sm::Assignment *a = stmt->toSelfAssignment()) {
    cl::filelocation lvLoc = a->lValue->getLLoc();
    // векторизовать конкатенацию
    VectorizeConcat v(a->assignedExpr.object(), intoList);
    v.assignEntity   = lvLoc.text(syntaxerContext.transformSentence);
    v.assignEntityUp = lvLoc.textUp(syntaxerContext.transformSentence);
    v.setReferences();
    // транслировать ее по заданным правилам
    v.transformConcat();
  }
}


void transformExpression(Ptr<Sm::PlExpr> stmt, Sm::List<Sm::RefAbstract> *intoList, bool /*semi*/){
  VectorizeConcat v(stmt.object(), intoList);
  v.setReferences();
  // транслировать ее по заданным правилам
  v.transformConcat();
}


void tryConcatListAndTransform(Ptr<Sm::BaseList<Sm::StatementInterface> > stmtInterfaceList) {
  typedef Sm::BaseList<Sm::StatementInterface> It;
  Ptr<AlgebraicCompound> comp;
//  for (It::iterator it = stmtInterfaceList->begin(); it != stmtInterfaceList->end(); ++it) {
//    i


//  }

}

void transformStmtList(Ptr<Sm::BaseList<Sm::StatementInterface> > stmtInterfaceList) {
  if (!stmtInterfaceList)
    return;
  if (stmtInterfaceList->size() == 1) {
    if (Sm::ExecuteImmediate *exec = stmtInterfaceList->front()->toSelfExecuteImmediate()) {
      Ptr<Sm::List<Sm::RefAbstract> > ref = new Sm::List<Sm::RefAbstract>();
      if (exec->intoVars)
        for (auto &v : *(exec->intoVars))
          ref->push_back(v.object());
      transformExpression(exec->execExpr, ref.object(), false);
      return;
    }
  }




}




Chunck::Chunck(Ptr<PlExpr> e, SmartLexer::StringLexer *lexer) : expr(e) {
  if (RefExpr *r = e->toSelfRefExpr()) {
    if (r->reference->size() == 1) {
      if (r->reference->entity()->squoted()) {
        isLiteral = true;
        quotedText = r->reference->entity()->getText();
        lexer->setLexString(quotedText);
        lexer->loc.initialize();

        yy::parser::token::yytokentype state;
        while ((state = lexer->lex())) {
          Token tok;
          tok.tokId    = state;
          tok.location = lexer->loc;
          tokens.push_back(tok);
          setTokens.insert(state);
        }
      }
      else if (r->refEntity()->toNormalizedString() == "TO_CHAR") {
        Ptr<CallArgList> l = r->refEntity()->callArglist;
        if (l && l->size() == 1) {
          Ptr<Sm::FunCallArg> arg = l->front();
          if (FunCallArgExpr *e = arg->toSelfFunCallArgExpr()) {
            isToChar = true;
            expr = e->_expr;
          }
        }
      }
    }
  }
}


}

void SentenceTransformer::transform() {

  cntx_->transformSentence =  "TOK_TRANSFORM_SENTENCE\n" + cntx_->transformSentence
      + "\nTOK_TRANSFORM_SENTENCE";
  sentence = cntx_->transformSentence;

  syntaxerContext.stage = SyntaxerContext::SYNTAX_ANALYZE;
  parseBuffer();
}
