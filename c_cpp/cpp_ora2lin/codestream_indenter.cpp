#include <unordered_set>
#include "config_converter.h"
#include "semantic_id_lists.h"
#include "semantic_base.h"
#include "codestream_indenter.h"
#include "codespacer.h"
#include "codegenerator.h"

using namespace Sm;


int Sm::s::MultilineTextChunck::getLastLineLength(const std::string &text) {
  for (string::const_reverse_iterator rit = text.rbegin(); rit != text.rend(); ++rit)
    if (*rit == '\n')
      return std::distance(text.rbegin(), rit);
  return text.length();
}


void codestreamAssert(bool v) {
  if (v)
    throw 999;
}

void codestreamAssert2(bool ) {
//  if (v)
//    throw 999;
}

void IndentingContext::checkInvariant() const {
  if (tab_ < 0)
    throw 999;

//  if (tab_ > LINE_LENGTH_MAX || tab_ < 0 || (shared.sourceLineSize_ < absoluteColumnPos))
//    throw 999;
}



void IndentedLineContext::startLine(SpacerStream::iterator &_startLineIt) {
#ifdef DISABLE_INDEX_SPACERS
  if ((*_startLineIt)->sid == 96131)
    cout << "";
#endif
  lineCurrentColumn = 0;
  startLineIt       = _startLineIt;
  tabBeginOfLine_   = 0;
  lineStage         = START_LINE;
}

void Sm::IndentedLineContext::updateLineAttributes(SpacerStream &stream, SpacerStream::iterator &currentSpacerIt) {
  Sm::Spacer *currentSpacer = *currentSpacerIt;
  Sm::Spacer::Cathegory cat = currentSpacer->cathegory();
  switch (cat) {
    case Spacer::Tab_: {
      if (lineStage == START_LINE) {
        tabBeginOfLine_ += currentSpacer->getContextLength(tabAtBeginOfLine());
        codestreamAssert(tabBeginOfLine_ > 65535);
      }
    } break;
    default:
      lineStage = TAIL_LINE;
      break;
  }

  switch (cat) {
    case Spacer::Name_:
      lineCurrentColumn += Sm::s::Name::getContextLength(currentSpacerIt, stream);
      break;
    case Sm::Spacer::CMultiLineComment_:
      throw 999;
      break;
    case Sm::Spacer::OMultiLineComment_:
      break;
    default:
      lineCurrentColumn += currentSpacer->length();
      break;
  }
}


void Sm::IndentedLine::afterIncrement(Sm::Spacer *previousSpacer) {
  isNewline = false;

#ifdef DISABLE_INDEX_SPACERS
  Spacer *sp = *it;
  if (sp->sid == 1384184)
    cout << "";
#endif

  Spacer::Cathegory cat = previousSpacer ? previousSpacer->cathegory() : Spacer::Empty_;
  switch (cat) {
    case Spacer::Connect_: // команда connect
    case Spacer::Endl_:    // конец строки
    case Spacer::Empty_:   // начало индентинга
      startLine(it);
      isNewline = true;
      break;
    default:
      break;
  }

  if (Sm::s::TextChunk *textChunk = (*it)->toSelfTextChunk()) {
    if (textChunk->text_.size() && textChunk->text_.back() == '\n') {
      throw 999;
      startLine(it);
      isNewline = true;
      return;
    }
  }
  else if (Sm::s::MultilineTextChunck *mTextChunck = (*it)->toSelfMultilineTextChunk()) {
    mTextChunck->previousAbsolutePos = current.lineCurrentColumn;
    if (mTextChunck->isMultiline())
      startLine(it);
//    current.lineCurrentColumn += mTextChunck->length();
  }
  current.updateLineAttributes(stream, it);
}



void IndentedLine::startLine(SpacerStream::iterator &_startLineIt) {
  previous = current;
  current.startLine(_startLineIt);
}


void Sm::IndentedLine::incAndUpdate() {
#ifdef DISABLE_INDEX_SPACERS
  if ((*it)->sid == 96143)
    cout << "";
#endif

  Spacer* previous = *it;
  ++it;
  afterIncrement(previous);
}

size_t IndentingAction::getAid() {
  static size_t x = 0;
  ++x;
  if (x == 300075)
    cout << "";
  return x;
}

void IndentingAction::offsetColumnLength(int off) {
  for (Childs::value_type &v : childs)
    v->offsetColumnLength(off);
}

void IndentingBracket::offsetColumnLength(int off) {
  sAssert(off < 0 && (obracketCol < (unsigned int)(-off) || cbracketCol < (unsigned int)(-off)));
  obracketCol += off;
  cbracketCol += off;
  taboffset   += off;
  IndentingAction::offsetColumnLength(off);
}

IndentingAction::Column IndentingBracket::lastIndentedCol() const { return cbracketCol - taboffset; }
IndentingAction::Column IndentingSubconstruct::lastIndentedCol() const { return col - taboffset; }

IndentingAction::Column IndentingBracket::firstIndentedCol() const { return obracketCol - taboffset; }
IndentingAction::Column IndentingSubconstruct::firstIndentedCol() const { return col - taboffset; }


void IndentingSubconstruct::offsetColumnLength(int off) {
  sAssert(off < 0 && (col < (unsigned int)(-off)));
  col += off;
  taboffset += off;
  IndentingAction::offsetColumnLength(off);
}



void skipMultilineComment(Sm::SpacerStream &stream, SpacerStream::iterator nextIt, SpacerStream::iterator &updatedIt) {
  int deep = 1;
  for (++nextIt; nextIt != stream.end(); ++nextIt) {
    switch ((*nextIt)->cathegory()) {
      case Spacer::OMultiLineComment_:
        ++deep;
        break;

      case Spacer::CMultiLineComment_:
        --deep;
        if (!deep) {
          updatedIt = ++nextIt;
          return;
        }
        break;

      default:
        break;
    }
  }
  updatedIt = nextIt;
}

void Sm::IndentingAction::insertEndlAndTabReal(Sm::SpacerStream &stream, SpacerStream::iterator it, unsigned int newTabsize) {
  // Определение предыдущей категории - нужно для удаления лишнего пробельного разделителя после точки с запятой или запятой
  Spacer::Cathegory prevCat = Spacer::Empty_;
  if (it != stream.begin()) {
    SpacerStream::iterator prevIt = it;
    --prevIt;
    prevCat = (*prevIt)->cathegory();
  }

  if (it != stream.end()) { // пропуск многострочных комментариев
    SpacerStream::iterator nextIt = it;
    ++nextIt;
    Spacer::Cathegory cat;
    // пропуск спейсеров, представляющих синтаксически пустоту или пробел
    while (nextIt != stream.end() &&
           (((cat = (*nextIt)->cathegory()) == Spacer::Name_) || (cat == Spacer::Tab_)))
      ++nextIt;

    // пропуск многострочных комментариев
    if (nextIt != stream.end() && (*nextIt)->cathegory() == Spacer::OMultiLineComment_)
      skipMultilineComment(stream, nextIt, it);
  }

  // удаление лишнего пробельного разделителя после точки с запятой или запятой
  switch (prevCat) {
    case Spacer::Semicolon_:
    case Spacer::Comma_:
      while (it != stream.end() && (*it)->cathegory() == Spacer::Name_)
        it = stream.erase(it);
      break;
    default:
      break;
  }

#ifndef DISABLE_INDEX_SPACERS
  stream.insert(it, s::staticSpacers->endl.object());
  stream.insert(it, s::staticSpacers->getUniqueTabSpacer(newTabsize).object());
#else
  {
    Ptr<Sm::s::Endl> p = new s::Endl();
    stream.insert(it, p.object());
    Ptr<Sm::s::tab> t = new s::tab(newTabsize);
    stream.insert(it, t.object());

    static const unordered_set<unsigned int> insertedIds = {
      96776, 96777
    };
    if (insertedIds.count(p->sid) || insertedIds.count(t->sid))
      cout << "";
  }
#endif

  tabsize = newTabsize;
  indented = true;
}


void IndentingAction::insertEndlAndTabStub(Sm::SpacerStream &, SpacerStream::iterator, unsigned int newTabsize) {
  tabsize  = newTabsize;
  indented = true;
}


void Sm::Indenter::executeDelayIndenting() {
  IndentingContextShared sharedCtx(currentLine.stream, currentLine.previous.lineCurrentColumn);
  IndentingContext localCtx(sharedCtx);
  localCtx.setTab(currentLine.previous.tabAtBeginOfLine());

  indentingActions.indent(localCtx);
  indentingActions.childs.clear();
}


IndentingAction *Sm::IndentingAction::getFirstIndentedChild() {
  for (Childs::value_type &c : childs) {
    if (IndentingAction *retC = c->getFirstIndentedChild())
      return retC;
    if (c->indented)
      return c.object();
  }
  return 0;
}



bool IndentingAction::indentedSelfOrChilds() const {
  if (indented)
    return true;
  for (const Childs::value_type &v : childs)
    if (v->indentedSelfOrChilds())
      return true;
  return false;
}


IndentingAction *Sm::IndentingAction::getPrevIndented(
      IndentingContext &ctx,
      PositionInParent::reverse_iterator levelPos,
      bool &isMultiline)
{
  if (!parent || levelPos == ctx.shared.positionInParent.rend())
    return 0;

  if (indented)
    return this;

  for (unsigned int posInParent = **levelPos; posInParent; ) {
     --posInParent;
    Sm::sAssert(parent->childs.size() <= posInParent);
    if (IndentingAction *act = parent->childs[posInParent]) {
      if (act->indented)
        return act;
      if (!isMultiline && act->indentedSelfOrChilds())
        isMultiline = true;
    }

    // Ошибка в следующем коде: если вернуть позицию вложенной конструкции соседнего предыдущего
    // элемента - то выравнивание будет происходить относительно этой конструкции -
    // Из за этого будет нарушаться представление иерархии в коде - выравнивание относительно
    // не предка или соседа, а относительно элемента в сосденей вложенной иерархии
    // (которая принадлежит соседу или предку).
    // Нужно же выравнивать по позиции предыдущего или элемента-предка. (см. код выше)
    //    if (IndentingAction *act = parent->childs[posInParent]->getLastIndented())
    //      return act;
  }

  return parent->getPrevIndented(ctx, ++levelPos, isMultiline);
}

int IndentedPos::posWithTabsize(unsigned int startCol) const {
  if (!startCol && !absolutePos && !tabsize)
    return 0;
  return isMultiline ? tabsize : (startCol - absolutePos) + tabsize; // - 1;
}

void IndentingAction::indentSavedCopyBase(IndentingContext &ctx)
{
  unsigned int childPos = 0;
  ctx.shared.positionInParent.push_back(&childPos);
  for (Childs::value_type &ref : childs) {
    IndentedPos prevIndentedPos = ref->getPrevIndentedPos(ctx);
    if (ctx.skipTailIndenting(prevIndentedPos))
      break;

    ref->indent(ctx);
    ++childPos;
  }
  ctx.shared.positionInParent.pop_back();
}

void IndentingAction::indentSavedCopy(IndentingContext &ctx) {
  if (!skipChildsIndenting(ctx))
    indentSavedCopyBase(ctx);
}

void IndentingAction::indentStub(IndentingContext &ctx) {
  InsertEndlAndTab oldVal = ctx.shared.insertEndlAndTab;
  ctx.shared.insertEndlAndTab = &IndentingAction::insertEndlAndTabStub;
  IndentingAction::indent(ctx);

  ctx.shared.insertEndlAndTab = oldVal;

}

void IndentingAction::indent(IndentingContext &ctx) {
  if (skipChildsIndenting(ctx))
    return;

  IndentingContext localCtx(ctx);
  indentSavedCopyBase(localCtx);
  ctx.checkout(localCtx);
}

// subconstruct -> endl tab+2 subconstruct
void Sm::IndentingSubconstruct::indent(IndentingContext &ctx) {
  if (!indented) {
    unsigned int resultTabsize = tabsize + ctx.tab(); // tab+2
    codestreamAssert2(resultTabsize >= LINE_LENGTH_MAX);
    (this->*(ctx.shared.insertEndlAndTab))(ctx.shared.stream, next(it), resultTabsize);
  }

  ctx.setAbsoluteColumnPos(col);
  if (!skipChildsIndenting(ctx)) {
    IndentingContext localCtx(ctx);
    localCtx.incTabIfNeed();
    IndentingAction::indent(localCtx);
  }
}


IndentedPos IndentingAction::getPrevIndentedPos(IndentingContext &ctx) {
  bool isMultiline = indented;

  IndentingAction *v;
  if ((v = getPrevIndented(ctx, ctx.shared.positionInParent.rbegin(), isMultiline))) {
    // если найдены объемлющие скобки - взять за основу открывающую скобку
    for (IndentingAction *par = this->parent; par; par = par->parent)
      if (par == v)
        return IndentedPos(/*prevIndentedPos*/v->firstIndentedCol(), v->tabsize, isMultiline, v);

    // если найдена соседняя, идущая ранее группа - брать за основу закрывающую скобку
    return IndentedPos(/*prevIndentedPos*/v->lastIndentedCol(), v->tabsize, isMultiline, v);
  }

  return IndentedPos(0, 0, indented, 0);
}


IndentedPos IndentingBracket::getPrevIndentedPos(IndentingContext &ctx) {
  IndentedPos ip = IndentingAction::getPrevIndentedPos(ctx);
  Column absCbrCol = absCbracketCol();
  if (absCbrCol < ip.absolutePos)
    cout << "indenting error: absCbrCol=" <<  absCbrCol << " < ip.absolutePos=" << ip.absolutePos << endl;
  return ip;
}


void IndentingBracket::indentArglistBraces(IndentingContext localCtx, int cbracketTab)
{
  localCtx.incTab(tabsize);
  codestreamAssert2(localCtx.tab() >= LINE_LENGTH_MAX);

  localCtx.flags.clrSkipTail();
  IndentingAction::indent(localCtx);

  (this->*(localCtx.shared.insertEndlAndTab))(localCtx.shared.stream, next(obracket), localCtx.tab()); // obracket -> obracket endl ogroup tab+2
  (this->*(localCtx.shared.insertEndlAndTab))(localCtx.shared.stream, cbracket, cbracketTab);  // cbracket -> cgroup endl tab cbracket
  indented = true;
}


bool IndentingContext::skipTailIndenting(IndentedPos &prevIndentedPos) {
  checkInvariant();
  if (flags.skipTail()) {
    // проверка, что хвост имеет маленькую длину
    return shared.sourceLineSize_ - prevIndentedPos.absolutePos < LINE_LENGTH_MAX;
    // c absoluteColumnPos - нельзя сравнивать shared.sourceLineSize_, т.к. absoluteColumnPos может колебаться
    // между предыдущей перенесенной позицией и концом строки - и
    // возможны ситуации, кодга длина между текущей позицией и концом строки - вроде и меньше максимальной,
    // но при этом длина между последней перенесенной и концом строки - больше максимальной почти в два раза.
  }
  return false;
}

bool IndentingContext::skipChildsIndenting() {
  checkInvariant();
  return flags.indentOnlyLongChilds() && shared.sourceLineSize_ < LINE_LENGTH_MAX;
}

void IndentingContext::incTabIfNeed() {
  checkInvariant();
  if (flags.childsOffset() && !shared.positionInParent.empty())
    tab_ += 2;
}



bool IndentingBracket::updateCtxForCommaList(IndentingContext &localCtx)
{
  localCtx.incTabIfNeed();
  IndentedPos prevIndentedPos = getPrevIndentedPos(localCtx);
  int tab = prevIndentedPos.posWithTabsize(absObracketCol());
  if ((absCbracketCol() - prevIndentedPos.absolutePos) >= LINE_LENGTH_MAX) {
    localCtx.setTab(tab);
    return true;
  }
  return false;
}

void Sm::IndentingBracket::indent(IndentingContext &ctx) {
  static const PairSpacerCathegories pairSpacer;
  Spacer::Cathegory cat = (*obracket)->cathegory();
  IndentingContext localCtx(ctx);

  switch (cat) {
    case Spacer::OBracketView_:
      localCtx.incTab(2);
      localCtx.flags.clrIndentOnlyLongChilds();
      indentArglistBraces(localCtx, ctx.tab());
      break;

    case Spacer::OBracketArglist_:
      localCtx.incTab(2);
      indentArglistBraces(localCtx, ctx.tab());
      break;

    case Spacer::OBracketCall_:
      localCtx.incTab(2);
      indentArglistBraces(localCtx, ctx.tab());
      break;

    case Spacer::OBracketInsert_: {
      IndentedPos prevIndentedPos = getPrevIndentedPos(localCtx);
      if (absCbracketCol() - prevIndentedPos.absolutePos > LINE_LENGTH_MAX)  {
        indented = true;

        if (cbracketCol - obracketCol > LINE_LENGTH_MAX) { // перенос столбцом для очень длинного содежимого внутри скобок
          int oldPos = localCtx.absPos();
          localCtx.setAbsoluteColumnPos(localCtx.tab());
          offsetColumnLength(-(oldPos - localCtx.absPos()));
          localCtx.incTab(2);
          indentArglistBraces(localCtx, ctx.tab());
        }
        else { // перенос только скобок для содержимого скобок, умещающегося на отдельную строку
          (this->*(ctx.shared.insertEndlAndTab))(localCtx.shared.stream, next(obracket), localCtx.tab() + 2);
          (this->*(ctx.shared.insertEndlAndTab))(localCtx.shared.stream, cbracket, localCtx.tab());
        }
      }
      else {
        // рассчет длины до первого куска текста включительно после закрывающей скобки
        Sm::SpacerStream::iterator nextIt = next(cbracket);
        int cbracketOffset = 0;
        int spaceOffset = 0;
        for (; nextIt != ctx.shared.stream.end(); ++nextIt) {
          cbracketOffset += (*nextIt)->length();
          Spacer::Cathegory cat = (*nextIt)->cathegory();
          if (cat == Spacer::Name_ || pairSpacer.emptyCathegories[cat]) {
            spaceOffset += (*nextIt)->length();
            continue;
          }
          break;
        } // перенести после закрывающей скобки, если кусок текста после закрывающей скобки вылезет за предел
        if (this->absCbracketCol() + cbracketOffset - prevIndentedPos.absolutePos > LINE_LENGTH_MAX) { //
          indented = true;
          (this->*(ctx.shared.insertEndlAndTab))(localCtx.shared.stream, next(cbracket), localCtx.tab() + 1 + spaceOffset); // 1 - длина скобки, spaceOffset - длина пробела после закрывающей скобки до куска текста
        }
      }
    } break;

    case Spacer::OBracket_: {
      IndentingAction *firstIndentedChild;
      unsigned int firstIndentedChildPos;
      // Рассчет таба, на который надо выполнять смещение при индентинге
      IndentedPos prevIndentedPos = getPrevIndentedPos(localCtx);

      if ((absCbracketCol() - prevIndentedPos.absolutePos) >= LINE_LENGTH_MAX) {
        indented = true;

        if (obracket != localCtx.shared.stream.begin()) {

          unsigned int col = absObracketCol();
          {
            Spacer* prevObracket;
            if ((prevObracket = *prev(obracket))->cathegory() == Spacer::TextChunk_) {
              codestreamAssert(obracketCol < prevObracket->length());
              col -= prevObracket->length();
              codestreamAssert(col < prevIndentedPos.absolutePos);
            }
          }
          int tab = prevIndentedPos.posWithTabsize(col);
          localCtx.setTab(tab);
        }
        localCtx.incTabIfNeed();
        // индентинг содержимого скобок
        IndentingAction::indent(localCtx);

        firstIndentedChild = getFirstIndentedChild();
        firstIndentedChildPos = firstIndentedChild ? firstIndentedChild->lastIndentedCol() : 0;

//        IndentingSubconstruct *s;
        // CommaMakestr - не должна встречаться внутри OBracket_
//        sAssert(firstIndentedChild && ((s = firstIndentedChild->toSelfIndentingSubconstruct()) && (*(s->it))->cathegory() == Spacer::CommaMakestr_));

        // Проверка - что позиция первого перенесенного потомка - не стоит раньше позиции первого перенесенного предшественника по восходящей
        codestreamAssert(firstIndentedChild && (firstIndentedChildPos < prevIndentedPos.absolutePos));

        indented = false;
        // Индентинг самих символов скобок - при необходимости
        // Выбор первой позиции, которую следует считать следующей перенесенной после скобки позицией
        //   Это может быть либо позиция первого перенесенного потомка, либо (если потомки не переносились) - позиция закрывающей скобки
        if (firstIndentedChild) {
          unsigned int firstEndlPosAfterObracket = firstIndentedChild ? firstIndentedChildPos : absCbracketCol();
          if (firstEndlPosAfterObracket - prevIndentedPos.absolutePos >= LINE_LENGTH_MAX) {
            (this->*(ctx.shared.insertEndlAndTab))(localCtx.shared.stream, next(obracket), localCtx.tab() + 2); // obracket -> obracket endl ogroup tab+2
            (this->*(ctx.shared.insertEndlAndTab))(localCtx.shared.stream, cbracket, localCtx.tab());  // cbracket -> cgroup endl tab cbracket
            indented = true;
          }
        }
        else
          indented = false;
      }
    } break;


    case Spacer::OTabCommalist_:
      if (!childs.empty() && updateCtxForCommaList(localCtx)) {
        localCtx.incTab(2);
        localCtx.flags.setChildsOffset();
        IndentingAction::indentSavedCopy(localCtx);
      }
      break;

    case Spacer::OColumn_:
      localCtx.flags.clrSkipTail();
      // pass

    case Spacer::OCommalist_:
      if (!childs.empty() && updateCtxForCommaList(localCtx)) {
        localCtx.flags.clrChildsOffset();
        IndentingAction::indentSavedCopy(localCtx);
      }
      break;

    default:
      throw 999;
      break;
  }

  ctx.checkout(localCtx);
  ctx.setAbsoluteColumnPos(cbracketCol);
}



void IndentingAction::addChild(IndentingAction *node) {
  node->parent = this;
  childs.push_back(node);
}


void Sm::Indenter::addIndentingAction(IndentingAction *node)
{
  if (openBracesStack.empty())
    indentingActions.addChild(node);
  else
    openBracesStack.back()->addChild(node);
}

void Indenter::createSubconstructNode(int summaryTab)
{
  if (needIndentingLongSubconstruct)
    addIndentingAction(new IndentingSubconstruct(currentLine.it, currentLine.current.lineCurrentColumn, summaryTab));
}

void Indenter::createBracketNode()
{
  if (!commentDepth && needIndentingLongSubconstruct) {
    openBracesStack.push_back(new IndentingBracket(currentLine.it, currentLine.current.lineCurrentColumn, tabstack.summaryTab()));
    tabstack.pushSummary();
  }
}


struct PairSpacerCathegories {
  Spacer::Cathegory pairCathegories[Spacer::LastSpacerCathegory_];

  PairSpacerCathegories() {
    for (int i = 0; i < Spacer::LastSpacerCathegory_; ++i)
      pairCathegories[i] = Spacer::Empty_;

    pairCathegories[Spacer::OBracket_       ] = Spacer::CBracket_       ;
    pairCathegories[Spacer::OBracketArglist_] = Spacer::CBracketArglist_;
    pairCathegories[Spacer::OBracketInsert_ ] = Spacer::CBracketInsert_ ;
    pairCathegories[Spacer::OBracketCall_   ] = Spacer::CBracketCall_   ;
    pairCathegories[Spacer::OBracketView_   ] = Spacer::CBracketView_   ;
    pairCathegories[Spacer::OCommalist_     ] = Spacer::CCommalist_     ;
    pairCathegories[Spacer::OColumn_        ] = Spacer::CColumn_        ;
    pairCathegories[Spacer::OTabCommalist_  ] = Spacer::CTabCommalist_  ;
  }

};

void Sm::Indenter::buildIndentTree() {
  static const PairSpacerCathegories pairSpacer;
  Sm::Spacer::Cathegory cat = (*currentLine.it)->cathegory();

#ifdef DISABLE_INDEX_SPACERS
  if ((*currentLine.it)->sid == 102
//      || (*currentLine.it)->sid == 96148
      )
    cout << "";

  static size_t iter = 0;
  ++iter;
  if (iter == 37)
    cout << "";
#endif


  switch (cat) {
    case Spacer::OBracket_:
    case Spacer::OBracketCall_:
    case Spacer::OCommalist_:
    case Spacer::OColumn_:
    case Spacer::OTabCommalist_:
      createBracketNode();
      break;

    case Spacer::OBracketInsert_:
    case Spacer::OBracketArglist_:
    case Spacer::OBracketView_: {
      Sm::SpacerStream::iterator nextIt = next(currentLine.it);
      if (nextIt != currentLine.endIt() && pairSpacer.pairCathegories[cat] == (*nextIt)->cathegory()) {
        currentLine.it = nextIt;
        int tabOffset = 1; // длина закрывающей скобки
        for (/*TODO: убрать ++ и посмотреть что будет*/++nextIt; nextIt != currentLine.endIt(); ++nextIt) {
          Spacer::Cathegory cat = (*nextIt)->cathegory();
          switch (cat) {
            case Spacer::Name_:
              tabOffset += Sm::s::Name::getContextLength(nextIt, currentLine.stream);
              continue;
            case Spacer::Tab_:
              tabOffset += (*nextIt)->getContextLength(0);
              continue;

            default:
              if (pairSpacer.emptyCathegories[cat])
                continue;
              break;
          }
          break;
        }
        createSubconstructNode(tabstack.summaryTab() + tabOffset);
      }
      else
        createBracketNode();
      break;
    }

    case Spacer::CommaMakestr_: {
      IndentingSubconstruct *p = new IndentingSubconstruct(currentLine.it, currentLine.current.lineCurrentColumn, tabstack.summaryTab());
      if (currentLine.current.lineCurrentColumn < LINE_LENGTH_MAX/3)
        p->indented = true;
      addIndentingAction(p);
    } break;

    case Spacer::Semicolon_:
    case Spacer::Comma_:
      if (!commentDepth && needIndentingLongSubconstruct)
        addIndentingAction(new IndentingSubconstruct(currentLine.it, currentLine.current.lineCurrentColumn, tabstack.summaryTab()));
      break;

    case Spacer::CBracket_:
    case Spacer::CBracketArglist_: // pass
    case Spacer::CBracketInsert_:
    case Spacer::CBracketCall_:
    case Spacer::CBracketView_:
    case Spacer::CCommalist_:
    case Spacer::CColumn_:
    case Spacer::CTabCommalist_:
      if (!commentDepth && needIndentingLongSubconstruct) {
        Ptr<IndentingBracket> br = openBracesStack.back();
        openBracesStack.pop_back();

        br->cbracket    = currentLine.it;
        br->cbracketCol = currentLine.current.lineCurrentColumn;
        tabstack.popSummary();

        addIndentingAction(br.object());
      }
      break;

    case Spacer::Subconstruct_:
      createSubconstructNode(tabstack.summaryTab());
      break;

    case Spacer::OTabLevel_:
      tabstack.push((*currentLine.it)->level());
      break;

    case Spacer::CTabLevel_:
      codestreamAssert(tabstack.empty());
      tabstack.pop();
      break;

    default:
      break;
  }
}


void Indenter::executeLastDelayIndenting()
{
  currentLine.previous = currentLine.current;
  executeDelayIndenting();
}

void Sm::Indenter::mainIndentingLoop() {
  static size_t cnt = 0;
  ++cnt;
  if (cnt == 78255)
    cout << "";


  bool indentingDisabled = false;
  bool inSingleLineComment = false;
#ifdef DISABLE_INDEX_SPACERS
  bool breakLast = false;
#endif


  Spacer* previous = 0;
  for (; currentLine.it != currentLine.endIt(); ) {
#ifdef DISABLE_INDEX_SPACERS
    if ((*currentLine.it)->sid == 985145) {
      breakLast = true;
      cout << "";
    }
#endif
    currentLine.afterIncrement(previous);

    switch ((*currentLine.it)->cathegory()) {
      case Spacer::DisableIndenting_:
        indentingDisabled = true;
        break;

      case Spacer::EnableIndenting_:
        indentingDisabled = false;
        break;

      case Spacer::Comment_:
        inSingleLineComment = true;
        break;

      case Spacer::OMultiLineComment_: {
        skipMultilineComment(currentLine.stream, currentLine.it, currentLine.it);
        if (currentLine.it == currentLine.stream.end()) {
          executeLastDelayIndenting();
          return;
        }
        else
          --currentLine.it;
      } break;

      default:
        if (indentingDisabled)
          break;

        if (currentLine.isNewline) {
          currentLine.isNewline = false;
          inSingleLineComment = false;
          executeDelayIndenting();
        }
        if (!inSingleLineComment)
          buildIndentTree();
        break;
    }

    previous = *currentLine.it;
    ++currentLine.it;
  }

#ifdef DISABLE_INDEX_SPACERS
  if (breakLast)
    cout << "";
#endif

  executeLastDelayIndenting();
}



void Indenter::multilinePostFormat(SpacerStream::iterator it, MultilineComments &ctx) {
  if (ctx.empty())
    return;

  SpacerStream::iterator copyIt = it;

  MultilineComments::iterator vIt;
  /* В каждой новой строке многострочного комментария после начальных табов -
     * вставить комментарий, если он там еще не стоит
     */
  for (vIt = ctx.begin(); vIt != ctx.end(); ++ vIt) {
    s::OMultiLineComment *cmd = (*(vIt->begin))->toSelfOMultiLineComment();
    codestreamAssert(!cmd);
    for (SpacerStream::iterator cIt = vIt->begin; cIt != vIt->end; ++cIt)
      if ((*cIt)->cathegory() == Spacer::Endl_)
        for (++cIt; cIt != vIt->end; ++cIt) {
          bool inserted = false;
          switch ((*cIt)->cathegory()) {
            case Spacer::Name_:
            case Spacer::Tab_:
            case Spacer::Stub_:
              break;
            case Spacer::OMultiLineComment_:
            case Spacer::Comment_:
              inserted = true;
              break;
            default:
              inserted = true;
              currentLine.stream.insert(cIt, s::staticSpacers->comment(cmd->mode).object());
              break;
          }
          if (inserted)
            break;
        }
  }

  // Заменить промежутки комментариев на пробелы
  vIt = ctx.begin();
  MultilineComments::iterator prevEndIt = prev(ctx.end());
  vIt->begin = currentLine.stream.insert(vIt->begin, Sm::getSpacerName());
  if (vIt != prevEndIt) {
    if (vIt->end != currentLine.endIt())
      *(vIt->end) = Sm::getSpacerName();

    for (++vIt; vIt != prevEndIt; ++vIt) {
      *(vIt->begin) = Sm::getSpacerName();
      if (vIt->end != currentLine.endIt())
        *(vIt->end)   = Sm::getSpacerName();
    }
    *(vIt->begin) = Sm::getSpacerName();
  }

  for (vIt = ctx.begin(); vIt != ctx.end(); ++ vIt)
    currentLine.stream.splice(copyIt, currentLine.stream, vIt->begin, vIt->end);
  ctx.clear();
}

void Sm::Indenter::removeCommentsIfNeed() {
  if (!this->state.executeBlock_)
    return;

  for (SpacerStream::iterator it = currentLine.stream.begin(); it != currentLine.endIt(); ++it) {
    Spacer::Cathegory cat = (*it)->cathegory();
    switch (cat) {
      case Spacer::OMultiLineComment_:
      case Spacer::Comment_: {
        SpacerStream::iterator removeBegin = it;
        for (; it != currentLine.endIt(); ++it) {
          if ((*it)->cathegory() == Spacer::Endl_)
            break;
        }
        currentLine.stream.erase(removeBegin, it);
      }
      default:
        break;
    }
  }
}

void Sm::Indenter::indentMultilineComments() {
  MultilineComments ctx;

  for (SpacerStream::iterator it = currentLine.stream.begin(); it != currentLine.endIt(); ++it) {
//    if ((*it)->sid == 343418)
//      cout << "";
    Spacer::Cathegory cat = (*it)->cathegory();
    switch (cat) {
      case Spacer::OMultiLineComment_: {
        SpacerStream::iterator closeIt = it;
        skipMultilineComment(currentLine.stream, it, closeIt);
        if (closeIt != currentLine.endIt())
          --closeIt;
        if (ctx.size() > 1000000)
          cout << "";
        ctx.push_back(MultilineCommentContext(it, closeIt));
        it = closeIt;
      } break;

      case Spacer::Endl_: {
        multilinePostFormat(it, ctx);
      } break;

      default:
        break;
    }
  }
  multilinePostFormat(currentLine.endIt(), ctx);
}


namespace Sm {
void indenterTestTabs() {
  Sm::Codestream str;
//  str << s::ObracketArglist() << "a" << s::comma() << "b" << s::comma() << "c" << s::CbracketArglist() << s::StubLength(110) << s::endl;
  str.incIndentingLevel(2);
//  str << s::tab() << s::ObracketArglist() << "a" << s::comma() << "b" << s::comma() << "c" << s::CbracketArglist() << s::StubLength(110) << s::endl;
  str.incIndentingLevel(2);
  str.incIndentingLevel(2);
  str << s::OBracketView()    << "a" << s::comma() << "b" << s::comma() << "c" << s::CBracketView()    << s::StubLength(110) << s::endl;
  str << s::OBracketArglist() << "a" << s::comma() << "b" << s::comma() << "c" << s::CBracketArglist() << s::StubLength(110) << s::endl;
  str << s::OBracketCall()    << "a" << s::comma() << "b" << s::comma() << "c" << s::CBracketCall()    << s::StubLength(110) << s::endl;

  str << s::otablevel(2) << s::otablevel(2) << s::OBracketView()      << "a" << s::comma() << "b" << s::comma() << "c"  << s::CBracketView()    << s::ctablevel() << s::ctablevel()<< s::StubLength(110) << s::endl;
  str << s::otablevel(2) << s::otablevel(2) << s::OBracketArglist()   << "a" << s::comma() << "b" << s::comma() << "c"  << s::CBracketArglist() << s::ctablevel() << s::ctablevel()<< s::StubLength(110) << s::endl;
  str << s::otablevel(2) << s::otablevel(2) << s::OBracketCall()      << "a" << s::comma() << "b" << s::comma() << "c"  << s::CBracketCall()    << s::ctablevel() << s::ctablevel()<< s::StubLength(110) << s::endl;
  cout << str.str() << endl;

}


void indenterTest() {
  Sm::Codestream str;
  str.procMode(CodestreamState::PROC);

  str << s::tab(8)
      << s::statementPointer(0, str.state())
      << s::otabcommalist()
      << "EXECUTE"
      << s::name;

  stringstream mtext;
  mtext << "\"SELECT MAX(DataFrom)\n"
        << "  FROM M2_ALL.BudjetPeriod\n"
        << " WHERE (BJCL_ID = ?) AND (BLC_ID = ?) AND (DATAFROM < ?);\"";
  string mtextStr = mtext.str();

  Sm::translateMultilineText(str, mtextStr);

  str << s::CommaMakestr()
      << s::ocommalist()
      << "SYS.AA_GET_IKEY_INT"
         << s::obracket
         << "BUDJETCALC_LISTBUDJETPERIOD"
         << s::CommaCodegen()
         << "I"
         << s::CommaCodegen()
         << "OLD_BJCL_ID"
         << s::cbracket
      << s::CommaCodegen()
      << "SYS.AA_GET_IKEY_INT"
         << s::obracket
         << "BUDJETCALC_LISTBUDJETPERIOD"
         << s::CommaCodegen()
         << "I"
         << s::CommaCodegen()
         << "OLD_BLC_ID"
         << s::cbracket
      << s::CommaCodegen()
      << "SYS.AA_GET_IKEY_DATE"
         << s::obracket
         << "BUDJETCALC_LISTBUDJETPERIOD"
         << s::CommaCodegen()
         << "I"
         << s::CommaCodegen()
         << "OLD_DATAFROM"
         << s::cbracket
      << s::CommaCodegen()
      << "SYS.AA_GET_IKEY_DATE"
         << s::obracket
         << "BUDJETCALC_LISTBUDJETPERIOD"
         << s::CommaCodegen()
         << "I"
         << s::CommaCodegen()
         << "OLD_DATAFROM"
         << s::cbracket
      << s::CommaCodegen()
      << "SYS.AA_GET_IKEY_DATE"
         << s::obracket
         << "BUDJETCALC_LISTBUDJETPERIOD"
         << s::CommaCodegen()
         << "I"
         << s::CommaCodegen()
         << "OLD_DATAFROM"
         << s::cbracket



      << s::ccommalist()
      << s::ctabcommalist()
      << s::name
      << "INTO"
      << s::name
      << "VSP_DATATO"
      << s::semicolon
      << s::endl;

  cout << str.str() << endl;
}







}
