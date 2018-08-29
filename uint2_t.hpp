#pragma once

#include <cstdint>
#include <type_traits>
#include <iterator>
#include <limits>
#include <climits>
#include <boost/compressed_pair.hpp>
#include <algorithm>
#include <cassert>

namespace hew
{

typedef std::uint8_t uint2_t;

template <typename InputIterator>
inline constexpr bool __is_input_iterator_v =
        std::is_convertible_v<typename std::iterator_traits<InputIterator>::iterator_category,
                std::input_iterator_tag>;

template <typename ForwardIterator>
inline constexpr bool __is_forward_iterator_v =
        std::is_convertible_v<typename std::iterator_traits<ForwardIterator>::iterator_category,
                std::forward_iterator_tag>;

template <typename Alloc, typename Traits = std::allocator_traits<Alloc>>
inline constexpr bool __is_noexcept_move_assign_container_v =
        Traits::propagate_on_container_move_assignment::value || Traits::is_always_equal::value;

}

namespace hew
{

template <class Cp, bool IsConst> class __uint2_iterator;
template <class Cp> class __uint2_const_reference;

template <class Cp>
class __uint2_reference
{
    typedef typename Cp::__storage_type    __storage_type;
    typedef typename Cp::__storage_pointer __storage_pointer;

    __storage_pointer    seg_;
    unsigned             shift_;

    friend typename Cp::__self;

    friend class __uint2_const_reference<Cp>;
    friend class __uint2_iterator<Cp, false>;
public:
    operator uint2_t() const noexcept
    { return static_cast<uint2_t> (*seg_ >> shift_ & 3); }

    uint2_t operator~() const noexcept
    { return static_cast<uint2_t>((*seg_ >> shift_ ^ 3) & 3); }

    __uint2_reference& operator=(uint2_t x) noexcept
    {
        *seg_ &= ~(__storage_type(3) << shift_);
        *seg_ |=   __storage_type(x) << shift_;
        return *this;
    }

    __uint2_reference& operator=(const __uint2_reference& x) noexcept
    { return operator=(static_cast<uint2_t>(x)); }

    void flip() noexcept {*seg_ ^= (__storage_type(3) << shift_);}
    __uint2_iterator<Cp, false> operator&() const noexcept
    { return __uint2_iterator<Cp, false>(seg_, shift_ / 2); }

private:
    __uint2_reference(__storage_pointer seg, unsigned pos) noexcept
        : seg_(seg), shift_(pos * 2) {}
};

template <class Cp>
class __uint2_const_reference
{
    typedef typename Cp::__storage_type          __storage_type;
    typedef typename Cp::__const_storage_pointer __storage_pointer;

    __storage_pointer    seg_;
    unsigned             shift_;

    friend typename Cp::__self;
    friend class __uint2_iterator<Cp, true>;
public:
    __uint2_const_reference(const __uint2_reference<Cp>& x) noexcept
        : seg_(x.seg_), shift_(x.shift_) {}

    operator uint2_t() const noexcept
    { return static_cast<uint2_t>(*seg_ >> shift_ & 3); }

    __uint2_iterator<Cp, true> operator&() const noexcept
    { return __uint2_iterator<Cp, true>(seg_, shift_ / 2); }

    __uint2_const_reference& operator=(const __uint2_const_reference& x) = delete;
private:
    constexpr
    __uint2_const_reference(__storage_pointer seg, unsigned pos) noexcept
        : seg_(seg), shift_(pos * 2) {}
};

template <class Cp, bool IsConst>
class __uint2_iterator
{
public:
    typedef typename Cp::difference_type                                difference_type;
    typedef uint2_t                                                     value_type;
    typedef __uint2_iterator                                            pointer;
    typedef std::conditional_t<IsConst, __uint2_const_reference<Cp>, 
                                        __uint2_reference<Cp>>          reference;
    typedef std::random_access_iterator_tag                             iterator_category;

private:
    typedef typename Cp::__storage_type                                 __storage_type;
    typedef std::conditional_t<IsConst, typename Cp::__const_storage_pointer,
                                        typename Cp::__storage_pointer> __storage_pointer;
    static const unsigned bases_per_word = Cp::bases_per_word;

    __storage_pointer seg_;
    unsigned          pos_;

public:
    __uint2_iterator() noexcept
        : seg_(nullptr), pos_(0)
    {}

    __uint2_iterator(const __uint2_iterator<Cp, false>& it) noexcept
        : seg_(it.seg_), pos_(it.pos_) {}

    reference operator*() const noexcept
    { return reference(seg_, pos_); }

    __uint2_iterator& operator++()
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

    __uint2_iterator operator++(int)
    {
        __uint2_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    __uint2_iterator& operator--()
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

    __uint2_iterator operator--(int)
    {
        __uint2_iterator tmp = *this;
        --(*this);
        return tmp;
    }

    __uint2_iterator& operator+=(difference_type n)
    {
        if (n >= 0)
            seg_ += (n + pos_) / bases_per_word;
        else
            seg_ += static_cast<difference_type>(n - bases_per_word + pos_ + 1)
                    / static_cast<difference_type>(bases_per_word);
        n &= (bases_per_word - 1);
        pos_ = static_cast<unsigned>((n + pos_) % bases_per_word);
        return *this;
    }

    __uint2_iterator& operator-=(difference_type n)
    {
        return *this += -n;
    }

    __uint2_iterator operator+(difference_type n) const
    {
        __uint2_iterator t(*this);
        t += n;
        return t;
    }

    __uint2_iterator operator-(difference_type n) const
    {
        __uint2_iterator t(*this);
        t -= n;
        return t;
    }

    friend __uint2_iterator operator+(difference_type n, const __uint2_iterator& it) { return it + n; }

    friend difference_type operator-(const __uint2_iterator& x, const __uint2_iterator& y)
    { return (x.seg_ - y.seg_) * bases_per_word + x.pos_ - y.pos_; }

    reference operator[](difference_type n) const { return *(*this + n); }

    friend bool operator==(const __uint2_iterator& x, const __uint2_iterator& y)
    { return x.seg_ == y.seg_ && x.pos_ == y.pos_; }

    friend bool operator!=(const __uint2_iterator& x, const __uint2_iterator& y)
    { return !(x == y); }

    friend bool operator<(const __uint2_iterator& x, const __uint2_iterator& y)
    { return x.seg_ < y.seg_ || (x.seg_ == y.seg_ && x.pos_ < y.pos_); }

    friend bool operator>(const __uint2_iterator& x, const __uint2_iterator& y)
    { return y < x; }

    friend bool operator<=(const __uint2_iterator& x, const __uint2_iterator& y)
    { return !(y < x); }

    friend bool operator>=(const __uint2_iterator& x, const __uint2_iterator& y)
    { return !(x < y); }

private:
    __uint2_iterator(__storage_pointer seg, unsigned pos) noexcept
        : seg_(seg), pos_(pos) {}

    friend typename Cp::__self;

    friend class __uint2_reference<Cp>;
    friend class __uint2_const_reference<Cp>;
    friend class __uint2_iterator<Cp, true>;
};

template <class Cp>
void
swap(__uint2_reference<Cp> x, __uint2_reference<Cp> y) noexcept
{
    uint2_t t = x;
    x = y;
    y = t;
}

template <class Cp, class Dp>
void
swap(__uint2_reference<Cp> x, __uint2_reference<Dp> y) noexcept
{
    uint2_t t = x;
    x = y;
    y = t;
}

template <class Cp>
void
swap(__uint2_reference<Cp> x, uint2_t& y) noexcept
{
    uint2_t t = x;
    x = y;
    y = t;
}

}

namespace hew
{

template <bool>
class __vector_base_common
{
protected:
    __vector_base_common() = default;
    void __throw_length_error() const;
    void __throw_out_of_range() const;
};

template <bool b>
void
__vector_base_common<b>::__throw_length_error() const
{
    throw std::length_error("vector");
}

template <bool b>
void
__vector_base_common<b>::__throw_out_of_range() const
{
    throw std::out_of_range("vector");
}

}

namespace std
{

template <class Allocator>
class vector<hew::uint2_t, Allocator>
    : private hew::__vector_base_common<true>
{
public:
    typedef vector                                   __self;
    typedef hew::uint2_t                             value_type;
    typedef Allocator                                allocator_type;
    typedef std::allocator_traits<allocator_type>    __alloc_traits;
    typedef typename __alloc_traits::size_type       size_type;
    typedef typename __alloc_traits::difference_type difference_type;
    typedef size_type                                __storage_type;
    typedef hew::__uint2_iterator<vector, false>     pointer;
    typedef hew::__uint2_iterator<vector, true>      const_pointer;
    typedef pointer                                  iterator;
    typedef const_pointer                            const_iterator;
    typedef std::reverse_iterator<iterator>          reverse_iterator;
    typedef std::reverse_iterator<const_iterator>    const_reverse_iterator;

private:
    typedef typename __alloc_traits::template rebind_alloc<__storage_type> __storage_allocator;
    typedef std::allocator_traits<__storage_allocator> __storage_traits;
    typedef typename __storage_traits::pointer         __storage_pointer;
    typedef typename __storage_traits::const_pointer   __const_storage_pointer;

    __storage_pointer                                      begin_;
    size_type                                              size_;
    boost::compressed_pair<size_type, __storage_allocator> cap_alloc_;
public:
    typedef hew::__uint2_reference      <vector> reference;
    typedef hew::__uint2_const_reference<vector> const_reference;
private:
    size_type& __cap() noexcept
    { return cap_alloc_.first(); }

    const size_type& __cap() const noexcept
    { return cap_alloc_.first(); }

    __storage_allocator& __alloc() noexcept
    { return cap_alloc_.second(); }

    const __storage_allocator& __alloc() const noexcept
    { return cap_alloc_.second(); }

    static const unsigned bases_per_word = static_cast<unsigned>(sizeof(__storage_type) * CHAR_BIT / 2);

    static size_type __internal_cap_to_external(size_type n) noexcept
    { return n * bases_per_word; }

    static size_type __external_cap_to_internal(size_type n) noexcept
    { return (n - 1) / bases_per_word + 1; }

public:
    vector() noexcept(std::is_nothrow_default_constructible_v<allocator_type>);

    explicit vector(const allocator_type& a) noexcept;
    ~vector();
    explicit vector(size_type n);
    explicit vector(size_type n, const allocator_type& a);
    vector(size_type n, const value_type& x);
    vector(size_type n, const value_type& x, const allocator_type& a);
    template <class InputIterator>
    vector(InputIterator first, InputIterator last,
           std::enable_if_t<hew::__is_input_iterator_v  <InputIterator> &&
                           !hew::__is_forward_iterator_v<InputIterator>>* = 0);
    template <class InputIterator>
    vector(InputIterator first, InputIterator last, const allocator_type& a,
           std::enable_if_t<hew::__is_input_iterator_v  <InputIterator> &&
                           !hew::__is_forward_iterator_v<InputIterator>>* = 0);
    template <class ForwardIterator>
    vector(ForwardIterator first, ForwardIterator last,
           std::enable_if_t<hew::__is_forward_iterator_v<ForwardIterator>>* = 0);
    template <class ForwardIterator>
    vector(ForwardIterator first, ForwardIterator last, const allocator_type& a,
           std::enable_if_t<hew::__is_forward_iterator_v<ForwardIterator>>* = 0);

    vector(const vector& v);
    vector(const vector& v, const allocator_type& a);
    vector& operator=(const vector& v);

    vector(std::initializer_list<value_type> il);
    vector(std::initializer_list<value_type> il, const allocator_type& a);

    vector(vector&& v) noexcept;
    vector(vector&& v, const allocator_type& a);
    vector& operator=(vector&& v)
    noexcept((hew::__is_noexcept_move_assign_container_v<Allocator, __alloc_traits>));

    vector& operator=(std::initializer_list<value_type> il)
    { assign(il.begin(), il.end()); return *this; }

    template <class InputIterator>
    std::enable_if_t
    <
        hew::__is_input_iterator_v  <InputIterator> &&
       !hew::__is_forward_iterator_v<InputIterator>
    >
    assign(InputIterator first, InputIterator last);
    template <class ForwardIterator>
    std::enable_if_t
    <
        hew::__is_forward_iterator_v<ForwardIterator>
    >
    assign(ForwardIterator first, ForwardIterator last);

    void assign(size_type n, const value_type& x);
    void assign(std::initializer_list<value_type> il)
    { assign(il.begin(), il.end()); }

    allocator_type get_allocator() const noexcept
    { return allocator_type(this->__alloc()); }

    size_type max_size() const noexcept;

    size_type capacity() const noexcept
    { return __internal_cap_to_external(__cap()); }

    size_type size() const noexcept
    { return size_; }

    bool empty() const noexcept
    { return size_ == 0; }
    void reserve(size_type n);
    void shrink_to_fit() noexcept;

    iterator begin() noexcept
    { return __make_iter(0); }

    const_iterator begin() const noexcept
    { return __make_iter(0); }

    iterator end() noexcept
    { return __make_iter(size_); }

    const_iterator end() const noexcept
    { return __make_iter(size_); }

    reverse_iterator rbegin() noexcept
    { return       reverse_iterator(end()); }

    const_reverse_iterator rbegin() const noexcept
    { return const_reverse_iterator(end()); }

    reverse_iterator rend() noexcept
    { return       reverse_iterator(begin()); }

    const_reverse_iterator rend()   const noexcept
    { return const_reverse_iterator(begin()); }

    const_iterator         cbegin()  const noexcept
    { return __make_iter(0); }

    const_iterator         cend()    const noexcept
    { return __make_iter(size_); }

    const_reverse_iterator crbegin() const noexcept
    { return rbegin(); }

    const_reverse_iterator crend()   const noexcept
    { return rend(); }

    reference       operator[](size_type n)       { return __make_ref(n); }
    const_reference operator[](size_type n) const { return __make_ref(n); }
    reference       at(size_type n);
    const_reference at(size_type n) const;

    reference       front()       { return __make_ref(0); }
    const_reference front() const { return __make_ref(0); }
    reference       back()        { return __make_ref(size_ - 1); }
    const_reference back()  const { return __make_ref(size_ - 1); }

    void push_back(const value_type& x);
    template <class... Args>
    reference emplace_back(Args&&... args)
    {
        push_back(value_type(std::forward<Args>(args)...));
        return this->back();
    }

    void pop_back() { --size_; }

    template <class... Args>
    iterator emplace(const_iterator position, Args&&... args)
    { return insert(position, value_type(std::forward<Args>(args)...)); }

    iterator insert(const_iterator position, const value_type& x);
    iterator insert(const_iterator position, size_type n, const value_type& x);
    template <class InputIterator>
    std::enable_if_t
    <
        hew::__is_input_iterator_v  <InputIterator> &&
       !hew::__is_forward_iterator_v<InputIterator>,
        iterator
    >
    insert(const_iterator position, InputIterator first, InputIterator last);
    template <class ForwardIterator>
    std::enable_if_t
    <
        hew::__is_forward_iterator_v<ForwardIterator>,
        iterator
    >
    insert(const_iterator position, ForwardIterator first, ForwardIterator last);

    iterator insert(const_iterator position, std::initializer_list<value_type> il)
    { return insert(position, il.begin(), il.end()); }

    iterator erase(const_iterator position);
    iterator erase(const_iterator first, const_iterator last);

    void clear() noexcept {size_ = 0;}

    void swap(vector&) noexcept;

    static void swap(reference x, reference y) noexcept { hew::swap(x, y); }

    void resize(size_type sz, value_type x = 0);
    void flip() noexcept;

    bool __invariants() const;

private:
    void __invalidate_all_iterators();
    void __vallocate(size_type n);
    void __vdeallocate() noexcept;

    static size_type __align_it(size_type new_size) noexcept
    { return new_size + (bases_per_word-1) & ~((size_type)bases_per_word-1); }
    size_type __recommend(size_type new_size) const;
    void __construct_at_end(size_type n, value_type x);
    template <class ForwardIterator>
    std::enable_if_t
    <
        hew::__is_forward_iterator_v<ForwardIterator>
    >
    __construct_at_end(ForwardIterator first, ForwardIterator last);

    reference __make_ref(size_type pos) noexcept
    { return reference(begin_ + pos / bases_per_word, static_cast<unsigned>(pos % bases_per_word)); }

    const_reference __make_ref(size_type pos) const noexcept
    { return const_reference(begin_ + pos / bases_per_word, static_cast<unsigned>(pos % bases_per_word)); }

    iterator __make_iter(size_type pos) noexcept
    { return iterator(begin_ + pos / bases_per_word, static_cast<unsigned>(pos % bases_per_word)); }

    const_iterator __make_iter(size_type pos) const noexcept
    { return const_iterator(begin_ + pos / bases_per_word, static_cast<unsigned>(pos % bases_per_word)); }

    iterator __const_iterator_cast(const_iterator p) noexcept
    { return begin() + (p - cbegin());}

    void __copy_assign_alloc(const vector& v)
    { __copy_assign_alloc(v, std::bool_constant<
            __storage_traits::propagate_on_container_copy_assignment::value>()); }

    void __copy_assign_alloc(const vector& c, std::true_type)
    {
        if (__alloc() != c.__alloc())
            __vdeallocate();
        __alloc() = c.__alloc();
    }

    void __copy_assign_alloc(const vector&, std::false_type) {}

    void __move_assign(vector& c, std::false_type);
    void __move_assign(vector& c, std::true_type)
    noexcept(std::is_nothrow_move_assignable_v<allocator_type>);

    void __move_assign_alloc(vector& c)
    noexcept(
    !__storage_traits::propagate_on_container_move_assignment::value ||
    std::is_nothrow_move_assignable_v<allocator_type>)
    {__move_assign_alloc(c, std::bool_constant<
            __storage_traits::propagate_on_container_move_assignment::value>());}

    void __move_assign_alloc(vector& c, std::true_type)
    noexcept(std::is_nothrow_move_assignable_v<allocator_type>)
    {
        __alloc() = std::move(c.__alloc());
    }

    void __move_assign_alloc(vector&, std::false_type)noexcept {}

    friend class hew::__uint2_reference<vector>;
    friend class hew::__uint2_const_reference<vector>;
    friend class hew::__uint2_iterator<vector, false>;
    friend class hew::__uint2_iterator<vector, true>;
};

template <class Allocator>
void
vector<hew::uint2_t, Allocator>::__invalidate_all_iterators()
{
}

//  Allocate space for n objects
//  throws length_error if n > max_size()
//  throws (probably bad_alloc) if memory run out
//  Precondition:  begin_ == end_ == __cap() == 0
//  Precondition:  n > 0
//  Postcondition:  capacity() == n
//  Postcondition:  size() == 0
template <class Allocator>
void
vector<hew::uint2_t, Allocator>::__vallocate(size_type n)
{
    if (n > max_size())
        this->__throw_length_error();
    n = __external_cap_to_internal(n);
    this->begin_ = __storage_traits::allocate(this->__alloc(), n);
    this->size_ = 0;
    this->__cap() = n;
}

template <class Allocator>
void
vector<hew::uint2_t, Allocator>::__vdeallocate() noexcept
{
    if (this->begin_ != nullptr)
    {
        __storage_traits::deallocate(this->__alloc(), this->begin_, __cap());
        __invalidate_all_iterators();
        this->begin_ = nullptr;
        this->size_ = this->__cap() = 0;
    }
}

template <class Allocator>
typename vector<hew::uint2_t, Allocator>::size_type
vector<hew::uint2_t, Allocator>::max_size() const noexcept
{
    size_type amax = __storage_traits::max_size(__alloc());
    size_type nmax = std::numeric_limits<size_type>::max() / 2;  // end() >= begin(), always
    if (nmax / bases_per_word <= amax)
        return nmax;
    return __internal_cap_to_external(amax);
}

//  Precondition:  new_size > capacity()
template <class Allocator>
typename vector<hew::uint2_t, Allocator>::size_type
vector<hew::uint2_t, Allocator>::__recommend(size_type new_size) const
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
template <class Allocator>
void
vector<hew::uint2_t, Allocator>::__construct_at_end(size_type n, value_type x)
{
    size_type old_size = this->size_;
    this->size_ += n;
    std::fill_n(__make_iter(old_size), n, x);
}

template <class Allocator>
template <class ForwardIterator>
std::enable_if_t
<
    hew::__is_forward_iterator_v<ForwardIterator>
>
vector<hew::uint2_t, Allocator>::__construct_at_end(ForwardIterator first, ForwardIterator last)
{
    size_type old_size = this->size_;
    this->size_ += std::distance(first, last);
    std::copy(first, last, __make_iter(old_size));
}

template <class Allocator>
vector<hew::uint2_t, Allocator>::vector()
noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
    : begin_    (nullptr),
      size_     (0),
      cap_alloc_(0)
{
}

template <class Allocator>
vector<hew::uint2_t, Allocator>::vector(const allocator_type& a) noexcept
    : begin_    (nullptr),
      size_     (0),
      cap_alloc_(0, static_cast<__storage_allocator>(a))
{
}

template <class Allocator>
vector<hew::uint2_t, Allocator>::vector(size_type n)
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

template <class Allocator>
vector<hew::uint2_t, Allocator>::vector(size_type n, const allocator_type& a)
    : begin_    (nullptr),
      size_     (0),
      cap_alloc_(0, static_cast<__storage_allocator>(a))
{
    if (n > 0)
    {
        __vallocate(n);
        __construct_at_end(n, 0);
    }
}

template <class Allocator>
vector<hew::uint2_t, Allocator>::vector(size_type n, const value_type& x)
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

template <class Allocator>
vector<hew::uint2_t, Allocator>::vector(size_type n, const value_type& x, const allocator_type& a)
    : begin_    (nullptr),
      size_     (0),
      cap_alloc_(0, static_cast<__storage_allocator>(a))
{
    if (n > 0)
    {
        __vallocate(n);
        __construct_at_end(n, x);
    }
}

template <class Allocator>
template <class InputIterator>
vector<hew::uint2_t, Allocator>::vector(InputIterator first, InputIterator last,
       std::enable_if_t<hew::__is_input_iterator_v  <InputIterator> &&
                       !hew::__is_forward_iterator_v<InputIterator>>*)
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

template <class Allocator>
template <class InputIterator>
vector<hew::uint2_t, Allocator>::vector(InputIterator first, InputIterator last, const allocator_type& a,
       std::enable_if_t<hew::__is_input_iterator_v  <InputIterator> &&
                       !hew::__is_forward_iterator_v<InputIterator>>*)
    : begin_    (nullptr),
      size_     (0),
      cap_alloc_(0, static_cast<__storage_allocator>(a))
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

template <class Allocator>
template <class ForwardIterator>
vector<hew::uint2_t, Allocator>::vector(ForwardIterator first, ForwardIterator last,
                                        std::enable_if_t<hew::__is_forward_iterator_v<ForwardIterator>>*)
    : begin_    (nullptr),
      size_     (0),
      cap_alloc_(0)
{
    auto n = static_cast<size_type>(std::distance(first, last));
    if (n > 0)
    {
        __vallocate(n);
        __construct_at_end(first, last);
    }
}

template <class Allocator>
template <class ForwardIterator>
vector<hew::uint2_t, Allocator>::vector(ForwardIterator first, ForwardIterator last, const allocator_type& a,
                                        std::enable_if_t<hew::__is_forward_iterator_v<ForwardIterator>>*)
    : begin_    (nullptr),
      size_     (0),
      cap_alloc_(0, static_cast<__storage_allocator>(a))
{
    auto n = static_cast<size_type>(std::distance(first, last));
    if (n > 0)
    {
        __vallocate(n);
        __construct_at_end(first, last);
    }
}

template <class Allocator>
vector<hew::uint2_t, Allocator>::vector(std::initializer_list<value_type> il)
    : begin_    (nullptr),
      size_     (0),
      cap_alloc_(0)
{
    auto n = static_cast<size_type>(il.size());
    if (n > 0)
    {
        __vallocate(n);
        __construct_at_end(il.begin(), il.end());
    }
}

template <class Allocator>
vector<hew::uint2_t, Allocator>::vector(std::initializer_list<value_type> il, const allocator_type& a)
    : begin_    (nullptr),
      size_     (0),
      cap_alloc_(0, static_cast<__storage_allocator>(a))
{
    auto n = static_cast<size_type>(il.size());
    if (n > 0)
    {
        __vallocate(n);
        __construct_at_end(il.begin(), il.end());
    }
}

template <class Allocator>
vector<hew::uint2_t, Allocator>::~vector()
{
    if (begin_ != nullptr)
        __storage_traits::deallocate(__alloc(), begin_, __cap());
    __invalidate_all_iterators();
}

template <class Allocator>
vector<hew::uint2_t, Allocator>::vector(const vector& v)
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

template <class Allocator>
vector<hew::uint2_t, Allocator>::vector(const vector& v, const allocator_type& a)
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

template <class Allocator>
vector<hew::uint2_t, Allocator>&
vector<hew::uint2_t, Allocator>::operator=(const vector& v)
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

template <class Allocator>
vector<hew::uint2_t, Allocator>::vector(vector&& v) noexcept
    : begin_    (v.begin_),
      size_     (v.size_),
      cap_alloc_(std::move(v.cap_alloc_))
{
    v.begin_ = nullptr;
    v.size_ = 0;
    v.__cap() = 0;
}

template <class Allocator>
vector<hew::uint2_t, Allocator>::vector(vector&& v, const allocator_type& a)
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

template <class Allocator>
vector<hew::uint2_t, Allocator>&
vector<hew::uint2_t, Allocator>::operator=(vector&& v)
noexcept((hew::__is_noexcept_move_assign_container_v<Allocator, __alloc_traits>))
{
    __move_assign(v, std::bool_constant<
            __storage_traits::propagate_on_container_move_assignment::value>());
    return *this;
}

template <class Allocator>
void
vector<hew::uint2_t, Allocator>::__move_assign(vector& c, std::false_type)
{
    if (__alloc() != c.__alloc())
        assign(c.begin(), c.end());
    else
        __move_assign(c, std::true_type());
}

template <class Allocator>
void
vector<hew::uint2_t, Allocator>::__move_assign(vector& c, std::true_type)
noexcept(std::is_nothrow_move_assignable_v<allocator_type>)
{
    __vdeallocate();
    __move_assign_alloc(c);
    this->begin_ = c.begin_;
    this->size_ = c.size_;
    this->__cap() = c.__cap();
    c.begin_ = nullptr;
    c.__cap() = c.size_ = 0;
}

template <class Allocator>
void
vector<hew::uint2_t, Allocator>::assign(size_type n, const value_type& x)
{
    size_ = 0;
    if (n > 0)
    {
        size_type c = capacity();
        if (n <= c)
            size_ = n;
        else
        {
            vector v(__alloc());
            v.reserve(__recommend(n));
            v.size_ = n;
            swap(v);
        }
        std::fill_n(begin(), n, x);
    }
    __invalidate_all_iterators();
}

template <class Allocator>
template <class InputIterator>
std::enable_if_t
<
    hew::__is_input_iterator_v  <InputIterator> &&
   !hew::__is_forward_iterator_v<InputIterator>
>
vector<hew::uint2_t, Allocator>::assign(InputIterator first, InputIterator last)
{
    clear();
    for (; first != last; ++first)
        push_back(*first);
}

template <class Allocator>
template <class ForwardIterator>
std::enable_if_t
<
    hew::__is_forward_iterator_v<ForwardIterator>
>
vector<hew::uint2_t, Allocator>::assign(ForwardIterator first, ForwardIterator last)
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

template <class Allocator>
void
vector<hew::uint2_t, Allocator>::reserve(size_type n)
{
    if (n > capacity())
    {
        vector v(this->__alloc());
        v.__vallocate(n);
        v.__construct_at_end(this->begin(), this->end());
        swap(v);
        __invalidate_all_iterators();
    }
}

template <class Allocator>
void
vector<hew::uint2_t, Allocator>::shrink_to_fit() noexcept
{
    if (__external_cap_to_internal(size()) > __cap())
    {
        try
        {
            vector(*this, allocator_type(__alloc())).swap(*this);
        }
        catch (...)
        {
        }
    }
}

template <class Allocator>
typename vector<hew::uint2_t, Allocator>::reference
vector<hew::uint2_t, Allocator>::at(size_type n)
{
    if (n >= size())
        this->__throw_out_of_range();
    return (*this)[n];
}

template <class Allocator>
typename vector<hew::uint2_t, Allocator>::const_reference
vector<hew::uint2_t, Allocator>::at(size_type n) const
{
    if (n >= size())
        this->__throw_out_of_range();
    return (*this)[n];
}

template <class Allocator>
void
vector<hew::uint2_t, Allocator>::push_back(const value_type& x)
{
    if (this->size_ == this->capacity())
        reserve(__recommend(this->size_ + 1));
    ++this->size_;
    back() = x;
}

template <class Allocator>
typename vector<hew::uint2_t, Allocator>::iterator
vector<hew::uint2_t, Allocator>::insert(const_iterator position, const value_type& x)
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
        vector v(__alloc());
        v.reserve(__recommend(size_ + 1));
        v.size_ = size_ + 1;
        r = std::copy(cbegin(), position, v.begin());
        std::copy_backward(position, cend(), v.end());
        swap(v);
    }
    *r = x;
    return r;
}

template <class Allocator>
typename vector<hew::uint2_t, Allocator>::iterator
vector<hew::uint2_t, Allocator>::insert(const_iterator position, size_type n, const value_type& x)
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
        vector v(__alloc());
        v.reserve(__recommend(size_ + n));
        v.size_ = size_ + n;
        r = std::copy(cbegin(), position, v.begin());
        std::copy_backward(position, cend(), v.end());
        swap(v);
    }
    std::fill_n(r, n, x);
    return r;
}

template <class Allocator>
template <class InputIterator>
std::enable_if_t
<
    hew::__is_input_iterator_v  <InputIterator> &&
   !hew::__is_forward_iterator_v<InputIterator>,
    typename vector<hew::uint2_t, Allocator>::iterator
>
vector<hew::uint2_t, Allocator>::insert(const_iterator position, InputIterator first, InputIterator last)
{
    difference_type off = position - begin();
    auto p = __const_iterator_cast(position);
    iterator old_end = end();
    for (; size() != capacity() && first != last; ++first)
    {
        ++this->size_;
        back() = *first;
    }
    vector v(__alloc());
    if (first != last)
    {
        try
        {
            v.assign(first, last);
            auto old_size = static_cast<difference_type>(old_end - begin());
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

template <class Allocator>
template <class ForwardIterator>
std::enable_if_t
<
    hew::__is_forward_iterator_v<ForwardIterator>,
    typename vector<hew::uint2_t, Allocator>::iterator
>
vector<hew::uint2_t, Allocator>::insert(const_iterator position, ForwardIterator first, ForwardIterator last)
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
        vector v(__alloc());
        v.reserve(__recommend(size_ + n));
        v.size_ = size_ + n;
        r = std::copy(cbegin(), position, v.begin());
        std::copy_backward(position, cend(), v.end());
        swap(v);
    }
    std::copy(first, last, r);
    return r;
}

template <class Allocator>
typename vector<hew::uint2_t, Allocator>::iterator
vector<hew::uint2_t, Allocator>::erase(const_iterator position)
{
    auto r = __const_iterator_cast(position);
    std::copy(position + 1, this->cend(), r);
    --size_;
    return r;
}

template <class Allocator>
typename vector<hew::uint2_t, Allocator>::iterator
vector<hew::uint2_t, Allocator>::erase(const_iterator first, const_iterator last)
{
    auto r = __const_iterator_cast(first);
    difference_type d = last - first;
    std::copy(last, this->cend(), r);
    size_ -= d;
    return r;
}

template <class Allocator>
void
vector<hew::uint2_t, Allocator>::swap(vector& x) noexcept
{
    std::swap(this->begin_, x.begin_);
    std::swap(this->size_, x.size_);
    std::swap(this->__cap(), x.__cap());
    if constexpr (__alloc_traits::propagate_on_container_swap::value)
        std::swap(this->__alloc(), x.__alloc());
}

template <class Allocator>
void
vector<hew::uint2_t, Allocator>::resize(size_type sz, value_type x)
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
            vector v(__alloc());
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

template <class Allocator>
void
vector<hew::uint2_t, Allocator>::flip() noexcept
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

template <class Allocator>
bool
vector<hew::uint2_t, Allocator>::__invariants() const
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

template <class Allocator>
bool
operator==(const vector<hew::uint2_t, Allocator>& x, const vector<hew::uint2_t, Allocator>& y)
{
    const typename vector<hew::uint2_t, Allocator>::size_type sz = x.size();
    return sz == y.size() && std::equal(x.begin(), x.end(), y.begin());
}

template <class Allocator>
bool
operator!=(const vector<hew::uint2_t, Allocator>& x, const vector<hew::uint2_t, Allocator>& y)
{
    return !(x == y);
}

template <class Allocator>
bool
operator< (const vector<hew::uint2_t, Allocator>& x, const vector<hew::uint2_t, Allocator>& y)
{
    return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
}

template <class Allocator>
bool
operator> (const vector<hew::uint2_t, Allocator>& x, const vector<hew::uint2_t, Allocator>& y)
{
    return y < x;
}

template <class Allocator>
bool
operator>=(const vector<hew::uint2_t, Allocator>& x, const vector<hew::uint2_t, Allocator>& y)
{
    return !(x < y);
}

template <class Allocator>
bool
operator<=(const vector<hew::uint2_t, Allocator>& x, const vector<hew::uint2_t, Allocator>& y)
{
    return !(y < x);
}

template <class Allocator>
void
swap(vector<hew::uint2_t, Allocator>& x, vector<hew::uint2_t, Allocator>& y)
noexcept(noexcept(x.swap(y)))
{
    x.swap(y);
}

}
