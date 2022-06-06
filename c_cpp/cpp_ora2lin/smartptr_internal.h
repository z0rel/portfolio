#ifndef SMARTPTR_INTERNAL
#define SMARTPTR_INTERNAL



#include <limits>
#include <climits>
#include <set>
#include "smartptr_union.h"

#include "project_optimization.h"
COMPILER_SMARTPTR_OPTIMIZATION_PUSH()

namespace smart {

#ifdef SMARTPTR_DEBUG1
namespace smart_internal {
#endif


/// Базовый класс для подсчета ссылок
class Smart {
public:
  typedef void SmartVoidType;
  /// Счетчик ссылок для реализации [отложенной] деструкции объекта. Если weakRef = 0, объект удаляется
  mutable volatile SmartCount strongRef;
  mutable attributes::Attributes __flags__;

  Smart& operator=(const Smart &o) { __flags__.v = o.__flags__.v; return *this; }
  Smart()               : strongRef(0)           { __flags__.v = 0; }
  Smart(const Smart &o) : strongRef(o.strongRef) { __flags__.v = o.__flags__.v; }
  ~Smart() { strongRef = std::numeric_limits<SmartCount>::max(); }

  enum SMART_MAXVALUE { SMART_MAXVALUE__ = USHRT_MAX };

protected:
  Smart(int) : strongRef(1) { __flags__.v = 0; }
};

class Smart4 {
public:
  typedef void SmartVoidType;
  enum SMART_MAXVALUE { SMART_MAXVALUE__ = UINT_MAX };

  /// Счетчик ссылок для реализации деструкции объекта.
  mutable volatile unsigned int strongRef;


  Smart4& operator=(const Smart &) { return *this; }
  Smart4()               : strongRef(0)           {}
  Smart4(const Smart &o) : strongRef(o.strongRef) {}
  ~Smart4() { strongRef = SMART_MAXVALUE__; }

protected:
  Smart4(int) : strongRef(1) {}
};


typedef Smart SingleSmart;



/// Интерфейс подсчета ссылок Smart::strongRef, с возможностью инициализации из Smart::Ref
/// При множественном наследовании Smart - нужно делать его виртуальным, чтобы экземпляр ссылки был один, иначе память будет либо рушиться либо будут утечки
template <class SMART> class Ptr {

  inline void init(const SMART *ptr) {
//    const SMART* ptr = ptrArg;
    if (ptr != nullptr && ptr->strongRef != SMART::SMART_MAXVALUE__) {
      ++(ptr->strongRef);
      smartPtr = const_cast<SMART*>(ptr);
    }
    else
      smartPtr = nullptr;
  }

#ifdef SMARTPTR_DEBUG1
protected:
#endif

  SMART *smartPtr;
public:
  typedef SMART value_type;
  typedef SMART dereferenced_type;

  inline Ptr() : smartPtr(0) {}
  inline Ptr(SMART *ptrArg)  { init(ptrArg); }
  inline Ptr(const Ptr &ptr) { init(ptr.smartPtr); }

  inline Ptr(Ptr &&ptr) {
    smartPtr = ptr.smartPtr;
    ptr.smartPtr = 0;
  }

  inline ~Ptr() {
    if (smartPtr != nullptr && --(smartPtr->strongRef) == 0) {
      delete smartPtr;
      smartPtr = 0;
    }
  }

  inline void moveGet(Ptr<SMART> &oth) {
    smartPtr = oth.smartPtr;
    oth.smartPtr = 0;
  }

  inline Ptr &operator=(const SMART *ptrArg) {
    this->~Ptr();
    init(ptrArg);
    return *this;
  }

  inline Ptr &operator=(const Ptr &ptr) { return operator =(ptr.smartPtr); }

  inline bool          valid()       const { return smartPtr != 0; }
  inline SMART*        object()      const { return smartPtr; }
  inline const SMART*  objectConst() const { return smartPtr; }

  inline bool operator!()  const { return !valid(); }
  inline bool operator==(const SMART * const ptr) const { return smartPtr == ptr; }
  inline bool operator!=(const SMART * const ptr) const { return smartPtr != ptr; }

  inline operator bool()         const { return valid(); }
  inline operator const SMART*() const { return objectConst(); }
  inline operator SMART*()       const { return object(); }

  inline SMART*        operator->()       { return object(); }
  inline const SMART*  operator->() const { return objectConst(); }
  inline SMART&        operator*()        { return *object(); }
  inline const SMART&  operator*()  const { return *object(); }
};

#ifdef SMARTPTR_DEBUG1
}
#endif


}

COMPILER_SMARTPTR_OPTIMIZATION_POP()



#endif // SMARTPTR_INTERNAL
