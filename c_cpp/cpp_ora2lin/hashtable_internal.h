#ifndef HASHTABLE_INTERNAL_H
#define HASHTABLE_INTERNAL_H

#include "project_optimization.h"
COMPILER_HASH_FUN_OPTIMIZATION_PUSH()

#include <stdio.h>
#include <stdlib.h>
#include "hash_table.h"

#if  defined(_WIN64) || defined(_MSC_VER) || _WIN64 || defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#if _MSC_VER > 1400
#define __PRETTY_FUNCTION__ __FUNCTION__
#else
#define __PRETTY_FUNCTION__ " it is very very old compiler (: "
#endif
#endif

namespace BackportHashMap {
// вынести в static cons int
#define HASH_TABLE_MIN_SHIFT 3  /* 1 << 3 == 8 buckets */

#define VERIFY_AND_RETURN(expr)	\
       if ( (expr) ) { } else  { \
          printf ("error in : = %s, %s\n", __PRETTY_FUNCTION__, #expr); \
          return; \
       } \

#define VERIFY_AND_RETURN_VAL( expr, value )	\
       if ( (expr) ) { } else  { \
          printf ("error in : = %s, %s\n", __PRETTY_FUNCTION__, #expr); \
          return (value); \
       } \

/* хэш актуален */
#define HASH_IS_REAL(h_) ( (h_) > 1 )

template < typename key_t, typename value_t >
HashMap<key_t, value_t>::HashMap( const map_type & o )
  : arrays_size       ( o.arrays_size        ),
    mod               ( o.mod                ),
    mask              ( o.mask               ),
    nnodes            ( o.nnodes             ),
    noccupied         ( o.noccupied          ),
    key_destroy_func  ( o.key_destroy_func   ),
    value_destroy_func( o.value_destroy_func )
{
  keys   = new key_t*[arrays_size];
  hashes = new uint[arrays_size];
  for ( int i = 0; i < arrays_size; ++i ) {
    hashes[i] = o.hashes[i];
    if ( o.keys[i] )
      keys[i] = new key_t(*o.keys[i]);
  }
  if ( o.keys == (key_t**)o.values )
    values = (value_t**)keys;
  else {
    values   = new value_t*[arrays_size];
    for ( int i = 0; i < arrays_size; ++i )
      if ( o.values[i] )
        values[i] = new value_t(*o.values[i]);
  }
}

/**
 * Вставка нового узла или замена значения старого
 */
template < typename key_t, typename value_t >
value_t* HashMap<key_t, value_t>::insert (
    const key_t & key, const value_t & value) {
  return insert_internal( key, &value );
}

/**
 * Вставка нового узла. Не заменять если узел уже существует.
 */
template < typename key_t, typename value_t >
value_t* HashMap<key_t, value_t>::insert (
    const key_t & key, const value_t & value, bool & already_exist) {
  return insert_internal( key, &value, &already_exist);
}
template < typename key_t, typename value_t >
std::pair< key_t*, value_t* > HashMap<key_t, value_t>::insert_pair(
    const key_t & key, const value_t & value ) {

  uint key_hash;
  uint node_index = lookup_node( key, &key_hash );
  value_t * ptr = insert_node( node_index, key_hash, key, &value, (bool *)0 );

  return std::pair<key_t*, value_t*>( keys[node_index] , ptr);
}


/**
 * Заменить и ключ и значение в таблице на новые.
 */
template < typename key_t, typename value_t >
void HashMap<key_t, value_t>::replace (
    const key_t & key, const value_t & value) {
  insert_internal( key, &value );
}


/**
 * Вставить новое значение в таблицу. Если ключ для значения не существует - он будет создан.
 */
template < typename key_t, typename value_t >
void HashMap<key_t, value_t>::add ( const key_t & key ) {
  insert_internal( key, &key );
}

/**
 * Проверить, содержится ли ключ в таблице.
 */
template < typename key_t, typename value_t >
bool HashMap<key_t, value_t>::contains ( const key_t &  key) const
{
  uint node_index;
  uint node_hash;

  node_index = lookup_node( key, &node_hash );

  return HASH_IS_REAL( hashes[node_index] );
}


/**
 * Удаление ключа и значения, ассоциированного с ним из хэш-таблицы
 * @return @c true если ключ был найден и удален из хэш-таблицы
 */
template < typename key_t, typename value_t >
bool HashMap<key_t, value_t>::erase ( const key_t & key ) {
  return remove_internal ( key, true );
}

/**
 * Удалить все ключи и все ассоциированные с ними значения из хэш-таблицы
 */
template < typename key_t, typename value_t >
void HashMap<key_t, value_t>::clear()
{
  remove_all_nodes(true);
  maybe_resize();
}

template < typename key_t, typename value_t >
inline value_t & HashMap<key_t, value_t>::operator[] ( const key_t & key ) {
  bool    exists;
  return *insert_internal ( key, 0, &exists );
}


template < typename key_t, typename value_t >
inline void HashMap<key_t, value_t>::setShift( int shift ) {
  /* Каждый размер таблицы ассоциируется с простым модулем (первые простые числа
   * меньше, чем размер таблицы) used to find the initial bucket. Probing
   * then works modulo 2^n. Простой модуль необходим для получения хорошего
   * распределения при плохих хэш-функциях.
   */
  static const int prime_mod [] = {
    1         , /* Для 1 << 0 */
    2         ,
    3         ,
    7         ,
    13        ,
    31        ,
    61        ,
    127       ,
    251       ,
    509       ,
    1021      ,
    2039      ,
    4093      ,
    8191      ,
    16381     ,
    32749     ,
    65521     , /* Для 1 << 16 */
    131071    ,
    262139    ,
    524287    ,
    1048573   ,
    2097143   ,
    4194301   ,
    8388593   ,
    16777213  ,
    33554393  ,
    67108859  ,
    134217689 ,
    268435399 ,
    536870909 ,
    1073741789,
    2147483647  /* Для 1 << 31 */
  };

  arrays_size = 1 << shift      ;
  mod         = prime_mod[shift];
  mask        = ~((uint)-1 << shift); /* shift единиц от младшего разряда */
}

template < typename key_t, typename value_t >
inline void HashMap<key_t, value_t>::setShiftBySize ( int size )
{
  int shift;
  for ( shift = 0; size; ++shift )
    size >>= 1;

  shift = (shift > HASH_TABLE_MIN_SHIFT) ? shift : HASH_TABLE_MIN_SHIFT;
  setShift ( shift );
}

/**
 * @parem key    искомый ключ
 * @hash_return  указатель для размещения хэш-значения
 *
 * Выполнение поиска в хэш-таблице, с сохранением  дополнительной
 * информации, обычно необходимой для вставки.
 *
 * Данная функция сначала вычисляет значение хэша от ключа, используя
 * заданную хэш-функцию.
 *
 * Если вхождение в таблицу совпадающее с ключем найдено,
 * то данная функция возвращает индекс позиции в таблице и если нет,
 * то индекс неиспользуемого узла (пустого либо надгробия), куда
 * может быть вставлен ключ.
 *
 * Вычисленное хэш-значение возвращается в переменной, на которую
 * указывает hash_return.
 *
 * Это необходимо для сохранения уже посчитанного хэш-значения для для новой записи
 * (при вставке)
 *
 * На этапе инициализации либо добавления элементов было рассчитано:
 * <размер таблицы> = 1 << позиция самого левого разряда(<количество узлов> * 2)
 * <модуль> = простому числу, наиболее близкому к <размер таблицы>.
 * <маска > = 1 от младшего к старшему разряду до позиции
 *            <позиция самого левого разряда(<количество узлов> * 2)> включительно
 *
 * Поиск позиции с заданным хэшем:
 * <1 pos> = <хэш> % <модуль>
 * <2 pos> = (<1 pos> + 1) & <маска>
 * <3 pos> = (<2 pos> + 2) & <маска>
 * <4 pos> = (<3 pos> + 3) & <маска>
 * ...
 *
 * @return индекс описанного узла
 */
template < typename key_t, typename value_t >
inline uint HashMap<key_t, value_t>::lookup_node (
    const key_t & key, uint * hash_return) const
{
  uint node_index;
  uint node_hash;
  uint hash_value;
  uint first_tombstone = 0;
  bool have_tombstone  = false;
  uint step = 0;

  hash_value = internal::HashFunctions::hash_fun<key_t>(key);
  if ( (!HASH_IS_REAL (hash_value)) )
    hash_value = 2;

  *hash_return = hash_value;

  node_index = hash_value % mod;
  node_hash  = hashes[node_index];

  /* пока хэш используется */
  while ( node_hash ) {
      /* Сначала выполняется проверка: "совпадают ли полные хэш-значения",
       * таким образом в большинстве случаев избегается полное сравнение ключей
       * с помощью вызова функции сравнения.
       */
    if (node_hash == hash_value) {
      key_t * node_key = keys[node_index];

      if ( keys != (key_t**)values &&
          internal::HashFunctions::key_equal<key_t> (node_key, key) )
        return node_index;
      else if (node_key == &key)
        return node_index;
    }
    else if ( (node_hash == 1) && !have_tombstone ) { /* в данном узле является надгробием */
      first_tombstone = node_index;
      have_tombstone = true;
    }

    node_index += ++step;
    node_index &= mask;
    node_hash   = hashes[node_index];
  }

  if (have_tombstone)
    return first_tombstone;

  return node_index;
}

/**
 * @param notify @c true, если должна быть освобождена память узла
 *
 * Удаление узла из хэш-таблицы и модификация счетчика узлов.
 * Узел заменяется на надгробие. Не выполняется изменение размера таблицы.
 */
template < typename key_t, typename value_t >
inline void HashMap<key_t, value_t>::remove_node (
    bool notify, int i ) {
  key_t   * key;
  value_t * value;

  key   = keys  [i];
  value = values[i];

  /* Установить надгробие (tombstone) - данный хэш "умер" */
  hashes[i] = 1;

  keys  [i] = (key_t  *)0;
  values[i] = (value_t*)0;

  nnodes--;

  if (notify && key_destroy_func)
    key_destroy_func   (key  );

  if (notify && value_destroy_func)
    value_destroy_func (value);
}

/**
 * @param notify @c true, если должна быть освобождена память узлов
 *
 * Удаление всех узлов из таблицы. Поскольку это может являться
 * шагом перед полным удалением таблицы, не производится resize.
 */
template < typename key_t, typename value_t >
inline void HashMap<key_t, value_t>::remove_all_nodes ( bool notify )
{
  key_t   * key   = 0;
  value_t * value = 0;

  nnodes    = 0;
  noccupied = 0;

  if ( !notify ||
      ( key_destroy_func   == NULL &&
        value_destroy_func == NULL) )
  {
    memset (hashes, 0, arrays_size * sizeof (uint    ));
    memset (keys  , 0, arrays_size * sizeof (key_t  *));
    memset (values, 0, arrays_size * sizeof (value_t*));

    return;
  }

  for ( int i = 0; i < arrays_size; ++i ) {
    if (HASH_IS_REAL (hashes[i])) {
      key   = keys[i];
      value = values[i];

      hashes[i] = 0          ; /* хэш не используется */
      keys  [i] = (key_t*)0  ;
      values[i] = (value_t*)0;

      if ( ( key != (key_t*)0 ) && key_destroy_func != 0 )
        key_destroy_func (key);

      if ( ( value != (value_t *)0 ) && value_destroy_func != 0 )
        value_destroy_func (value);
    }
    else if ( hashes[i] == 1 ) /* хэш умер - hash is tombstone */
      hashes[i] = 0; /* хэш не используется */
  }
}

/**
 * Изменение размера хэш-таблицы на оптимальный размер основано на
 * числе удерживающихся узлов. Если вызвана данная функция, то
 * изменение размера произойдет даже если это не является необходимым.
 *
 * Данная функция может "изменить" хэш-таблицу на ее текущий размер, с
 * дополнительным эффектом очистки надгробий, другими словами выполняется
 * оптимизация поиска.
 */
template < typename key_t, typename value_t >
inline void HashMap<key_t, value_t>::resize()
{
  key_t   **new_keys  ;
  value_t **new_values;
  uint     *new_hashes;
  int       old_size  ;
  int       i         ;

  old_size = arrays_size;
  setShiftBySize ( nnodes * 2 );

  new_keys = new key_t*[ arrays_size ];
  memset(new_keys, 0, arrays_size * sizeof(key_t*) );
  if ( keys == (key_t**)values)
    new_values = (value_t**)new_keys;
  else {
    new_values = new value_t*[ arrays_size ];
    memset(new_values, 0, arrays_size * sizeof(value_t*) );
  }
  new_hashes =  new uint[arrays_size];
  memset(new_hashes, 0, arrays_size * sizeof(uint) );

  for ( i = 0; i < old_size; ++i )
  {
    uint node_hash = hashes[i];
    uint hash_val;
    uint step = 0;

    if ( !HASH_IS_REAL (node_hash) )
      continue;

    hash_val = node_hash % mod;
    /* пока хэш-значение используется */
    while ( new_hashes[hash_val] ) {
      ++step;
      hash_val += step;
      hash_val &= mask;
    }

    new_hashes[hash_val] = hashes[i];
    new_keys  [hash_val] = keys  [i];
    new_values[hash_val] = values[i];
  }

  if (keys != (key_t**)values)
    delete[] values;

  delete[] keys;
  delete[] hashes;

  keys   = new_keys  ;
  values = new_values;
  hashes = new_hashes;

  noccupied = nnodes;
}

/**
 * Изменение размера хэш-таблицы (если нужно)
 *
 * По сути, вызывает resize(), если таблица отклонилась
 * слишком далеко от идеального размера относительно ее числа узлов.
 *
 * Отклонение имеет место в двух случаях:
 * 1) <размер таблицы> > <число узлов> * 4 и <размер таблицы> > 8
 * 2) <размер таблицы> < <число узлов> + <число надгробий> + ( <число узлов> + <число надгробий> ) / 16
 *
 */
template < typename key_t, typename value_t >
inline void HashMap<key_t, value_t>::maybe_resize() {
  if ( ( arrays_size >  nnodes * 4 && arrays_size > 1 << HASH_TABLE_MIN_SHIFT ) ||
       ( arrays_size <= noccupied + (noccupied / 16)                          ) )
    resize();
}

/**
 * Инициализация хэш-таблицы
 */
template < typename key_t, typename value_t >
void
HashMap<key_t, value_t>::init_full (
    DestroyKeyNotify   key_destroy_function,
    DestroyValueNotify value_destroy_function )
{
  setShift( HASH_TABLE_MIN_SHIFT );
  nnodes    = 0;
  noccupied = 0;
  if (key_destroy_function)
    key_destroy_func   = key_destroy_function;
  else
    key_destroy_func   = key_single_destroyer;

  if (value_destroy_function)
    value_destroy_func = value_destroy_function;
  else
    value_destroy_func = value_single_destroyer;

  keys               = new key_t*[ arrays_size ];
  memset(keys, 0, arrays_size * sizeof(key_t*));
  values             = (value_t**)keys;
  hashes             = new uint[ arrays_size ];
  memset(hashes, 0, arrays_size * sizeof(uint));
}


template < typename key_t, typename value_t >
HashMap<key_t, value_t>::~HashMap ()
{
  clear();
  remove_all_nodes(true);

  if ( keys == (key_t**)values )
    delete[] keys;
  else {
    delete[] values;
    delete[] keys;
  }

  delete[] hashes;
}

/**
 * Поиск ключа в таблице.
 * @return Ассоциированное значение, или 0, если ключ не найден.
 */
template < typename key_t, typename value_t >
inline value_t * HashMap<key_t, value_t>::find( const key_t & key ) const
{
  uint node_index;
  uint node_hash;

  node_index = lookup_node ( key, &node_hash );

  return HASH_IS_REAL (hashes[node_index])
           ? values[node_index]
           : 0;
}

/**
 * Хэш-поиск пары
 */
template < typename key_t, typename value_t >
inline bool HashMap<key_t, value_t>::find_pair(
    const key_t & lookup_key,
    key_t      ** orig_key  ,
    value_t    *& value )
{
  uint node_index;
  uint node_hash;

  node_index = lookup_node(lookup_key, &node_hash);

  if (!HASH_IS_REAL (hashes[node_index]))
    return false;

  if (orig_key)
    *orig_key = keys[node_index];

  value = values[node_index];

  return true;
}


/**
 * Вставка значения на позиции node_index в хэш таблице или модификация его.
 */
template < typename key_t, typename value_t >
inline value_t * HashMap<key_t, value_t>::insert_node (
    uint            node_index   ,
    uint            key_hash     ,
    const key_t   & new_key_ref  ,
    const value_t * new_value_ref,
    bool          * already_exists   )
{
  uint old_hash;
  value_t * value_to_free = 0;

  old_hash = hashes[node_index];

  bool already_exists_;
  if (!already_exists)
    already_exists = &already_exists_;


  if ( ( *already_exists = HASH_IS_REAL (old_hash) ) && already_exists != &already_exists_ )
    return values[node_index];


  /*
   * Установка производится в три этапа. Сначала, разбираемся с ключем,
   * так как это самое сложное. Затем удостоверимся, что нам нужно разбить
   * таблицу на две части ( поскольку запись значения приведет тому, что
   * инвариантное множество станет испорченным ).
   * Затем разбираемся со значением.
   *
   * Здесь есть три случая для ключа:
   *
   *  - запись уже существует в таблице, использовать ключ повторно:
   *    освободить только что полученный ключ и использовать ceotcnde.ott значение
   *
   *  - запись уже существует в таблице, не использовать ключ повторно:
   *    освободить запись в таблице, использовать новый ключ
   *
   *  - запись не существует в таблице:
   *    использовать новый ключ, ничего не освобождать
   *    Одновременно с этим обновить хэш
   */
  if ( *already_exists ) {
    /* Примечание: необходимо сохранить старое значение, прежде чем записывать новый ключ
     * так как значение может измениться (в случае, когда два массива являются общими)
     */

    value_to_free = values[node_index];

    /* данный кусок подразумевал замену ключа новым*/
    /*
      if (keep_new_key) {
          key_to_free = keys[node_index];
          keys[node_index] = new_key;
      }
      else
        key_to_free = new_key;
    */
  }
  else {
    hashes[node_index] = key_hash;
    //keys[node_index] = new_key; /* данный кусок подразумевал замену ключа новым*/
    keys  [node_index] = new key_t(new_key_ref);
  }

  /*
   * Шаг второй: проверить, совпадает ли значение, которое мы собираемся записать в
   * таблицу, с ключом в той же позиции. Если нет, необходимо разбить таблицу.
   *
   * По сути хитрая проверка:
   * 1-й эл-т конъюнкции: указатель на массив ключей равен указателю на массив значений
   *   т.е. память под значения еще не выделена, и указатели на массивы лишь были созданы в конструкторе.
   * 2-й эл-т конъюнкции: программа не пытается вставить в качестве нового значения вставить тот же элемент.
   *    по сути это было бы актуально для add, если бы он сейчас поддерживался:
   *     например: (1) создаем новый ключ, массив значений еще не проинициализирован => ничего не делать
   *               (2) добавляем к этому же ключу значение, массив значений еще не проинициализирован =>
   *                     под него нужно выделить память.
   *               (3) Добавляем новое значение или ключ - массив значений уже проинициализирован => ничего не делать
   */

  value_t * new_value = new_value_ref ? new value_t(*new_value_ref) : new value_t;
  if ( keys == (key_t**)values && keys[node_index] != (key_t* )new_value )  {
    values = new value_t*[ arrays_size ];
    memcpy ( values, keys, sizeof ( key_t** )*arrays_size );
  }

  /* Шаг 3: Фактически выполнение записи */
  values[node_index] = new_value;

  if (*already_exists) {
    if ( value_destroy_func && value_to_free )
      (*value_destroy_func)(value_to_free);
  }
  else { /* регистрация новых данных (обновление счетчиков) */
    ++nnodes;
    if ( !old_hash ) { /* если old_hash не используется */

      ++noccupied; /* был заменен пустой узел и узел не являлся надгробием */
      maybe_resize();
    }
  }

  return new_value;
}


/**
 * Реализация общей логики вставки и замены элемента.
 *
 * Выполняет поиск ключа key. Если ключ найден - то соответствующее значение заменяется на новое
 * значение value. (и возможно заменяется также ключ) Если не найден - то создается новый узел.
 */
template < typename key_t, typename value_t >
inline value_t * HashMap<key_t, value_t>::insert_internal (
    const key_t   & key  ,
    const value_t * value,
    bool          * already_exist )
{
  uint key_hash;
  uint node_index = lookup_node( key, &key_hash );
  return insert_node( node_index, key_hash, key, value, already_exist);
}


/**
 * @param  notify @c true если нужно освобождать память
 * @return true если узел был найден и удален, иначе false
 *
 * Реализует общую логику для remove() и steal_remove() functions.
 *
 * Выполняет поиск ключа и если он найден, то вызывается память,
 * выделенная под узлы - освобождается.
 */
template < typename key_t, typename value_t >
inline bool HashMap<key_t, value_t>::remove_internal(
    const key_t & key, bool notify )
{
  uint node_index;
  uint node_hash;

  node_index = lookup_node( key, &node_hash );

  if ( !HASH_IS_REAL( hashes[node_index] ) )
    return false;

  remove_node ( notify, node_index );
  maybe_resize();

  return true;
}


namespace internal {

// Сделано без специализации шаблонов в силу кривсти MSVC6
template < typename HashMapType, typename key_t, typename value_t >
class Iterator {
  friend class HashMap<key_t, value_t>;
public:
  typedef HashMap < key_t, value_t > BaseHashTableType ;
  typedef Iterator< HashMap<key_t, value_t>, key_t, value_t > BaseIterType;

  typedef typename BaseHashTableType::key_type    key_type         ;
  typedef typename BaseHashTableType::value_type  value_type       ;
  typedef typename BaseHashTableType::mapped_type hash_value_type  ;
  typedef ptrdiff_t                               difference_type  ;
  typedef HashPair<key_type, hash_value_type>     mapped_type      ;
  typedef mapped_type                            *pointer          ;
  typedef mapped_type                            &reference        ;
  typedef std::forward_iterator_tag               iterator_category;

protected:
  HashMapType  *hash_table;
  int           position  ;
  bool          validStep ;
  value_type    stdpair;

  Iterator(HashMapType *_hash_table)
    : hash_table(_hash_table), position(-1), key(0), value(0) { ++*this; }

  Iterator(HashMapType *_hash_table, int)
    : hash_table(_hash_table), position(hash_table->size()), validStep(false), key(0), value(0) {}

  /**
   * Advances @iter and retrieves the key and/or value that are now
   * pointed to as a result of this advancement. If %FALSE is returned,
   * @key and @value are not set, and the iterator becomes invalid.
   */
  inline bool next();

public:
  key_type        *key;
  hash_value_type *value;

  Iterator()
    : hash_table(0), position(0), validStep(false), key(0), value(0) {}

  Iterator & operator=(const Iterator<HashMapType, key_t, value_t> &other) {
    hash_table = other.hash_table;
    position   = other.position  ;
    validStep  = other.validStep ;
    key        = other.key       ;
    value      = other.value     ;
    return *this;
  }

  inline value_type& operator*() {
    stdpair.first  = *key;
    stdpair.second = *value;
    return stdpair;
  }

  inline void              renew          () { position = -1; ++*this; }

  inline bool operator++()       { return validStep = next(); }
  inline bool valid     () const { return validStep;          }
  inline operator bool  () const { return validStep;          }

  inline bool operator==( const BaseIterType & other ) {
    return ( hash_table == other.hash_table && position == other.position );
  }

  inline bool operator!=( const BaseIterType & other ) {
    return ( hash_table != other.hash_table || position != other.position );
  }

};

template < typename HashMapType, typename key_t, typename value_t >
inline bool Iterator< HashMapType, key_t, value_t >::next ( )
{
  int _position;

  // чтобы не итерировать дальше, когда достигнут конец
  VERIFY_AND_RETURN_VAL( position < hash_table->arrays_size, false );

  _position = position;

  do
    if ( ++_position >= hash_table->arrays_size ) { // условие завершения по достижению конца
      position = _position;
      return false;
    }
  while ( !HASH_IS_REAL ( hash_table->hashes[_position] ) );

  key   = hash_table->keys  [_position];
  value = hash_table->values[_position];

  position = _position;
  return true;
}

}

inline constexpr uint emptyStringHash() { return BackportHashMap::internal::HashFunctions::hash_fun_emptystr(); }

}


COMPILER_HASH_FUN_OPTIMIZATION_POP()

#endif // HASHTABLE_INTERNAL_H
