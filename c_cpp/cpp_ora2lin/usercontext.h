#ifndef USERCONTEXT_H
#define USERCONTEXT_H

#include <ctime>
#include <unordered_map>
#include "app_conf.h"
#include "lwriter.h"
#include "hash_table.h"
#include "resolved_entity.h"
#include "semantic_flags.h"


using namespace std;
using namespace BackportHashMap;


/// Контекст пользователя
class  UserContext : public virtual smart::Smart, public Sm::ResolvedEntitySNode  {
protected:
  Sm::SemanticTree *toSTreeBase() const;
  virtual SmartVoidType* getThisPtr() const { return const_cast<SmartVoidType*>(static_cast<const SmartVoidType*>(this)); }
public:
  typedef Sm::Table            Table   ;
  typedef Sm::View             View    ;
  typedef Sm::Sequence         Sequence;
  typedef Sm::Synonym          Synonym ;
  typedef Sm::Function         Function;
  typedef Sm::Package          Package ;
  typedef Sm::Type::ObjectType ObjectType;
  typedef Sm::Trigger          Trigger ;
  typedef Sm::Index            Index   ;
  typedef Sm::DatabaseLink     DBlink  ;

  /// Таблицы как разница
  typedef map< string, std::pair<smart::Ptr<Table>, smart::Ptr<Table> > > DiffTables;

  /// Таблицы
  typedef map< string, smart::Ptr<Table>    >    Tables;
  /// Представления
  typedef map< string, smart::Ptr<View>     >    Views;
  /// Последовательности
  typedef map< string, smart::Ptr<Sequence> >    Sequences;
  /// Приватные синонимы
  typedef map< string, smart::Ptr<Synonym>  >    Synonyms;
  /// Автономные хранимые функции
  typedef map< string, smart::Ptr<Function> >    Functions;
  /// Пакеты
  typedef map< string, smart::Ptr<Package>  >    Packages;
  /// Типы, определяемые пользоваетелем
  typedef map< string, smart::Ptr<ObjectType > > Types;
  /// Триггеры БД. !!! Объект имеет собственное пр-во имен, и это нужно учитывать при резолвинге.
  typedef map< string, smart::Ptr<Trigger>  >    Triggers;
  /// Индексы.     !!! Объект имеет собственное пр-во имен, и это нужно учитывать при резолвинге.
  typedef map< string, smart::Ptr<Index>    >    Indices;
  /// Сслылки на внешние БД
  typedef map< string, smart::Ptr<DBlink>   >    DBLinks;

  vector<smart::Ptr<Sm::alter_user::UserSettings> > alterCommands;

  Synonyms  synonyms;
  Sequences sequences;
  Tables    tables;
  Views     views;
  Functions functions;
  Types     types;
  Packages  packages;
  Triggers  triggers;
  Indices   indices;
  DBLinks   dblinks;

  typedef std::map<Sm::CathegoriesOfDefinitions::ScopedEntities, std::map<std::string, smart::Ptr<Sm::Id> > > WrpEntities;
  WrpEntities wrpEntities;

  DiffTables diffTables;

  smart::Ptr<Sm::Id> username;
  smart::Ptr<Sm::Id> password;

  std::map<std::string, smart::Ptr<Sm::ResolvedEntity> > entityNamespace;

  mutable Sm::SemanticTree *initializerSemanticNode;

  UserContext(smart::Ptr<Sm::Id> _username, Sm::Id *_password = 0);
  UserContext();

  void replaceChildsIf(Sm::ExprTr tr);
  void replaceStatementsIf(Sm::StmtTr tr, Sm::StmtTrCond cond);
  void traverseModelStatements(Sm::StatementActor &fun);
  void traverseDeclarations(Sm::DeclActor tr);

  void storeSQL(LinterWriter &, ModelActions &, Sm::DependEntitiesMap &filter);
  void storeSQLTableKeys(LinterWriter &);
  void connectToUser(LinterWriter &) {}

  smart::Ptr<Sm::Datatype> getDatatype() const;
  void linterDefinition(Sm::Codestream &str);
  void grantResourceDefinition(Sm::Codestream &str);
  void linterReference(Sm::Codestream &str);

  void collectInitializers(Sm::UserEntitiesMap &container);

  smart::Ptr<Sm::Id> getName() const;

  bool getFieldRef(smart::Ptr<Sm::Id> &field);

  UserContext* userContext() const { return (UserContext*)this; }
  bool isDefinition() const { return true; }
  void resolve(ModelContext & model);

  void resolveName(ModelContext &model, const Sm::Id2 * name);
  bool resolveName(ModelContext &model, Sm::Id *name, bool maybePublic);
  void storeKeysActions(Sm::Codestream &str);

  Table *findTable(string & str);
  void initUserOnStream(Sm::Codestream &str);

  void printTablesWithCleanNumberFields(Sm::Codestream &str);
  void printTablesFieldsWithCleanNumberFields(Sm::Codestream &str);
  void printTablesFieldsWithBigNumberFields(Sm::Codestream &str);
  void calculateNumberPrecStatistic(std::map<string, int> &stat);
  void userInitializer(Sm::EntitiesSet &container, Sm::Codestream &str);

  void collectInitializedEntities(Sm::Definitions &defs);

  bool usedInQueryAndContainsFields() { return true; }
  virtual ~UserContext();

  ScopedEntities ddlCathegory() const { return User_; }
  virtual UserContext* toSelfUserContext() const { return const_cast<UserContext*>(this); }
};

namespace Sm {
  inline Codestream& operator<<(Codestream& s, UserContext &obj) { return obj.translate(s); }
  inline Codestream& operator<<(Codestream& s, UserContext *obj) { return obj ? obj->translate(s) : s; }
};


#endif // USERCONTEXT_H
