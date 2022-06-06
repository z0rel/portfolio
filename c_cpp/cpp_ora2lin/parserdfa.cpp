
#if defined(_MSC_VER) && (_MSC_VER < 1400)
#pragma warning (disable : 4786)
#endif

#include "parserdfa.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "semantic_id.h"


using namespace std;
using namespace LexerDFA;
using namespace smart;
using namespace PlsqlHelper;

namespace LexerDFA {

AceptIdTable LexerDfa::acceptIdTable;

template < typename T >
inline void changeContainerSize(T & container, size_t size ) {
  if (container.size() < size)
    container.resize(size);
}

template < typename T >
inline void changeContainerSizeV( T & container, size_t size, typename T::value_type val ) {
  if (container.size() < size)
    container.resize(size, val);
}

template < typename T >
inline void changeContainerSize(T      & container,
                                size_t   sz1      ,
                                size_t   sz2      ) {
  changeContainerSize(container     , sz1 + 1);
  changeContainerSize(container[sz1], sz2 + 1);
}

template < typename T >
inline void changeContainerSize( T      & container,
                                 size_t   sz1      ,
                                 size_t   sz2      ,
                                 size_t   sz3      ) {
  changeContainerSize(container          , sz1 + 1);
  changeContainerSize(container[sz1]     , sz2 + 1);
  changeContainerSize(container[sz1][sz2], sz3 + 1);
}

LexerDfa::UcharTable::UcharTable() { memset(table, UCHAR_MAX, 0x100); }

AceptIdTable::AceptIdTable() {
  for (int i = 0; i < 0x100; ++i)
    state[i] = false;
  for (int i = 'A'; i <= 'Z'; ++i)
    state[i] = true;
  for (int i = 'a'; i <= 'z'; ++i)
    state[i] = true;
  for (int i = '0'; i <= '9'; ++i)
    state[i] = true;
  state[(unsigned int)((unsigned char)'_')] = true;
  state[(unsigned int)((unsigned char)'#')] = true;
  state[(unsigned int)((unsigned char)'$')] = true;

  for (int i = 0; i < 0x100; ++i)
    flags[i] = 0;
  flags[(unsigned int)((unsigned char)'#')] = FLAG_ID_HAS_SQUARE_SYMBOL;
  flags[(unsigned int)((unsigned char)'$')] = FLAG_ID_HAS_DOLLAR_SYMBOL;


  string specSigns = "?+-*/\\[]{}()%^:&;№@!|";
  for (string::iterator it = specSigns.begin(); it != specSigns.end(); ++it)
    flags[(unsigned int)((unsigned char)(*it))] = FLAG_ID_HAS_CP1251_SPEC_SYMBOLS;

  const int acceptedSpecIdSymbols[] = {
    0x80, 0x81, 0x83, 0x8A, 0x8C, 0x8E, 0x8D, 0x8F,
    0x90,             0x9A, 0x9C, 0x9E, 0x9D, 0x9F,
    0xA1, 0xA2, 0xA3, 0xA5, 0xA8, 0xAA, 0xAF,
    0xB2, 0xB3, 0xB4, 0xB8, 0xBA
  };

  for (unsigned int i = 0; i < SIZEOF_TABLE(acceptedSpecIdSymbols); ++i)
    flags[acceptedSpecIdSymbols[i]] = FLAG_ID_HAS_CP1251_SPEC_SYMBOLS;

  for (int i = 0xbc; i < 0x100; ++i)
    flags[i] = FLAG_ID_HAS_CP1251_SPEC_SYMBOLS;


  for (int i = 0; i < 0x100; ++i) {
    this->toupper[i] = ::toupper(i);
    this->isalpha[i] = ::isalpha(i);

    this->alphaUpper[i] = ::isalpha(i) ? ::toupper(i) : i;
  }
}

}


template < typename T, typename VALUE_T >
inline void changeContainerSizeV3(
    T     & container,
    size_t  sz1      ,
    size_t  sz2      ,
    size_t  sz3      ,
    VALUE_T val      ) {
  changeContainerSize ( container          , sz1 + 1      );
  changeContainerSize ( container[sz1]     , sz2 + 1      );
  changeContainerSizeV( container[sz1][sz2], sz3 + 1, val );
}

class SymClassEnum {
  static const unsigned int maxChar;

  template<typename T>
  inline static void addCharClass(set<char> &containerName, T *funcName) {
    for (unsigned int i = 0; i < 0x100; ++i)
      if ( funcName(i) ) {
        unsigned char x = i;
        char *c = (char*)&x;
        containerName.insert(*c);
      }
  }

  static void addAlnum (set<char> &result) { addCharClass(result, ::isalnum ); }
  static void addAlpha (set<char> &result) { addCharClass(result, ::isalpha ); }
  static void addCntrl (set<char> &result) { addCharClass(result, ::iscntrl ); }
  static void addDigit (set<char> &result) { addCharClass(result, ::isdigit ); }
  static void addGraph (set<char> &result) { addCharClass(result, ::isgraph ); }
  static void addLower (set<char> &result) { addCharClass(result, ::islower ); }
  static void addPrint (set<char> &result) { addCharClass(result, ::isprint ); }
  static void addPunct (set<char> &result) { addCharClass(result, ::ispunct ); }
  static void addSpace (set<char> &result) { addCharClass(result, ::isspace ); }
  static void addUpper (set<char> &result) { addCharClass(result, ::isupper ); }
  static void addXdigit(set<char> &result) { addCharClass(result, ::isxdigit); }

public:
  typedef void (*AddAction)( set<char> & result );
  map<string, AddAction> converterMap;

  SymClassEnum() {
    converterMap["alnum" ] = addAlnum;
    converterMap["alpha" ] = addAlpha;
    converterMap["cntrl" ] = addCntrl;
    converterMap["digit" ] = addDigit;
    converterMap["graph" ] = addGraph;
    converterMap["lower" ] = addLower;
    converterMap["print" ] = addPrint;
    converterMap["punct" ] = addPunct;
    converterMap["space" ] = addSpace;
    converterMap["upper" ] = addUpper;
    converterMap["xdigit"] = addXdigit;
  }
};

BranchNum Statistics::getNextBranchNum(unsigned int level, unsigned int size) {
  changeContainerSize(mappedIndicesLevel , level + 1);

  if (!size)
    return UINT_MAX;

  changeContainerSizeV(branchCounters, level + 1, 0);
  return branchCounters[level]++;
}

DfaTree::DfaTree(Statistics &_statistics) : statistics (_statistics) {}

DfaTree::DfaTree(char _c, Statistics &_statistics, unsigned int level, BranchNum branchNum, BranchNum parentBranchNum, unsigned short nextFlags, bool)
  : statistics(_statistics),
    c         (_c         ),
    branchNum (branchNum  ),
    level_    (level + 1  ),
    flags     (nextFlags  )
{
  changeContainerSize( statistics.mappedIndicesLevel , level           + 1 );
  changeContainerSize( statistics.mappedIndicesBranch, parentBranchNum + 1 );

  statistics.mappedIndicesLevel[level].insert(pair<char, unsigned char>(c, statistics.mappedIndicesLevel[level].size() ) );
  statistics.mappedIndicesBranch[parentBranchNum].insert(pair<char, unsigned char>(c, statistics.mappedIndicesBranch[parentBranchNum].size()));
}

void DfaTree::finalize( action_t       action   ,
                        unsigned short nextFlags) {
  this->action  = action   ;
  this->flags  |= nextFlags;
}

bool skipBrackets( const char *& str, size_t size, char cOpen, char cClose ) {
  if ( *str != cOpen )
    return false;

  unsigned int inBracket = 1;

  while ( size && inBracket ) {
    ++str;
    --size;

    if ( *str != '\\' && *(str + 1) == cOpen  )
      inBracket++; // TODO: BUG!!
    if ( *str != '\\' && *(str + 1) == cClose )
      inBracket--;
  }

  if ( inBracket || ( size < 1 ) )
    throw 1;

  str  += 2;

  return true;
}

inline bool stopPredicate(const char *str, const char *e1, const char *e2) {
  return (str == e1 || str == e2) ? true : false;
}

bool skipBrLoop(const char *&str, const char *e1, const char *e2) {
  if (stopPredicate(str, e1, e2))
    return false;

  while (skipBrackets(str, e1 - str, '[', ']')) {}

  if (stopPredicate(str, e1, e2))
    return false;

  return true;
}

void DfaTree::add( const char     * str       ,
                   size_t           size      ,
                   action_t         action    ,
                   unsigned short   nextFlags ,
                   unsigned int     level     ) {
  if ( size <= 0 )
    return;
  if (size == 1) {
    addSymbol( str, size, action, nextFlags, level );
    return;
  }

  string s( str, size );
  bool founded = true;

  while ( founded ) {
    founded = false;

    size_t       lenStrIt   = s.size () - 1;
    const char * beginStrIt = s.c_str()   ;
    const char * endStrIt   = beginStrIt + lenStrIt;
    const char * fullEndIt  = beginStrIt + lenStrIt + 1;
    const char * strIt      = beginStrIt     ;

    for ( ; skipBrLoop( strIt, endStrIt, fullEndIt ); ++strIt ) {

      if ( *strIt == '\\' && *(strIt + 1) == '{' ) {
        if (stopPredicate(++strIt, endStrIt, fullEndIt))
          break;
        if (stopPredicate(++strIt, endStrIt, fullEndIt))
          break;
        continue;
      }
      else if ( *strIt != '{' )
        continue;

      const char * start = strIt;

      skipBrackets( strIt, endStrIt - strIt, '{', '}');
      if ( start == strIt || (*(strIt - 1) != '}') )
        throw 1;

      size_t groupStrLen = strIt - start - 2;

      map<string, string>::const_iterator it =
        statistics.groups.find( (string(start + 1, groupStrLen) ) );
      if ( it != statistics.groups.end() ) {
        s.replace( start - beginStrIt, groupStrLen + 2, it->second.c_str(), it->second.length());
        founded = true;
        break;
      }
    }
  }
  addSymbol( s.c_str(), s.size(), action, nextFlags, level );
}


void DfaTree::addMayBeIgnorecase( char c, set<char> & result ) {

  if (ignorecase) {
    int _c = c;
    char c1 = toupper( _c );
    char c2 = tolower( _c );
    if ( c1 != c2 ) {
      result.insert(c1);
      result.insert(c2);
    }
    else
      result.insert( c );
  }
  else
    result.insert( c );
}


void DfaTree::getNextGroup( const char     *& str         ,
                            size_t          & size        ,
                            unsigned short  & futureFlags ,
                            bool            & unneed      ,
                            set<char>       & result ) {
  static const SymClassEnum symClassConverter;

  if ( size < 2)
    return;
  if ( *str == '\\' && *(str + 1) == '[' ) {
    ++str ; --size;
    return;
  }
  if ( *str != '[' )
    return;

  if ( *(str + 1) == ']' )
    throw 1;

  vector<int> rangePos;

  unsigned int len = 0;
  unsigned int endLen = size - 1;
  const char * begGroup = ++str;

  if ( *str == '[' && *(str + 1) == ':') {
    for ( ; len < endLen && *str == '[' ; ++len, ++str  ) {
      if ( *(str + 1) == ':' ) {
        len += 2;
        str += 2;
        const char * begIt = str;
        for ( ; len < endLen && (*str != ':' || *(str + 1) != ']'); ++len, ++str  ) { }
        if ( begIt == str )
          throw 1;
        string symclass( begIt, str - begIt );
        map<string, SymClassEnum::AddAction>::const_iterator cit =
            symClassConverter.converterMap.find(symclass);
        if ( cit == symClassConverter.converterMap.end() )
          throw 1;
        (*(cit->second))(result);
      }
      else {
        ++len; ++str;
        const char * begIt = str;
        for ( ; len < endLen && (*str == '\\' || *(str + 1) != ']' ); ++len, ++str  ) {
          if ( *str != '\\' && *(str + 1) == '-' && (len + 2) < endLen ) {
            rangePos.push_back(len);
            str += 2;
            len += 2;
          }
          else
            addMayBeIgnorecase(*str, result);
        }
        if ( begIt == str )
          throw 1;
      }
    }
  }
  else {
    for ( ; len < endLen && (*str == '\\' || *(str + 1) != ']' ) ; ++len, ++str ) {
      // TODO: check (len + 2) < endLen
      if ( ( *str == '\\' && ( *(str + 1) == '[' || *(str + 1) == ']' ) ) ) {
        ++len, ++str;
      }

      if ( *str != '\\' && *(str + 1) == '-' && (len + 2) < endLen ) {
        rangePos.push_back(len);
        str += 2;
        len += 2;
      }
      else
        addMayBeIgnorecase(*str, result);
    }
    addMayBeIgnorecase(*str, result);
  }
  ++len;

  ++str;
  ++str;
  size -= ((str - begGroup) + 1);

  if ( size ) {
    if      ( *str == '+' ) futureFlags |= MULTIGROUP    ;
    else if ( *str == '*' ) { unneed = true; futureFlags |= MULTIGROUP;}
    else if ( *str == '?' ) { unneed = true; }
    if ( futureFlags || unneed ) {
      ++str;
      --size;
    }
  }

  for ( vector<int>::iterator it = rangePos.begin(); it != rangePos.end(); ++it ) {
    unsigned char start = *(unsigned char*)(begGroup + *it);
    unsigned char end   = *(unsigned char*)(begGroup + *it + 2);
    if ( start > end )
      continue;
    result.insert( *(char*)&end );
    for ( unsigned char i = start; i < end; ++i )
      result.insert( *(char*)&i );
  }
}

bool DfaTree::addGroup( const char    *& str        ,
                        size_t         & size       ,
                        action_t         action     ,
                        unsigned short   nextFlags  ,
                        unsigned int     level ) {
  bool unneed = false;
  unsigned short futureFlags = 0;
  pair< map<char, Ptr<DfaTree> >::iterator, bool > ret;
  set<char> group;

  getNextGroup( str, size, futureFlags, unneed, group );

  bool selfIcase = false;
  if ( group.empty() && ignorecase ) {
    addMayBeIgnorecase( *str, group );
    if ( group.size() > 1 ) {
      ++str;
      --size;
    }
    selfIcase = true;
  }
  else if ( group.empty() || ( group.size() == 1 && selfIcase)  )
    return false;

  unsigned short groupFlags = nextFlags;
  bool finalizeGroup = false;
  // Финализация каждого символа группы, если нужен постпросмотр
  if ( size > 1 ) {
    if ( *str == '\\' && *(str + 1) == '/' ) {
      ++str ;
      --size;
    }
    else if ( *str == '/' ) {
      groupFlags |= PREVIEW_FINALLY;
      finalizeGroup = true;
      //if ( !selfIcase )
      if (unneed)
        this->finalize( action, flags /*| PREVIEW_FINALLY*/ ); // TODO: НАХ PREVIEW_FINALLY
      ++str ;
      --size;
      nextFlags |= POSTPREVIEW;
    }
  }

  finalizeGroup = finalizeGroup || !size;

  // TODO: допилить объединение ветвей нижних уровней для группы (и учесть необязательность группы)
  vector< Ptr<DfaTree> > addedNodes;
  for ( set<char>::iterator it = group.begin(); it != group.end(); ++it ) {
    // найти символ в мэпе:
    ret = nextStates.insert( pair< char, Ptr<DfaTree> >(*it, Ptr<DfaTree>() ) );
    if ( ret.second ) {
      ret.first->second = new DfaTree( *it                                      ,
                                       statistics                               ,
                                       level                                    ,
                                       statistics.getNextBranchNum(level, size) ,
                                       branchNum                                ,
                                       groupFlags                               ,
                                       ignorecase                               );
      ret.first->second->groupId = ++statistics.groupsCounter;
      if ( finalizeGroup )
         ret.first->second->finalize( action, groupFlags | FINALLY | futureFlags );
      else
        ret.first->second->flags = futureFlags;

      if ( !size )
        ret.first->second->addSymbol( str, size, action, nextFlags, level + 1 );
      else
        addedNodes.push_back(ret.first->second);
    }
    else { // символ уже существует
      ret.first->second->updateBranchNum(size - 1, level, statistics);
      if ( finalizeGroup )
         ret.first->second->finalize( action, groupFlags | FINALLY | futureFlags );
      ret.first->second->addSymbol(str, size, action, nextFlags, level + 1);
    }
  }

  if ( addedNodes.size() ) {
    vector< Ptr<DfaTree> >::iterator itAddedNodes = addedNodes.begin();

    (*itAddedNodes)->addSymbol( str, size, action, nextFlags, level + 1 );
    map< char, smart::Ptr< DfaTree > > & states = (*itAddedNodes)->nextStates;

    for ( itAddedNodes++; itAddedNodes != addedNodes.end(); ++itAddedNodes ) {
      for ( map< char, smart::Ptr< DfaTree > >::iterator it = states.begin();
            it != states.end(); ++it )
           (*itAddedNodes)->nextStates.insert( *it );
    }
  }

  if ( !unneed )
    return true;

  // Поднятие графа, если символ группы не обязателен ( uneed = true )
  ret = nextStates.insert(std::pair<char, Ptr<DfaTree> >(*str, Ptr<DfaTree>() ));
  if ( ret.second ) {
    ret.first->second = new DfaTree( *str                                         ,
                                     statistics                                   ,
                                     level                                        ,
                                     statistics.getNextBranchNum(level, size - 1) ,
                                     branchNum                                    ,
                                     nextFlags                                    ,
                                     ignorecase                                   );
    if ( size ) {
      ret.first->second->updateBranchNum(size - 1, level, statistics);
      ret.first->second->addSymbol( str + 1, size - 1, action, nextFlags, level + 1 );
    }
    else {
      ret.first->second->finalize ( action, nextFlags | FINALLY );
    }
  }
  else { // символ уже существует
    if ( size ) {
      ret.first->second->updateBranchNum(size - 1, level, statistics);
      ret.first->second->addSymbol(str + 1, size - 1, action, nextFlags, level + 1);
    }
    else
      ret.first->second->finalize ( action, nextFlags ); // TODO: ?? nextFlags | FINALLY
  }
  return true;
}

void DfaTree::updateBranchNum(unsigned int size, unsigned int level, Statistics &statistics) {
  if (size && branchNum < UINT_MAX)
    statistics.getNextBranchNum(level, size);
}

void DfaTree::addSymbol( const char *   str        ,
                         size_t         size       ,
                         action_t       action     ,
                         unsigned short nextFlags  ,
                         unsigned int   level      ) {
  if ( !size ) { // Данное состояние - листовое.
    finalize( action, nextFlags | FINALLY );
    return;
  }
  else if (size > 1 ) {
    if ( *str == '\\' && *(str + 1) == '/' ) {
      ++str ;
      --size;
    }
    else if ( *str == '\\' && *(str + 1) == '$' ) {
      ++str ;
      --size;
    }
    else {
      if ( *str == '$' ) {
        this->finalize( action, flags | NEEDENDSTR );
        ++str ;
        --size;
      }
      if ( *str == '/' ) {
        this->finalize( action, flags | PREVIEW_FINALLY );
        ++str ;
        --size;
        nextFlags |= POSTPREVIEW;
      }
    }
  }

  if (addGroup(str, size, action, nextFlags, level))
    return;

  pair< map<char, Ptr<DfaTree> >::iterator, bool > ret;

  char _c = *str;
  // обработка негруппового символа.
  // Поиск активного символа среди уже имеющихся состояний
  ret = nextStates.insert( pair< char, Ptr<DfaTree> >( _c, Ptr<DfaTree>() ) );
  if ( ret.second ) { // Если символ среди имеющихся состояний - не найден - добавить его к имеющимся
    ret.first->second = new DfaTree( _c                                           ,
                                     statistics                                   ,
                                     level                                        ,
                                     statistics.getNextBranchNum(level, size) ,
                                     branchNum                                    ,
                                     nextFlags                                    ,
                                     ignorecase                                   );

    ret.first->second->addSymbol( str + 1, size - 1, action, nextFlags, level + 1 );
  }
  else { // среди возможных состояний символ найден => обработать поток отностительно него
    ret.first->second->updateBranchNum(size - 1, level, statistics);
    ret.first->second->addSymbol(str + 1, size - 1, action, nextFlags, level + 1);
  }
}


LexerDfa::LexerDfa(Keyword2TokenInitializerList keyword2TokenMap, RegexGroupInitializerList groups) {
  Statistics stat         ;
  DfaTree    dfaTree(stat);

  for (const RegexGroupInitializerList::value_type &v : groups)
    stat.groups[v.first] = v.second;
  for (const Keyword2TokenInitializerList::value_type &kw2TokIt : keyword2TokenMap) {
    dfaTree.add(kw2TokIt.first.c_str(), kw2TokIt.first.size(), kw2TokIt.second);
    token2keyword[kw2TokIt.second] = kw2TokIt.first;
  }

  build(stat, dfaTree);
}

LexerDfa::LexerDfa(const Keyword2TokenMap &keyword2TokenMap, RegexGroupMap groups) {
  Statistics stat         ;
  DfaTree    dfaTree(stat);

  for (const RegexGroupMap::value_type &v : groups)
    stat.groups[v.first] = v.second;
  for (const Keyword2TokenInitializerList::value_type &kw2TokIt : keyword2TokenMap) {
    dfaTree.add(kw2TokIt.first.c_str(), kw2TokIt.first.size(), kw2TokIt.second);
    token2keyword[kw2TokIt.second] = kw2TokIt.first;
  }

  build(stat, dfaTree);
}

LexerDfa::~LexerDfa() {}


void LexerDfa::build( Statistics & stat, DfaTree & tree ) {
  /*
   * Состояние парсера - содержит индекс перехода и флаги обхода (финальность, постпросмотр)
   * Если индекс перехода не задан - то состояние является завершающим.
   *
   * Индекс перехода - это упорядоченная пара (уровень, ветвь), причем
   *   Уровень задан неявно и равен уровню состояния + 1.
   *   Ветвь адресует набор допустимых состояний на следующем уровне.
   *     Набор допустимых состояний для ветви индексируется символами (которые
   *        тоже проиндексированы для уровня или ветви)
   *
   * Если индекс перехода не задан для состояния если ветвь = UINT_MAX
   *  ( т.е. состояние.ветвь = UINT_MAX )
   *
   * состояние = ТаблицаПереходов[уровень][ветвь][символ]
   *   Оно адресует набор допустимых состояний в позиции [уровень + 1][состояние.ветвь]
   *   Либо является завершающим если значение переменной "состояние.ветвь" равно UINT_MAX
   *
   * При этом могут быть   "финальные состояния" - с флагом FINALLY и без флага POSTPREVIEW
   *                     и "финальные с пердпросмотром" - c флагом FINALLY и PREVIEW_FINALLY
   * Состояния с флагами FINALLY и POSTPREVIEW - означают что принята ветвь с постпросмотом
   * Флаг POSTPREVIEW приоритетнее флага FINALLY.
   * Состояний с флагами POSTPREVIEW и PREVIEW_FINALLY установленными одновременно - быть не должно
   * Если после флага PREVIEW_FINALLY встретилось состояние с флагом FINALLY и без флага POSTPREVIEW
   * то оно будет приоритетнее предыдущих
   *
   * <уровни>   <ветви>   <символы>              <состояния>
   * [0]     -> [0    ] -> [0,..,.., ... , N1] = [{B0_0,F0_0},{B0_1,F0_1},...{B0_N1,F0_N1}]
   * [1]     -> [0    ] -> [0,..,.., ... , N2] = [{B0_0,F0_0},{B0_1,F0_1},...{B0_N2,F0_N2}]
   * [1]     -> [1    ] -> [0,..,.., ... , N2] = [{B1_0,F1_0},{B1_1,F1_1},...{B1_N2,F1_N2}]
   * ...
   * [1]     -> [B0_N1] -> [0,..,.., ... , N2] = [{B1_0,F1_0},{B1_1,F1_1},...{B1_N2,F1_N2}]
  */


  mappedCharTable.resize(stat.mappedIndices.size());

  vector< map<char, unsigned char> >::iterator itIdxMap   = stat.mappedIndices.begin();
  vector< UcharTable >::iterator               itIdxTable = mappedCharTable   .begin();

  for ( ; itIdxMap != stat.mappedIndices.end(); ++itIdxMap, ++itIdxTable ) {
    for ( map<char, unsigned char>::iterator itIdx = itIdxMap->begin();
          itIdx != itIdxMap->end(); ++itIdx )
      itIdxTable->insert(*(unsigned char *)&(itIdx->first), itIdx->second);
  }

  makeTransition( tree );
}

#define ACCES_TO_3D_CONTAINER( name, i1, i2, i3 ) (name)[i1][i2][i3]
#define ACCES_TO_2D_CONTAINER( name, i1, i2     ) (name)[i1][i2]
#define ACCES_TO_1D_CONTAINER( name, i1         ) (name)[i1]

Statistics::Statistics () : groupsCounter(0), mappedIndices(mappedIndicesLevel ) {}
#define MAP_CH_IDX(level, bnum) level

void LexerDfa::makeTransition( DfaTree & tree, unsigned int level ) {
  map<char, Ptr<DfaTree> >::iterator it    = tree.nextStates.begin();
  map<char, Ptr<DfaTree> >::iterator itEnd = tree.nextStates.end  ();

  for ( ; it != itEnd; ++it ) {
    BranchNum      bnum     = tree.branchNum;
    unsigned int   chCode   = *(unsigned char *)&(it->first);
    unsigned int   mappedCh = ACCES_TO_1D_CONTAINER(mappedCharTable, MAP_CH_IDX(level, bnum) )[chCode];
    unsigned short flags    = 0;
    if ( it->second->action && !( it->second->flags & PREVIEW_FINALLY ) )
      flags |= FINALLY; /* флаг финальности с проверкой постпросмотра */
    flags   |= it->second->flags;

    changeContainerSize( transitionCube, level, bnum, mappedCh );

    ACCES_TO_3D_CONTAINER(transitionCube, level, bnum, mappedCh)
        = State(it->second->branchNum, flags, it->second->groupId, it->second->c);

    if (it->second->flags & (FINALLY | PREVIEW_FINALLY) && !(it->second->flags & POSTPREVIEW)) {
      changeContainerSizeV3( finalStates, level, bnum, mappedCh, (action_t)0 );
      ACCES_TO_3D_CONTAINER(finalStates, level, bnum, mappedCh) = it->second->action;
    }
    if (bnum < UINT_MAX)
      makeTransition( *it->second, level + 1);
  }
}


size_t LexerDfa::size( bool needPrint ) const {
  size_t res1 = 0, res2 = 0, res3 = 0;

  res1 = sizeof(vector< UcharTable >) + mappedCharTable.size() * sizeof(UcharTable);

  for ( vector< vector< vector<State> > >::const_iterator itTr1 =
        transitionCube.begin(); itTr1 != transitionCube.end(); ++itTr1 ) {
    for ( vector< vector<State> >::const_iterator itTr2 = itTr1->begin();
          itTr2 != itTr1->end(); ++itTr2 )
      res2 += itTr2->size() * sizeof(State);
    res2 += sizeof ( vector<State> ) * itTr1->size();
  }
  res2 += sizeof( vector< vector< vector<State> > > ) + sizeof ( vector< vector<State> > ) * transitionCube.size();

  for ( vector< vector< vector<action_t> > >::const_iterator itFs1 = finalStates.begin();
        itFs1 != finalStates.end(); ++itFs1 ) {
    for ( vector< vector<action_t> >::const_iterator itFs2 = itFs1->begin();
          itFs2 != itFs1->end(); ++itFs2 )
      res3 += itFs2->size() * sizeof(action_t);
    res3 += sizeof( vector<action_t> ) * itFs1->size();
  }
  res3 += sizeof( vector< vector< vector<action_t> > > ) + sizeof( vector< vector<action_t> > ) * finalStates.size();

  if ( needPrint )
    cout << "size of mappedCharTable  : " << res1 << endl
         << "size of transitionCube   : " << res2 << endl
         << "size of finalStatesTable : " << res3 << endl
         << "summary size             : " << ( res1 + res2 + res3 ) << endl;

  return res1 + res2 + res3;
}


void LexerDfa::printDfaCharTable() {
  unsigned int i;
  cout << "    ";
  for (i = 0; i < mappedCharTable.size(); ++i)
    cout << setw(5) << i;
  cout << endl;

  for ( i = 0; i < 0x100; ++i) {
    cout << setw(4);
    if ( isprint(i) )
      cout << (char)i;
    else
      cout << ("\\" + toString(i) );

    for ( unsigned int j = 0; j < mappedCharTable.size(); ++j )
      cout << hex << setw(5) << (unsigned int)(mappedCharTable[j][i]) ;
    cout << endl;
  }
}

void LexerDfa::printTransitionLay( unsigned int level ) {
  for ( unsigned int i = 0; i < transitionCube[level].size(); ++i ) {
    cout << "br " << dec << setw(2) << i << ": ";
    for ( unsigned int j = 0; j < transitionCube[level][i].size(); ++j ) {
      cout << hex << setw(6) << transitionCube[level][i][j].nextBranch
           << ":" << hex << setw(4) << transitionCube[level][i][j].flags;
    }
    cout << endl;
  }
  cout << endl;
}




