#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "semantic_utility.h"

namespace Sm {
class Codestream;
class Id;
class StatementInterface;


Sm::Codestream& translateIdReference(Sm::Codestream &str, Ptr<Sm::Id> id, bool &isNotFirst, CathegoriesOfDefinitions::ScopedEntities &prevCat, bool &userAlreadyOutput);
void translateQueryFields(Sm::Codestream &str, EntityFields &fields, bool expand, const string &parentStr = "", unsigned int flags = 0);
void translateQueryFieldsEscape(Sm::Codestream &str, Sm::EntityFields &fields, bool expand);
void translateQueryFieldsEscape(Sm::Codestream &str, Sm::ResolvedEntity *fromObject, bool expand);
void translateCursorAsSelectCollection(Sm::Codestream &str, const IdEntitySmart &reference, Sm::ResolvedEntity *collection, Sm::ResolvedEntity *cursor);
void translateCursorAsInsertCollection(Sm::Codestream &str, const IdEntitySmart &collEntity, Sm::ResolvedEntity *cursor, Sm::Variable *indexVar, int fieldIndex = -1);
void translateCursorAsInsertCollection(Sm::Codestream &str, const IdEntitySmart &collEntity, Sm::ResolvedEntity *cursor);
void translateAsLinterCursor(ResolvedEntity *query, Sm::Codestream &str, bool expand = true);
void translateAsLinterStruct(ResolvedEntity *query, Sm::Codestream &str, bool expand = false);
void translateFieldsAsCursor(EntityFields &fields, Sm::Codestream &str, bool expand = true);
void translateFieldsAsStruct(EntityFields &fields, Sm::Codestream &str, bool expand = false);
void translateMultilineText(Codestream& os, string &s);
void translateAsMakestrArg(Codestream& str, SemanticTree *snode, ResolvedEntity *d, ResolvedEntity *ref, bool isDirectContext);
void setTranslateSqlToStringFlags(Codestream &dst, Codestream &src, unsigned int flags = 0);
void wrapSqlCodestreamIntoString(TranslatorTailMakestr *translator, Codestream &dst, Codestream &sqlExprStr, unsigned int flags = 0);

void translateCallArglist(Codestream &str, ResolvedEntity *funDef, Ptr<Sm::CallArgList> callArglist);

#define SQLSTR_NEED_DIRECT                   (1 << 0)
#define SQLSTR_SUPRESS_DIRECT                (1 << 1)
#define SQLSTR_NEED_MAKESTR                  (1 << 2)
#define SQLSTR_EXPAND_SELECT                 (1 << 3)
#define SQLSTR_HAS_ADDITIONAL_IDENTIFICATORS (1 << 4)
#define SQLSTR_IS_EXECUTE_BLOCK              (1 << 5)


struct StatementsTranslator : public CathegoriesOfDefinitions {
  static void translateStatement(Codestream &str, StatementInterface *it, BlockPlSql* codeBlock = 0);
};


}



#endif // CODEGENERATOR_H
