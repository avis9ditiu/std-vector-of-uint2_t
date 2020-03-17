#ifndef BIOVOLTRON_BASE_VECTOR
#define BIOVOLTRON_BASE_VECTOR

#include <cstdint>
#include <type_traits>
#include <iterator>
#include <cassert>
#include <limits>
#include <climits>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <boost/compressed_pair.hpp>

/*

namespace biovoltron
{

class base_vector
{
 public:
  typedef unsigned char                            value_type;
  typedef std::allocator<value_type> >             allocator_type;
  typedef implementation-defined                   iterator;
  typedef implementation-defined                   const_iterator;
  typedef typename allocator_type::size_type       size_type;
  typedef typename allocator_type::difference_type difference_type;
  typedef iterator                                 pointer;
  typedef const_iterator                           const_pointer;
  typedef std::reverse_iterator<iterator>          reverse_iterator;
  typedef std::reverse_iterator<const_iterator>    const_reverse_iterator;

  class reference
  {
   public:
    reference(const reference&) noexcept;
    operator value_type() const noexcept;
    reference& operator=(const value_type x) noexcept;
    reference& operator=(const reference& x) noexcept;
    iterator operator&() const noexcept;
    void flip() noexcept;
  };

  class const_reference
  {
   public:
    const_reference(const reference&) noexcept;
    operator value_type() const noexcept;
    const_iterator operator&() const noexcept;
  };

  base_vector() noexcept(is_nothrow_default_constructible<allocator_type>::value);
  explicit base_vector(const allocator_type&);
  explicit base_vector(size_type n, const allocator_type& a = allocator_type()); // C++14
  base_vector(size_type n, const value_type& value, const allocator_type& = allocator_type());
  base_vector(std::input_iterator auto first, std::input_iterator auto last, const allocator_type& = allocator_type());
  base_vector(const base_vector& x);
  base_vector(base_vector&& x)
  noexcept(is_nothrow_move_constructible<allocator_type>::value);
  base_vector(initializer_list<value_type> il);
  base_vector(initializer_list<value_type> il, const allocator_type& a);
  ~base_vector();
  base_vector& operator=(const base_vector& x);
  base_vector& operator=(base_vector&& x) noexcept;
  base_vector& operator=(initializer_list<value_type> il);
  void assign(std::input_iterator auto first, std::input_iterator auto last);
  void assign(size_type n, const value_type& u);
  void assign(initializer_list<value_type> il);

  allocator_type get_allocator() const noexcept;

  iterator begin() noexcept;
  const_iterator begin() const noexcept;
  iterator end() noexcept;
  const_iterator end() const noexcept;

  reverse_iterator rbegin() noexcept;
  const_reverse_iterator rbegin() const noexcept;
  reverse_iterator rend() noexcept;
  const_reverse_iterator rend() const noexcept;

  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;
  const_reverse_iterator crbegin() const noexcept;
  const_reverse_iterator crend() const noexcept;

  size_type size() const noexcept;
  size_type max_size() const noexcept;
  size_type capacity() const noexcept;
  bool empty() const noexcept;
  void reserve(size_type n);
  void shrink_to_fit() noexcept;

  reference operator[](size_type n);
  const_reference operator[](size_type n) const;
  reference at(size_type n);
  const_reference at(size_type n) const;

  reference front();
  const_reference front() const;
  reference back();
  const_reference back() const;

  storage_pointer data() noexcept;
  const storage_pointer data() const noexcept;

  void push_back(const value_type& x);
  template <class... Args>
  reference emplace_back(Args&& ... args);
  void pop_back();

  template <class... Args>
  iterator emplace(const_iterator position, Args&& ... args);
  iterator insert(const_iterator position, const value_type& x);
  iterator insert(const_iterator position, size_type n, const value_type& x);
  iterator insert(const_iterator position, std::input_iterator auto first, std::input_iterator auto last);
  iterator insert(const_iterator position, initializer_list<value_type> il);

  iterator erase(const_iterator position);
  iterator erase(const_iterator first, const_iterator last);

  void clear() noexcept;

  void resize(size_type sz);
  void resize(size_type sz, value_type x);
  void swap(base_vector&) noexcept;
  void flip() noexcept;

  bool __invariants() const;
};

bool operator== (const base_vector& x, const base_vector& y);
auto operator<=>(const base_vector& x, const base_vector& y);

void swap(base_vector& x, base_vector& y)
    noexcept(noexcept(x.swap(y)));

}  // biovoltron

*/

namespace biovoltron
{

template <class Cp, bool IsConst, typename Cp::__storage_type = 0> class base_iterator;
template <class Cp> class base_const_reference;

template <class Cp>
class base_reference
{
  typedef typename Cp::__storage_type    __storage_type;
  typedef typename Cp::__storage_pointer __storage_pointer;

  friend typename Cp::__self;
  friend class base_const_reference<Cp>;
  friend class base_iterator<Cp, false>;

  __storage_pointer seg_;
  unsigned shift_;
 public:
  operator unsigned char()  const noexcept {return  *seg_ >> shift_ & 3;}
  unsigned char operator~() const noexcept {return (*seg_ >> shift_ ^ 3) & 3;}

  base_reference& operator=(unsigned char x) noexcept
  {
    *seg_ &= ~(__storage_type(3) << shift_);
    *seg_ |=   __storage_type(x) << shift_;
    return *this;
  }

  base_reference& operator=(const base_reference& x) noexcept
  {return operator=(static_cast<unsigned char>(x));}

  void flip() noexcept {*seg_ ^= (__storage_type(3) << shift_);}
  base_iterator<Cp, false> operator&() const noexcept {return base_iterator<Cp, false>(seg_, shift_ / 2);}
 private:
  base_reference(__storage_pointer seg, unsigned pos) noexcept : seg_(seg), shift_(pos * 2) {}
};

template <class Cp>
class base_const_reference
{
  typedef typename Cp::__const_storage_pointer __storage_pointer;
  friend typename Cp::__self;
  friend class base_iterator<Cp, true>;

  __storage_pointer seg_;
  unsigned shift_;
 public:
  base_const_reference(const base_reference<Cp>& x) noexcept : seg_(x.seg_), shift_(x.shift_) {}
  base_const_reference& operator=(const base_const_reference& x) = delete;

  operator unsigned char() const noexcept {return *seg_ >> shift_ & 3;}
  base_iterator<Cp, true> operator&() const noexcept {return base_iterator<Cp, true>(seg_, shift_ / 2);}
 private:
  constexpr base_const_reference(__storage_pointer seg, unsigned pos) noexcept : seg_(seg), shift_(pos * 2) {}
};

template <class Cp, bool IsConst, typename Cp::__storage_type>
class base_iterator
{
 public:
  typedef typename Cp::difference_type                                               difference_type;
  typedef unsigned char                                                              value_type;
  typedef base_iterator                                                              pointer;
  typedef std::conditional_t<IsConst, base_const_reference<Cp>, base_reference<Cp> > reference;
  typedef std::random_access_iterator_tag                                            iterator_category;
 private:
  typedef std::conditional_t<
    IsConst, typename Cp::__const_storage_pointer, typename Cp::__storage_pointer> __storage_pointer;
  static const unsigned bases_per_word = Cp::bases_per_word;

  __storage_pointer seg_;
  unsigned pos_;
 public:
  base_iterator() noexcept : seg_(nullptr), pos_(0) {}
  base_iterator(const base_iterator<Cp, false>& it) noexcept : seg_(it.seg_), pos_(it.pos_) {}

  reference operator*() const noexcept {return reference(seg_, pos_);}
  reference operator[](difference_type n) const {return *(*this + n);}

  base_iterator& operator++()
  {
    if (pos_ != bases_per_word - 1)
      ++pos_;
    else 
    {
      pos_ = 0;
      ++seg_;
    }
    return *this;
  }

  base_iterator operator++(int)
  {
    base_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  base_iterator& operator--()
  {
    if (pos_ != 0)
      --pos_;
    else 
    {
      pos_ = bases_per_word - 1;
      --seg_;
    }
    return *this;
  }

  base_iterator operator--(int)
  {
    base_iterator tmp = *this;
    --(*this);
    return tmp;
  }

  base_iterator& operator+=(difference_type n)
  {
    if (n >= 0) seg_ += (n + pos_) / bases_per_word;
    else seg_ += static_cast<difference_type>(n - bases_per_word + pos_ + 1) /
        static_cast<difference_type>(bases_per_word);
    n &= (bases_per_word - 1);
    pos_ = static_cast<unsigned>((n + pos_) % bases_per_word);
    return *this;
  }

  base_iterator& operator-=(difference_type n) {return *this += -n;}

  base_iterator operator+(difference_type n) const
  {
    base_iterator t(*this);
    t += n;
    return t;
  }

  base_iterator operator-(difference_type n) const
  {
    base_iterator t(*this);
    t -= n;
    return t;
  }

  friend base_iterator operator+(difference_type n, const base_iterator& it) {return it + n;}
  friend difference_type operator-(const base_iterator& x, const base_iterator& y)
  {return (x.seg_ - y.seg_) * bases_per_word + x.pos_ - y.pos_;}

  friend bool operator==(const base_iterator& x, const base_iterator& y)
  {return x.seg_ == y.seg_ && x.pos_ == y.pos_;}
  friend bool operator!=(const base_iterator& x, const base_iterator& y) {return !(x == y);}
  friend bool operator< (const base_iterator& x, const base_iterator& y)
  {return x.seg_ < y.seg_ || (x.seg_ == y.seg_ && x.pos_ < y.pos_);}
  friend bool operator> (const base_iterator& x, const base_iterator& y) {return y < x;}
  friend bool operator<=(const base_iterator& x, const base_iterator& y) {return !(y < x);}
  friend bool operator>=(const base_iterator& x, const base_iterator& y) {return !(x < y);}

 private:
  base_iterator(__storage_pointer seg, unsigned pos) noexcept : seg_(seg), pos_(pos) {}

  friend typename Cp::__self;
  friend class base_reference<Cp>;
  friend class base_const_reference<Cp>;
  friend class base_iterator<Cp, true>;
};

template <class Cp>
void swap(base_reference<Cp> x, base_reference<Cp> y) noexcept
{
  unsigned char t = x;
  x = y;
  y = t;
}

template <class Cp, class Dp>
void swap(base_reference<Cp> x, base_reference<Dp> y) noexcept
{
  unsigned char t = x;
  x = y;
  y = t;
}

template <class Cp>
void swap(base_reference<Cp> x, unsigned char& y) noexcept
{
  unsigned char t = x;
  x = y;
  y = t;
}

template <bool>
class __base_vector_base_common
{
 protected:
  __base_vector_base_common() = default;
  void __throw_length_error() const;
  void __throw_out_of_range() const;
};

template <bool b>
void __base_vector_base_common<b>::__throw_length_error() const
{throw std::length_error("base_vector");}

template <bool b>
void __base_vector_base_common<b>::__throw_out_of_range() const
{throw std::out_of_range("base_vector");}

class base_vector : private __base_vector_base_common<true>
{
 public:
  typedef base_vector                              __self;
  typedef unsigned char                            value_type;
  typedef std::allocator<value_type>               allocator_type;
  typedef std::allocator_traits<allocator_type>    __alloc_traits;
  typedef typename __alloc_traits::size_type       size_type;
  typedef typename __alloc_traits::difference_type difference_type;
  typedef size_type                                __storage_type;
  typedef base_iterator<base_vector, false>        pointer;
  typedef base_iterator<base_vector, true>         const_pointer;
  typedef pointer                                  iterator;
  typedef const_pointer                            const_iterator;
  typedef std::reverse_iterator<iterator>          reverse_iterator;
  typedef std::reverse_iterator<const_iterator>    const_reverse_iterator;

  static constexpr unsigned bases_per_word = static_cast<unsigned>(sizeof(__storage_type) * CHAR_BIT / 2);
 private:
  typedef typename __alloc_traits::template rebind_alloc<__storage_type> __storage_allocator;
  typedef std::allocator_traits<__storage_allocator>                     __storage_traits;
  typedef typename __storage_traits::pointer                             __storage_pointer;
  typedef typename __storage_traits::const_pointer                       __const_storage_pointer;

  __storage_pointer                                      begin_;
  size_type                                              size_;
  boost::compressed_pair<size_type, __storage_allocator> cap_alloc_;
 public:
  typedef base_reference      <base_vector> reference;
  typedef base_const_reference<base_vector> const_reference;
 private:
  size_type&       __cap()       noexcept {return cap_alloc_.first();}
  const size_type& __cap() const noexcept {return cap_alloc_.first();}
  __storage_allocator&       __alloc()       noexcept {return cap_alloc_.second();}
  const __storage_allocator& __alloc() const noexcept {return cap_alloc_.second();}

  static size_type __internal_cap_to_external(size_type n) noexcept {return n * bases_per_word;}
  static size_type __external_cap_to_internal(size_type n) noexcept {return (n - 1) / bases_per_word + 1;}
 public:
  base_vector() noexcept(std::is_nothrow_default_constructible_v<allocator_type>);
  explicit base_vector(const allocator_type& a) noexcept;
  ~base_vector();
  explicit base_vector(size_type n);
  explicit base_vector(size_type n, const allocator_type& a);
  base_vector(size_type n, const value_type& x);
  base_vector(size_type n, const value_type& x, const allocator_type& a);

  base_vector(std::input_iterator auto first, std::input_iterator auto last);
  base_vector(std::input_iterator auto first, std::input_iterator auto last, const allocator_type& a);
  base_vector(std::forward_iterator auto first, std::forward_iterator auto last);
  base_vector(std::forward_iterator auto first, std::forward_iterator auto last, const allocator_type& a);

  base_vector(const base_vector& v);
  base_vector(const base_vector& v, const allocator_type& a);
  base_vector& operator=(const base_vector& v);

  base_vector(std::initializer_list<value_type> il);
  base_vector(std::initializer_list<value_type> il, const allocator_type& a);

  base_vector(base_vector&& v) noexcept;
  base_vector(base_vector&& v, const allocator_type& a);

  base_vector& operator=(base_vector&& v) noexcept;
  base_vector& operator=(std::initializer_list<value_type> il)
  {assign(il.begin(), il.end()); return *this;}

  void assign(std::input_iterator auto first, std::input_iterator auto last);
  void assign(std::forward_iterator auto first, std::forward_iterator auto last);

  void assign(size_type n, const value_type& x);
  void assign(std::initializer_list<value_type> il) {assign(il.begin(), il.end());}

  allocator_type get_allocator() const noexcept {return allocator_type(this->__alloc());}

  size_type max_size() const noexcept;
  size_type capacity() const noexcept {return __internal_cap_to_external(__cap());}
  size_type size() const noexcept {return size_;}
  bool empty() const noexcept {return size_ == 0;}
  void reserve(size_type n);
  void shrink_to_fit() noexcept;

  iterator                 begin()       noexcept {return __make_iter(0);}
  const_iterator           begin() const noexcept {return __make_iter(0);}
  iterator                   end()       noexcept {return __make_iter(size_);}
  const_iterator             end() const noexcept {return __make_iter(size_);}
  reverse_iterator        rbegin()       noexcept {return reverse_iterator(end());}
  const_reverse_iterator  rbegin() const noexcept {return const_reverse_iterator(end());}
  reverse_iterator          rend()       noexcept {return reverse_iterator(begin());}
  const_reverse_iterator    rend() const noexcept {return const_reverse_iterator(begin());}
  const_iterator          cbegin() const noexcept {return __make_iter(0);}
  const_iterator            cend() const noexcept {return __make_iter(size_);}
  const_reverse_iterator crbegin() const noexcept {return rbegin();}
  const_reverse_iterator   crend() const noexcept {return rend();}

  reference       operator[](size_type n)       {return __make_ref(n);}
  const_reference operator[](size_type n) const {return __make_ref(n);}
  reference       at(size_type n);
  const_reference at(size_type n) const;

  reference       front()       {return __make_ref(0);}
  const_reference front() const {return __make_ref(0);}
  reference       back()        {return __make_ref(size_ - 1);}
  const_reference back()  const {return __make_ref(size_ - 1);}

  __storage_pointer       data()       noexcept {return begin_;}
  const __storage_pointer data() const noexcept {return begin_;}

  void push_back(const value_type& x);
  template <class... Args>
  reference emplace_back(Args&&... args) {
    push_back(value_type(std::forward<Args>(args)...));
    return this->back();
  }

  void pop_back() {--size_;}

  template <class... Args>
  iterator emplace(const_iterator position, Args&&... args)
  {return insert(position, value_type(std::forward<Args>(args)...));}

  iterator insert(const_iterator position, const value_type& x);
  iterator insert(const_iterator position, size_type n, const value_type& x);
  iterator insert(const_iterator position, std::input_iterator auto first, std::input_iterator auto last);
  iterator insert(const_iterator position, std::forward_iterator auto first, std::forward_iterator auto last);
  iterator insert(const_iterator position, std::initializer_list<value_type> il)
  {return insert(position, il.begin(), il.end());}

  iterator erase(const_iterator position);
  iterator erase(const_iterator first, const_iterator last);

  void clear() noexcept {size_ = 0;}

  void swap(base_vector&) noexcept;
  static void swap(reference x, reference y) noexcept {biovoltron::swap(x, y);}

  void resize(size_type sz, value_type x = 0);
  void flip() noexcept;

  bool __invariants() const;

 private:
  void __invalidate_all_iterators();
  void __vallocate(size_type n);
  void __vdeallocate() noexcept;

  static size_type __align_it(size_type new_size) noexcept
  {return (new_size + (bases_per_word-1)) & ~((size_type)bases_per_word-1);}
  size_type __recommend(size_type new_size) const;
  void __construct_at_end(size_type n, value_type x);
  void __construct_at_end(std::forward_iterator auto first, std::forward_iterator auto last);

  reference       __make_ref (size_type pos)       noexcept {return reference      (begin_ + pos / bases_per_word, pos % bases_per_word);}
  const_reference __make_ref (size_type pos) const noexcept {return const_reference(begin_ + pos / bases_per_word, pos % bases_per_word);}
  iterator        __make_iter(size_type pos)       noexcept {return iterator       (begin_ + pos / bases_per_word, pos % bases_per_word);}
  const_iterator  __make_iter(size_type pos) const noexcept {return const_iterator (begin_ + pos / bases_per_word, pos % bases_per_word);}

  iterator __const_iterator_cast(const_iterator p) noexcept {return begin() + (p - cbegin());}

  void __copy_assign_alloc(const base_vector& v)
  {
    __copy_assign_alloc(v, std::bool_constant<__storage_traits::propagate_on_container_copy_assignment::value>());
  }

  void __copy_assign_alloc(const base_vector& c, std::true_type)
  {
    if (__alloc() != c.__alloc())
      __vdeallocate();
    __alloc() = c.__alloc();
  }

  void __copy_assign_alloc(const base_vector&, std::false_type) {}

  void __move_assign(base_vector& c, std::false_type);
  void __move_assign(base_vector& c, std::true_type) noexcept(std::is_nothrow_move_assignable_v<allocator_type>);
  void __move_assign_alloc(base_vector& c) noexcept(std::is_nothrow_move_assignable_v<allocator_type>)
  {__move_assign_alloc(c, std::bool_constant<__storage_traits::propagate_on_container_move_assignment::value>());}
  void __move_assign_alloc(base_vector& c, std::true_type) noexcept(
  std::is_nothrow_move_assignable_v<allocator_type>) {__alloc() = c.__alloc();}
  void __move_assign_alloc(base_vector&, std::false_type) noexcept {}

  friend class base_reference<base_vector>;
  friend class base_const_reference<base_vector>;
  friend class base_iterator<base_vector, false>;
  friend class base_iterator<base_vector, true>;
};

void base_vector::__invalidate_all_iterators()
{
}

//  Allocate space for n objects
//  throws length_error if n > max_size()
//  throws (probably bad_alloc) if memory run out
//  Precondition:  begin_ == end_ == __cap() == 0
//  Precondition:  n > 0
//  Postcondition:  capacity() == n
//  Postcondition:  size() == 0
void base_vector::__vallocate(size_type n)
{
  if (n > max_size())
    this->__throw_length_error();
  n = __external_cap_to_internal(n);
  this->begin_ = __storage_traits::allocate(this->__alloc(), n);
  this->size_ = 0;
  this->__cap() = n;
}

void base_vector::__vdeallocate() noexcept
{
  if (this->begin_ != nullptr)
  {
    __storage_traits::deallocate(this->__alloc(), this->begin_, __cap());
    __invalidate_all_iterators();
    this->begin_ = nullptr;
    this->size_ = this->__cap() = 0;
  }
}

typename base_vector::size_type
base_vector::max_size() const noexcept
{
  size_type amax = __storage_traits::max_size(__alloc());
  size_type nmax = std::numeric_limits<size_type>::max() / 2;  // end() >= begin(), always
  if (nmax / bases_per_word <= amax)
    return nmax;
  return __internal_cap_to_external(amax);
}

//  Precondition:  new_size > capacity()
typename base_vector::size_type
base_vector::__recommend(size_type new_size) const
{
  const size_type ms = max_size();
  if (new_size > ms)
    this->__throw_length_error();
  const size_type cap = capacity();
  if (cap >= ms / 2)
    return ms;
  return std::max(2*cap, __align_it(new_size));
}

//  Default constructs n objects starting at end_
//  Precondition:  n > 0
//  Precondition:  size() + n <= capacity()
//  Postcondition:  size() == size() + n
void base_vector::__construct_at_end(size_type n, value_type x)
{
  size_type old_size = this->size_;
  this->size_ += n;
  std::fill_n(__make_iter(old_size), n, x);
}

void base_vector::__construct_at_end(std::forward_iterator auto first, std::forward_iterator auto last)
{
  size_type old_size = this->size_;
  this->size_ += std::distance(first, last);
  std::copy(first, last, __make_iter(old_size));
}

base_vector::base_vector() noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0)
{
}

base_vector::base_vector(const allocator_type& a) noexcept
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0, a)
{
}

base_vector::base_vector(size_type n)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0)
{
  if (n > 0)
  {
    __vallocate(n);
    __construct_at_end(n, 0);
  }
}

base_vector::base_vector(size_type n, const allocator_type& a)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0, a)
{
  if (n > 0)
  {
    __vallocate(n);
    __construct_at_end(n, 0);
  }
}

base_vector::base_vector(size_type n, const value_type& x)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0)
{
  if (n > 0)
  {
    __vallocate(n);
    __construct_at_end(n, x);
  }
}

base_vector::base_vector(size_type n, const value_type& x, const allocator_type& a)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0, a)
{
  if (n > 0)
  {
    __vallocate(n);
    __construct_at_end(n, x);
  }
}

base_vector::base_vector(std::input_iterator auto first, std::input_iterator auto last)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0)
{
  try
  {
    for (; first != last; ++first)
      push_back(*first);
  }
  catch (...)
  {
    if (begin_ != nullptr)
      __storage_traits::deallocate(__alloc(), begin_, __cap());
    __invalidate_all_iterators();
    throw;
  }
}

base_vector::base_vector(std::input_iterator auto first, std::input_iterator auto last, const allocator_type& a)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0, a)
{
  try
  {
    for (; first != last; ++first)
      push_back(*first);
  }
  catch (...)
  {
    if (begin_ != nullptr)
      __storage_traits::deallocate(__alloc(), begin_, __cap());
    __invalidate_all_iterators();
    throw;
  }
}

base_vector::base_vector(std::forward_iterator auto first, std::forward_iterator auto last)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0)
{
  const auto n = static_cast<size_type>(std::distance(first, last));
  if (n > 0)
  {
    __vallocate(n);
    __construct_at_end(first, last);
  }
}

base_vector::base_vector(std::forward_iterator auto first, std::forward_iterator auto last, const allocator_type& a)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0, a)
{
  const auto n = static_cast<size_type>(std::distance(first, last));
  if (n > 0)
  {
    __vallocate(n);
    __construct_at_end(first, last);
  }
}

base_vector::base_vector(std::initializer_list<value_type> il)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0)
{
  const auto n = static_cast<size_type>(il.size());
  if (n > 0)
  {
    __vallocate(n);
    __construct_at_end(il.begin(), il.end());
  }
}

base_vector::base_vector(std::initializer_list<value_type> il, const allocator_type& a)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0, a)
{
  const auto n = static_cast<size_type>(il.size());
  if (n > 0)
  {
    __vallocate(n);
    __construct_at_end(il.begin(), il.end());
  }
}

base_vector::~base_vector()
{
  if (begin_ != nullptr)
    __storage_traits::deallocate(__alloc(), begin_, __cap());
  __invalidate_all_iterators();
}

base_vector::base_vector(const base_vector& v)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0, __storage_traits::select_on_container_copy_construction(v.__alloc()))
{
  if (v.size() > 0)
  {
    __vallocate(v.size());
    __construct_at_end(v.begin(), v.end());
  }
}

base_vector::base_vector(const base_vector& v, const allocator_type& a)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0, a)
{
  if (v.size() > 0)
  {
    __vallocate(v.size());
    __construct_at_end(v.begin(), v.end());
  }
}

base_vector& base_vector::operator=(const base_vector& v)
{
  if (this != &v)
  {
    __copy_assign_alloc(v);
    if (v.size_)
    {
      if (v.size_ > capacity())
      {
        __vdeallocate();
        __vallocate(v.size_);
      }
      std::copy(v.begin_, v.begin_ + __external_cap_to_internal(v.size_), begin_);
    }
    size_ = v.size_;
  }
  return *this;
}

base_vector::base_vector(base_vector&& v) noexcept
  : begin_    (v.begin_),
    size_     (v.size_),
    cap_alloc_(v.cap_alloc_)
{
  v.begin_ = nullptr;
  v.size_ = 0;
  v.__cap() = 0;
}

base_vector::base_vector(base_vector&& v, const allocator_type& a)
  : begin_    (nullptr),
    size_     (0),
    cap_alloc_(0, a)
{
  if (a == allocator_type(v.__alloc()))
  {
    this->begin_ = v.begin_;
    this->size_ = v.size_;
    this->__cap() = v.__cap();
    v.begin_ = nullptr;
    v.__cap() = v.size_ = 0;
  }
  else if (v.size() > 0)
  {
    __vallocate(v.size());
    __construct_at_end(v.begin(), v.end());
  }
}

base_vector& base_vector::operator=(base_vector&& v) noexcept
{
  __move_assign(v, std::bool_constant<
    __storage_traits::propagate_on_container_move_assignment::value>());
  return *this;
}

void base_vector::__move_assign(base_vector& c, std::false_type)
{
  if (__alloc() != c.__alloc())
    assign(c.begin(), c.end());
  else
    __move_assign(c, std::true_type());
}

void base_vector::__move_assign(base_vector& c, std::true_type) noexcept(
std::is_nothrow_move_assignable_v<allocator_type>)
{
  __vdeallocate();
  __move_assign_alloc(c);
  this->begin_ = c.begin_;
  this->size_ = c.size_;
  this->__cap() = c.__cap();
  c.begin_ = nullptr;
  c.__cap() = c.size_ = 0;
}

void base_vector::assign(size_type n, const value_type& x)
{
  size_ = 0;
  if (n > 0)
  {
    size_type c = capacity();
    if (n <= c)
      size_ = n;
    else
    {
      base_vector v(__alloc());
      v.reserve(__recommend(n));
      v.size_ = n;
      swap(v);
    }
    std::fill_n(begin(), n, x);
  }
  __invalidate_all_iterators();
}

void base_vector::assign(std::input_iterator auto first, std::input_iterator auto last)
{
  clear();
  for (; first != last; ++first)
    push_back(*first);
}

void base_vector::assign(std::forward_iterator auto first, std::forward_iterator auto last)
{
  clear();
  difference_type ns = std::distance(first, last);
  assert(ns >= 0 && "invalid range specified");
  const auto n = static_cast<size_type>(ns);
  if (n)
  {
    if (n > capacity())
    {
      __vdeallocate();
      __vallocate(n);
    }
    __construct_at_end(first, last);
  }
}

void base_vector::reserve(size_type n)
{
  if (n > capacity())
  {
    base_vector v(this->__alloc());
    v.__vallocate(n);
    v.__construct_at_end(this->begin(), this->end());
    swap(v);
    __invalidate_all_iterators();
  }
}

void base_vector::shrink_to_fit() noexcept
{
  if (__external_cap_to_internal(size()) > __cap())
  {
    try
    {
      base_vector(*this, allocator_type(__alloc())).swap(*this);
    }
    catch (...)
    {
    }
  }
}

typename base_vector::reference base_vector::at(size_type n)
{
  if (n >= size())
    this->__throw_out_of_range();
  return (*this)[n];
}

typename base_vector::const_reference base_vector::at(size_type n) const
{
  if (n >= size())
    this->__throw_out_of_range();
  return (*this)[n];
}

void base_vector::push_back(const value_type& x)
{
  if (this->size_ == this->capacity())
    reserve(__recommend(this->size_ + 1));
  ++this->size_;
  back() = x;
}

typename base_vector::iterator base_vector::insert(const_iterator position, const value_type& x)
{
  iterator r;
  if (size() < capacity())
  {
    const_iterator old_end = end();
    ++size_;
    std::copy_backward(position, old_end, end());
    r = __const_iterator_cast(position);
  }
  else
  {
    base_vector v(__alloc());
    v.reserve(__recommend(size_ + 1));
    v.size_ = size_ + 1;
    r = std::copy(cbegin(), position, v.begin());
    std::copy_backward(position, cend(), v.end());
    swap(v);
  }
  *r = x;
  return r;
}

typename base_vector::iterator base_vector::insert(const_iterator position, size_type n, const value_type& x)
{
  iterator r;
  size_type c = capacity();
  if (n <= c && size() <= c - n)
  {
    const_iterator old_end = end();
    size_ += n;
    std::copy_backward(position, old_end, end());
    r = __const_iterator_cast(position);
  }
  else
  {
    base_vector v(__alloc());
    v.reserve(__recommend(size_ + n));
    v.size_ = size_ + n;
    r = std::copy(cbegin(), position, v.begin());
    std::copy_backward(position, cend(), v.end());
    swap(v);
  }
  std::fill_n(r, n, x);
  return r;
}

typename base_vector::iterator base_vector::insert(const_iterator position, std::input_iterator auto first, std::input_iterator auto last)
{
  difference_type off = position - begin();
  iterator p = __const_iterator_cast(position);
  iterator old_end = end();
  for (; size() != capacity() && first != last; ++first)
  {
    ++this->size_;
    back() = *first;
  }
  base_vector v(__alloc());
  if (first != last)
  {
    try
    {
      v.assign(first, last);
      difference_type old_size = static_cast<difference_type>(old_end - begin());
      difference_type old_p = p - begin();
      reserve(__recommend(size() + v.size()));
      p = begin() + old_p;
      old_end = begin() + old_size;
    }
    catch (...)
    {
      erase(old_end, end());
      throw;
    }
  }
  p = std::rotate(p, old_end, end());
  insert(p, v.begin(), v.end());
  return begin() + off;
}

typename base_vector::iterator base_vector::insert(const_iterator position, std::forward_iterator auto first, std::forward_iterator auto last)
{
  const difference_type n_signed = std::distance(first, last);
  assert(n_signed >= 0 && "invalid range specified");
  const auto n = static_cast<size_type>(n_signed);
  iterator r;
  size_type c = capacity();
  if (n <= c && size() <= c - n)
  {
    const_iterator old_end = end();
    size_ += n;
    std::copy_backward(position, old_end, end());
    r = __const_iterator_cast(position);
  }
  else
  {
    base_vector v(__alloc());
    v.reserve(__recommend(size_ + n));
    v.size_ = size_ + n;
    r = std::copy(cbegin(), position, v.begin());
    std::copy_backward(position, cend(), v.end());
    swap(v);
  }
  std::copy(first, last, r);
  return r;
}

typename base_vector::iterator base_vector::erase(const_iterator position)
{
  iterator r = __const_iterator_cast(position);
  std::copy(position + 1, this->cend(), r);
  --size_;
  return r;
}

typename base_vector::iterator base_vector::erase(const_iterator first, const_iterator last)
{
  iterator r = __const_iterator_cast(first);
  difference_type d = last - first;
  std::copy(last, this->cend(), r);
  size_ -= d;
  return r;
}

void base_vector::swap(base_vector& x) noexcept
{
  std::swap(this->begin_, x.begin_);
  std::swap(this->size_, x.size_);
  std::swap(this->__cap(), x.__cap());
}

void base_vector::resize(size_type sz, value_type x)
{
  size_type cs = size();
  if (cs < sz)
  {
    iterator r;
    size_type c = capacity();
    size_type n = sz - cs;
    if (n <= c && cs <= c - n)
    {
      r = end();
      size_ += n;
    }
    else
    {
      base_vector v(__alloc());
      v.reserve(__recommend(size_ + n));
      v.size_ = size_ + n;
      r = std::copy(cbegin(), cend(), v.begin());
      swap(v);
    }
    std::fill_n(r, n, x);
  }
  else
    size_ = sz;
}

void base_vector::flip() noexcept
{
  // do middle whole words
  size_type n = size_;
  __storage_pointer p = begin_;
  for (; n >= bases_per_word; ++p, n -= bases_per_word)
    *p = ~*p;

  // do last partial word
  if (n > 0)
  {
    __storage_type m = ~__storage_type(0) >> (bases_per_word - n);
    __storage_type b = *p & m;
    *p &= ~m;
    *p |= ~b & m;
  }
}

bool base_vector::__invariants() const
{
  if (this->begin_ == nullptr)
  {
    if (this->size_ != 0 || this->__cap() != 0)
      return false;
  }
  else
  {
    if (this->__cap() == 0)
      return false;
    if (this->size_ > this->capacity())
      return false;
  }
  return true;
}

bool operator==(const base_vector& x, const base_vector& y)
{return x.size() == y.size() && std::equal(x.begin(), x.end(), y.begin());}

auto operator<=>(const base_vector& x, const base_vector& y)
{return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end());}

void swap(base_vector& x, base_vector& y) noexcept(noexcept(x.swap(y))) {x.swap(y);}

}

#endif //BIOVOLTRON_BASE_VECTOR
