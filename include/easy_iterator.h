#pragma once

#include <iterator>
#include <optional>
#include <exception>
#include <utility>
#include <tuple>
#include <functional>
#include <type_traits>

namespace easy_iterator {

  /**
   * Helper functions for comparing iterators.
   */
  namespace compare {

    struct ByValue {
      template <class T> bool operator()(const  T &a, const  T &b)const{ return a == b; }
    };

    struct ByAddress {
      template <class T> bool operator()(const  T &a, const  T &b)const{ return &a == &b; }
    };

    struct ByLastTupleElementMatch {
      template <typename ... Args> bool operator()(const std::tuple<Args...> & a, const std::tuple<Args...> &b) const { 
        return std::get<sizeof...(Args)-1>(a) == std::get<sizeof...(Args)-1>(b);
      }
    };
    
    struct Never {
      template <class T> bool operator()(const  T &, const  T &)const{ return false; }
    };
  }

  /**
   * Helper functions for incrementing iterators.
   */
  namespace increment {
    template <int A> struct ByValue {
      template <class T> void operator () (std::optional<T> &v) const { *v = *v + A; }
    };

    struct ByTupleIncrement {
      template <typename ... Args> void dummy(Args &&...) const { }
      template <typename ... Args, size_t ... Idx> auto getReferenceTuple(std::tuple<Args...> & v, std::index_sequence<Idx...>) const {
        dummy(++std::get<Idx>(v)...);
      }
      template <typename ... Args> auto operator()(std::optional<std::tuple<Args...>> & v) const { 
        return getReferenceTuple(*v, std::make_index_sequence<sizeof...(Args)>());
      }
    };

    template <typename T, bool(T::*Method)()> struct ByMemberCall {
      void operator () (std::optional<T> &v) const { if(!((*v).*Method)()) v.reset(); }
    };
    
  }

  /**
   * Helper functions for dereferencing iterators.
   */
  namespace dereference {
    struct ByValue {
      template <class T> auto & operator()(T & v) const { return v; }
    };

    struct ByValueDereference {
      template <class T> auto & operator()(T & v) const { return *v; }
    };

    struct ByTupleDereference {
      template <size_t Idx, class T> auto constexpr getElement(T & v) const {
        if constexpr (std::is_reference<decltype(*std::get<Idx>(v))>::value) {
          return std::reference_wrapper(*std::get<Idx>(v));
        } else {
          return *std::get<Idx>(v);
        }
      }
      template <size_t ... Idx, class T> auto getReferenceTuple(T & v, std::index_sequence<Idx...>) const {
          return std::make_tuple(getElement<Idx>(v)...);
      }
      template <typename ... Args> auto operator()(std::tuple<Args...> & v) const { 
        return getReferenceTuple(v, std::make_index_sequence<sizeof...(Args)>());
      }
    };

    template <typename T, typename R, R(T::*Method)()> struct ByMemberCall {
      R operator () (T &v) const { return (v.*Method)(); }
    };
    
  }

  /**
   * Exception when dereferencing an undefined iterator value.
   */
  struct UndefinedIteratorException: public std::exception {
    const char * what()const noexcept override{ return "attempt to dereference an undefined iterator"; }
  };

  /**
   * Base class for simple iterators. Takes several template parameters.
   * Implementations must define `operator++()` to update the value of `current`.
   * @param `T` - The data type held by the iterator
   * @param `D` - A functional that dereferences the data. Determines the value type of the iterator.
   * @param `C` - A function that compares two values of type `T`. Used to determine if two iterators are equal.
   */
  template <
    class T,
    typename D = dereference::ByValue,
    typename C = compare::ByValue
  > class Iterator: public std::iterator<std::input_iterator_tag, T> {
  protected:
    D dereferencer;
    C compare;
    mutable std::optional<T> current;
  public:
    Iterator() = default;
    Iterator(
      const std::optional<T> &first,
      const D & _dereferencer = D(),
      const C & _compare = C()
    ):dereferencer(_dereferencer),compare(_compare),current(first){ }
    decltype(dereferencer(*current)) operator *()const{
      if (!*this) { throw UndefinedIteratorException(); }
      return dereferencer(*current);
    }
    auto * operator->()const{ return &**this; }
    inline Iterator &operator++(){ advance(current); return *this; }
    bool operator!=(const Iterator &other)const{ return !operator==(other); }
    bool operator==(const Iterator &other)const{
      if (!*this) { return !other; }
      if (!other) { return false; }
      return compare(*current, *other.current);
    }
    explicit operator bool()const{ return bool(current); }
  };
  
  /**
   * Iterator where advance is defined by the functional held by `F`.
   */
  template <
    class T,
    typename F,
    typename D = dereference::ByValue,
    typename C = compare::ByValue
  > class CallbackIterator final : public Iterator<T,D,C> {
  protected:
    F callback;
  public:
    explicit CallbackIterator(
      const std::optional<T> & begin = std::optional<T>(),
      const F & _callback = F(),
      const D & _dereferencer = D(),
      const C & _compare = C()
    ):Iterator<T,D,C>(begin, _dereferencer, _compare), callback(_callback){ }
    inline CallbackIterator &operator++(){ callback(*this->current); return *this; }
  };

  template<
    class T,
    typename F
  > CallbackIterator(const T &, const F &) -> CallbackIterator<T, F>;

  template<
    class T,
    typename F,
    typename D
  > CallbackIterator(const T &, const F &, const D &) -> CallbackIterator<T, F, D>;

  template<
    class T,
    typename F,
    typename D,
    typename C
  > CallbackIterator(const T &, const F &, const D &, const C &) -> CallbackIterator<T, F, D, C>;

  /**
   * An iterator for pointer-based arrays.
   */
  template<class T> using IncrementPtrIterator = CallbackIterator<
    T*,
    increment::ByValue<1>,
    dereference::ByValueDereference
  >;

  /**
   * Helper class for `wrap()`.
   */
  template <class I> struct WrappedIterator {
    using iterator = I;
    iterator beginIterator, endIterator;
    iterator begin() const { return beginIterator; }
    iterator end() const { return endIterator; }
    WrappedIterator(iterator && begin, iterator && end):beginIterator(std::move(begin)),endIterator(std::move(end)){ }
  };

  /**
   * Wraps two iterators into a container with begin/end methods to match the C++ iterator convention.
   */
  template <class T> WrappedIterator<T> wrap(T && a, T && b) {
    return WrappedIterator<T>{std::move(a), std::move(b)};
  }

  /**
   * Helper class for `range()`.
   */
  template <class T> struct RangeIterator final: public Iterator<T> {
    T increment;
    RangeIterator(const T &start, const T &_increment = 1): Iterator<T>(start), increment(_increment) {}
    inline RangeIterator &operator++(){ *this->current += increment; return *this; }
  };

  /**
   * Returns an iterator that increases it's value from `begin` to the first value <= `end` by `increment` for each step.
   */
  template <class T> auto range(T begin, T end, T increment) {
    auto actualEnd = end - ((end - begin) % increment);
    return wrap(RangeIterator(begin, increment), RangeIterator(actualEnd, increment));
  }

  /**
   * Returns an iterator that increases it's value from `begin` to `end` by `1` for each step.
   */
  template <class T> auto range(T begin, T end) {
    return range<T>(begin, end, 1);
  }

  /**
   * Returns an iterator that increases it's value from `0` to `end` by `1` for each step.
   */
  template <class T> auto range(T end) {
    return range<T>(0, end);
  }

  /**
   * Wrappes the `rbegin` and `rend` iterators.
   */
  template <class T> auto reverse(T & v) {
    return wrap(v.rbegin(), v.rend());
  }

  /**
   * Returns an iterable object where all argument iterators are traversed simultaneously.
   */
  template <typename ... Args> auto zip(Args && ... args){
    auto begin = CallbackIterator(std::make_tuple(args.begin()...), increment::ByTupleIncrement(), dereference::ByTupleDereference(), compare::ByLastTupleElementMatch());
    auto end = CallbackIterator(std::make_tuple(args.end()...), increment::ByTupleIncrement(), dereference::ByTupleDereference(), compare::ByLastTupleElementMatch());
    return wrap(std::move(begin), std::move(end));
  }

  /**
   * Returns an object that is iterated as `[index, value]`.
   */
  template <class T> auto enumerate(T && t){
    return zip(wrap(RangeIterator(0),RangeIterator(0)), t);
  }
  
  namespace make_iterable_detail {
    template <class T> struct DefaultConstructor {
      T operator()()const{ return T(); }
    };
  }
  
  /**
   * Take a class `T` with that defines the methods `bool T::advance()` and `O T::value()` for any type `O`
   * and wraps it into an iterable class. The additional template parameter `F` may be used to define the
   * constuctor used to create `T` instances and is passed to the constructor of `MakeIterable<T,F>`.
   */
  template <class T, typename F = make_iterable_detail::DefaultConstructor<T>> struct MakeIterable {
    using iterator = CallbackIterator<
      T,
      increment::ByMemberCall<T, &T::advance>,
      dereference::ByMemberCall<T,decltype(std::declval<T&>().value()), &T::value>,
      compare::Never
    >;
    F initializer;
    iterator begin()const{ return iterator(initializer()); }
    iterator end()const{ return iterator(); }
    MakeIterable(F _initializer = F()):initializer(_initializer){ }
  };

}
