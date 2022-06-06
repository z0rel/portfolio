#ifndef __SMARTPTR_H
#define __SMARTPTR_H

#include <string>
#include <iostream>
#include <unordered_map>
#include <set>
#include <vector>

//#define SMARTPTR_DEBUG1 1

#include "smartptr_internal.h"

#ifdef SMARTPTR_DEBUG1

namespace smart {
  class   Smart;
  typedef Smart SingleSmart;


  void checkSmartPtr(const Smart *ptr);
  void checkSmartPtrToMax(const Smart *ptr);
  void constructorCheckSmartPtrToMax(const Smart *ptr);
  void constructorCheckSmartPtr(const Smart *ptr);


class Smart : public smart_internal::Smart {
public:
  int isDeleted__ = 0;


  Smart& operator=(const Smart &o) {
    constructorCheckSmartPtr(this);
    constructorCheckSmartPtr(&o);

    smart_internal::Smart::operator =(o);
    return *this;
  }
  Smart() : smart_internal::Smart() {}
  Smart(const Smart &o) : smart_internal::Smart(o) {
    constructorCheckSmartPtr(this);
    constructorCheckSmartPtr(&o);
  }
  virtual ~Smart() { isDeleted__ = 1; }

protected:
  Smart(int) : smart_internal::Smart(1) {}
};

template <class SMART> class Ptr : protected smart_internal::Ptr<SMART> {
  typedef smart_internal::Ptr<SMART> Base;

public:
  using Base::value_type;
  using Base::dereferenced_type;

  Ptr() : Base() {}
  Ptr(SMART *ptrArg) : Base(ptrArg) { constructorCheckSmartPtrToMax(this->smartPtr); }
  Ptr(const Ptr &ptr) : Base(ptr) { constructorCheckSmartPtrToMax(this->smartPtr); }
  Ptr(Ptr &&ptr) : Base(ptr) { constructorCheckSmartPtrToMax(this->smartPtr); }

  inline ~Ptr() { constructorCheckSmartPtrToMax(this->smartPtr); }

  inline void moveGet(Ptr<SMART> &oth) {
    checkSmartPtrToMax(oth.smartPtr);
    Base::moveGet(oth);
  }
  inline smart::Ptr<SMART> &operator=(const SMART *ptrArg) {
    constructorCheckSmartPtrToMax(ptrArg);
    checkSmartPtrToMax(this->smartPtr);
    Base::operator =(ptrArg);
    checkSmartPtrToMax(this->smartPtr);
    return *this;
  }
  inline smart::Ptr<SMART> &operator=(const smart::Ptr<SMART> &ptrArg) {
    checkSmartPtrToMax(ptrArg.smartPtr);
    checkSmartPtrToMax(this->smartPtr);
    Base::operator =(ptrArg);
    checkSmartPtrToMax(this->smartPtr);
    return *this;
  }
  bool valid() const {
    constructorCheckSmartPtrToMax(this->smartPtr);
    return Base::valid();
  }
  SMART*        object()      const {
    constructorCheckSmartPtrToMax(this->smartPtr);
    return Base::object();
  }
  const SMART*  objectConst() const {
    constructorCheckSmartPtrToMax(this->smartPtr);
    return Base::objectConst();
  }
  bool operator!()  const {
    constructorCheckSmartPtrToMax(this->smartPtr);
    return Base::operator !();
  }
  bool operator==(const SMART *ptr) const {
    constructorCheckSmartPtrToMax(this->smartPtr);
    constructorCheckSmartPtrToMax(ptr);
    return Base::operator ==(ptr);
  }
  bool operator!=(const SMART *ptr) const {
    constructorCheckSmartPtrToMax(this->smartPtr);
    constructorCheckSmartPtrToMax(ptr);
    return Base::operator !=(ptr);
  }
  operator bool()         const {
    constructorCheckSmartPtrToMax(this->smartPtr);
    return Base::operator bool();
  }
  operator const SMART*() const {
    checkSmartPtrToMax(this->smartPtr);
    return Base::operator const SMART *();
  }
  operator SMART*()       const {
    checkSmartPtrToMax(this->smartPtr);
    return Base::operator SMART *();
  }
  SMART* operator->() {
    checkSmartPtr(this->smartPtr);
    return Base::operator ->();
  }
  const SMART*  operator->() const {
    checkSmartPtr(this->smartPtr);
    return Base::operator ->();
  }
  SMART& operator*() {
    checkSmartPtr(this->smartPtr);
    return Base::operator *();
  }
  const SMART& operator*() const {
    checkSmartPtr(this->smartPtr);
    return Base::operator *();
  }
};


#if SMARTPTR_DEBUG

#define SET_OFFSET_THIS(ptr) ptr->offsetThisPointer__ = ((ssize_t)(ptr->getThisPtr()) - (ssize_t)ptr)
#define DEL_THIS(ptr) delete (uint8_t*)((uint8_t*)ptr + ptr->offsetThisPointer__)

namespace smartRtti {

void addSmartptr(const smart::Smart *ptr);
void eraseSmartptr(const smart::Smart *ptr);
struct DataOfTypes {
  std::string name;
  size_t size;
  std::set<const smart::Smart*> instances;
  size_t maxItems;
};
typedef std::unordered_map<size_t, DataOfTypes*> MaximalSizes;
void smartDoAction(const smart::Smart *ptr, bool isInc);

}

#endif


}

#endif

namespace smart {

    typedef std::set<const void*> SmartptrSet;

}


#endif
// vim:foldmethod=marker:foldmarker={,}
