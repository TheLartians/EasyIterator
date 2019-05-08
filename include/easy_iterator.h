#pragma once

#include <iterator>
#include <exception>
#include <utility>
#include <tuple>
#include <functional>
#include <type_traits>

namespace easy_iterator {

  /**
   * The end state for self-contained iterators.
   */
  struct IterationEnd {
    const IterationEnd & operator*()const{ return *this; }
  };
  
  /**
   * Helper functions for comparing iterators.
   */
  namespace compare {

    struct ByValue {
      template <class T> bool operator()(const T &a, const T &b)const{ return a == b; }
    };

    struct ByAddress {
      template <class T> bool operator()(const  T &a, const  T &b)const{ return &a == &b; }
    };

    struct ByLastTupleElementMatch {
      template <typename ... ArgsA, typename ... ArgsB> bool operator()(const std::tuple<ArgsA...> & a, const std::tuple<ArgsB...> &b) const {
        static_assert(sizeof...(ArgsA) == sizeof...(ArgsB), "comparing invalid tuples");
        return std::get<sizeof...(ArgsA)-1>(a) == std::get<sizeof...(ArgsB)-1>(b);
      }
    };
    
    struct Never {
      template <class A, class B> bool operator()(const A &, const B &)const{ return false; }
    };
  }

  /**
   * Helper functions for incrementing iterators.
   */
  namespace increment {
    template <int A> struct ByValue {
      template <class T> bool operator () (T &v) const { v = v + A; return true; }
    };
    
    struct ByTupleIncrement {
      template <typename ... Args> void dummy(Args &&...) const { }
      template <class T, size_t ... Idx> void updateValues(T & v, std::index_sequence<Idx...>) const {
        dummy(++std::get<Idx>(v)...);
      }
      template <typename ... Args> bool operator()(std::tuple<Args...> & v) const {
        updateValues(v, std::make_index_sequence<sizeof...(Args)>());
        return true;
      }
    };

    template <typename T, bool(T::*Method)()> struct ByMemberCall {
      bool operator () (T &v) const { return (v.*Method)(); }
    };
    
  }

  /**
   * Helper functions for dereferencing iterators.
   */
  namespace dereference {
    struct ByValue {
      template <class T> T & operator()(T & v) const { return v; }
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
   * Implementations must define `operator++()` to update the value of `value`.
   * @param `T` - The data type held by the iterator
   * @param `D` - A functional that dereferences the data. Determines the value type of the iterator.
   * @param `C` - A function that compares two values of type `T`. Used to determine if two iterators are equal.
   */
  template <
    class T,
    typename D = dereference::ByValue,
    typename C = compare::ByValue
  > class IteratorPrototype: public std::iterator<std::input_iterator_tag, T> {
  protected:
    D dereferencer;
    C compare;
  public:
    T value;
    using value_type = decltype(dereferencer(value));
    IteratorPrototype() = delete;
    template <class F, class AD = D, class AC = C> IteratorPrototype(
      F && first,
      AD && _dereferencer = D(),
      AC && _compare = C()
    ):dereferencer(std::forward<AD>(_dereferencer)),compare(std::forward<AC>(_compare)), value(std::forward<F>(first)) { }
    value_type operator *() {
      return dereferencer(value);
    }
    auto * operator->()const{ return &**this; }
    template <typename ... Args> bool operator==(const IteratorPrototype<Args...> &other)const{
      return compare(value, other.value);
    }
    template <typename ... Args> bool operator!=(const IteratorPrototype<Args...> &other)const{
      return !operator==(other);
    }
  };
  
  template<
    class T
  > IteratorPrototype(const T &) -> IteratorPrototype<T>;

  template<
    class T,
    typename D
  > IteratorPrototype(const T &, const D &) -> IteratorPrototype<T, D>;

  template<
    class T,
    typename D,
    typename C
  > IteratorPrototype(const T &, const D &, const C &) -> IteratorPrototype<T, D, C>;

  /**
   * IteratorPrototype where advance is defined by the functional held by `F`.
   */
  template <
    class T,
    typename F = increment::ByValue<1>,
    typename D = dereference::ByValue,
    typename C = compare::ByValue
  > class Iterator final : public IteratorPrototype<T,D,C> {
  protected:
    F callback;
    bool valid = true;
    using Base = IteratorPrototype<T,D,C>;
  public:
    template <typename TT> explicit Iterator(
      TT && begin,
      const F & _callback = F(),
      const D & _dereferencer = D(),
      const C & _compare = C()
    ):IteratorPrototype<T,D,C>(std::forward<TT>(begin), _dereferencer, _compare), callback(_callback){ }
    Iterator &operator++(){
      if (valid){
        valid = callback(this->value);
      }
      return *this;
    }
    typename Base::value_type operator *() {
      if (!valid) { throw UndefinedIteratorException(); }
      return Base::dereferencer(Base::value);
    }
    using Base::operator==;
    using Base::operator!=;
    bool operator==(const IterationEnd &)const{ return !valid; }
    bool operator!=(const IterationEnd &)const{ return valid; }
  };

  template<
    class T
  > Iterator(const T &) -> Iterator<T>;

  template<
    class T,
    typename F
  > Iterator(const T &, const F &) -> Iterator<T, F>;

  template<
    class T,
    typename F,
    typename D
  > Iterator(const T &, const F &, const D &) -> Iterator<T, F, D>;

  template<
    class T,
    typename F,
    typename D,
    typename C
  > Iterator(const T &, const F &, const D &, const C &) -> Iterator<T, F, D, C>;

  /**
   * Iterates by incrementing a pointer value. Returns the dereferenced pointer.
   */
  template<class T, class A = increment::ByValue<1>> using ReferenceIterator = Iterator<
    T*,
    A,
    dereference::ByValueDereference
  >;

  /**
   * Helper class for `wrap()`.
   */
  template <class IB, class IE = IB> struct WrappedIterator {
    IB beginIterator;
    IE endIterator;
    IB begin() const { return beginIterator; }
    IE end() const { return endIterator; }
    WrappedIterator(IB && begin, IE && end):beginIterator(std::move(begin)),endIterator(std::move(end)){ }
  };

  /**
   * Wraps two iterators into a container with begin/end methods to match the C++ iterator convention.
   */
  template <class IB, class IE> WrappedIterator<IB, IE> wrap(IB && a, IE && b) {
    return WrappedIterator<IB, IE>{std::move(a), std::move(b)};
  }

  /**
   * Helper class for `range()`.
   */
  template <class T> struct RangeIterator final: public IteratorPrototype<T> {
    T increment;
    RangeIterator(const T &start, const T &_increment = 1): IteratorPrototype<T>(start), increment(_increment) {}
    RangeIterator &operator++(){ IteratorPrototype<T>::value += increment; return *this; }
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
    auto begin = Iterator(std::make_tuple(args.begin()...), increment::ByTupleIncrement(), dereference::ByTupleDereference(), compare::ByLastTupleElementMatch());
    auto end = Iterator(std::make_tuple(args.end()...), increment::ByTupleIncrement(), dereference::ByTupleDereference(), compare::ByLastTupleElementMatch());
    return wrap(std::move(begin), std::move(end));
  }

  /**
   * Returns an object that is iterated as `[index, value]`.
   */
  template <class T> auto enumerate(T && t){
    return zip(wrap(RangeIterator(0), IterationEnd()), t);
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
    using iterator = Iterator<
      T,
      increment::ByMemberCall<T, &T::advance>,
      dereference::ByMemberCall<T,decltype(std::declval<T&>().value()), &T::value>,
      compare::Never
    >;
    F initializer;
    iterator begin()const{ return iterator(initializer()); }
    IterationEnd end()const{ return IterationEnd(); }
    MakeIterable(F _initializer = F()):initializer(_initializer){ }
  };

  /**
   * Iterates over the dereferenced values between `begin` and `end`.
   */
  template <class T, class I = increment::ByValue<1>> auto ValuesBetween(T * begin, T * end) {
    return wrap(ReferenceIterator<T, I>(begin), Iterator(end));
  }
  
}
