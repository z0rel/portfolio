#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "project_optimization.h"
COMPILER_HASH_FUN_OPTIMIZATION_PUSH()


#include <string.h>
#include <cstddef>
#include <iterator>
#include <utility>
#include "hash_fun.h"


namespace BackportHashMap {

template < typename key_t, typename value_t >
struct HashPair {
  HashPair( key_t * _key, value_t * _value )
    : key_ptr  (_key  ),
      value_ptr(_value) {}

  key_t   *& key_ptr  ;
  value_t *& value_ptr;

  inline key_t   & key  () const { return *key_ptr  ; }
  inline value_t & value() const { return *value_ptr; }
};

namespace internal {
template < typename HashMapType, typename key_t, typename value_t >
class Iterator;
}

template < typename key_t, typename value_t >
class HashMap
{
  friend class internal::Iterator<       HashMap<key_t, value_t>, key_t, value_t >;
  friend class internal::Iterator< const HashMap<key_t, value_t>, key_t, value_t >;

  typedef const HashMap <key_t, value_t> __const_map_type;
public:
  typedef std::pair<key_t, value_t> value_type;
  typedef value_t mapped_type;
  typedef key_t   key_type  ;

  typedef internal::HashActions<key_t, value_t> actions_t;
  typedef HashMap<key_t, value_t>               map_type ;

  typedef internal::Iterator<
      HashMap<key_t, value_t>, key_t, value_t >       iterator      ;
  typedef internal::Iterator<
      const HashMap<key_t, value_t>, key_t, value_t > const_iterator;

  typedef typename actions_t::HashFunc           HashFunc          ;
  typedef typename actions_t::KeyEqualFunc       KeyEqualFunc      ;
  typedef typename actions_t::DestroyKeyNotify   DestroyKeyNotify  ;
  typedef typename actions_t::DestroyValueNotify DestroyValueNotify;


  template < typename userdata_t >
  struct RemovePredicate {
    typedef bool (*pf ) ( key_t      & key       ,
                          value_t    & value     ,
                          userdata_t   user_data );
    typedef userdata_t Userdata;
  };

  template < typename userdata_t >
  struct EqualPredicate {
    typedef bool (*pf ) ( const key_t   & key       ,
                          const value_t & value     ,
                          userdata_t      user_data );
    typedef userdata_t Userdata;
  };

  template < typename userdata_t >
  struct TransformAction {
    typedef void (*pf ) ( const key_t  & key       ,
                          value_t     *& value     ,
                          userdata_t     user_data );
    typedef userdata_t Userdata;
  };

  typedef void (*HashInitializerFunc ) ( map_type & hash, void* user_data );

  static void value_single_destroyer( value_t *data ) { delete   data; }
  static void value_array_destroyer ( value_t *data ) { delete[] data; }
  static void key_single_destroyer  ( key_t   *data ) { delete   data; }
  static void key_array_destroyer   ( key_t   *data ) { delete[] data; }

  inline iterator       begin ()       { return iterator      ( this    ); }
  inline iterator       end   ()       { return iterator      ( this, 1 ); }

  inline const_iterator cbegin() const { return const_iterator( this    ); }
  inline const_iterator cend  () const { return const_iterator( this, 1 ); }

  /// Конструктор по умолчанию
  HashMap () { init_full( NULL, NULL); }

  /// Инициализация из функции
  HashMap ( HashInitializerFunc initializer, void* user_data = 0 ) {
    init_full  (  NULL, NULL      );
    initializer( *this, user_data );
  }

  /// Конструктор копирования
  HashMap( const map_type & o );

  /// Конструктор обмена
  // Внимание: при совпадении типов ключа и значения это не пройдет, и будет вызван копирующий конструктор.
  template <typename oth_key_t, typename oth_value_t>
  HashMap( const HashMap<oth_key_t, oth_value_t> & src, int = 0, int = 0 ) {
    swapInit( src );
  }

  // для явного указания, что должен быть вызван конструктор обмена
  template <typename oth_key_t, typename oth_value_t>
  HashMap( const HashMap<oth_key_t, oth_value_t> & src, int ) {
    swapInit( src );
  }


  HashMap ( DestroyKeyNotify key_destroy_func, DestroyValueNotify value_destroy_func ) {
    init_full( key_destroy_func, value_destroy_func );
  }

  virtual ~HashMap();

  inline bool      find_pair( const key_t & key, key_t **orig_key, value_t *& value );
  inline value_t * find     ( const key_t & key ) const;
  inline size_t    size     () const { return nnodes;   }
  inline bool      empty    () const { return !nnodes;  }

  // true => была замена
  value_t* insert  ( const key_t & key, const value_t & value, bool & already_exist );
  value_t* insert  ( const key_t & key, const value_t & value );
  void     replace ( const key_t & key, const value_t & value );
  bool     contains( const key_t & key ) const;
  void     add     ( const key_t & key );
  bool     erase   ( const key_t & key );
  void     clear   ();

  std::pair< key_t*, value_t* > insert_pair( const key_t & key, const value_t & value );

  inline value_t & operator[] ( const key_t & key );

private:
  template < typename oth_key_t, typename oth_value_t >
  inline void swapInit( const HashMap<oth_key_t, oth_value_t> & src ) {
    init_full( NULL, NULL );

    for ( typename HashMap<oth_key_t, oth_value_t>::const_iterator
          cit = src.cbegin(); cit; ++cit )
      insert(*cit.value, *cit.key);
  }

  inline void setShift         ( int shift       );
  inline void setShiftBySize   ( int arrays_size );

  inline uint lookup_node      ( const key_t & key, uint * hash_return ) const;
  inline bool remove_internal  ( const key_t & key, bool   notify      );

  inline void remove_node      ( bool notify, int i );
  inline void remove_all_nodes ( bool notify        );

  inline void resize           ();
  inline void maybe_resize     ();

  inline value_t * insert_node    ( uint            node_index         ,
                                    uint            key_hash           ,
                                    const key_t   & new_key_ref        ,
                                    const value_t * new_value_ref      ,
                                    bool          * already_exists = 0 );

  inline value_t * insert_internal( const key_t   & key                ,
                                    const value_t * value              ,
                                    bool          * already_exists = 0 );

  void init_full ( DestroyKeyNotify   key_destroy_func   ,
                   DestroyValueNotify value_destroy_func );

  int        arrays_size;
  int        mod        ;
  uint       mask       ;
  int        nnodes     ;
  int        noccupied  ;  /* nnodes + tombstones */
  key_t   ** keys       ;
  uint     * hashes     ;
  value_t ** values     ;

  DestroyKeyNotify   key_destroy_func  ;
  DestroyValueNotify value_destroy_func;
};

}
#ifndef __HASH_TABLE_INTERNAL_TEST__
#include "hashtable_internal.h"
#endif


COMPILER_HASH_FUN_OPTIMIZATION_POP()

#endif // HASHTABLE_H
