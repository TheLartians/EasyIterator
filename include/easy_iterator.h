#pragma once

#include <iterator>
#include <optional>
#include <exception>
#include <utility>

namespace easy_iterator {

  namespace compare {
    template <class T> struct ByValue {
      bool operator()(const  T &a, const  T &b)const{ return a == b; }
    };
    template <class T> struct ByAddress {
      bool operator()(const  T &a, const  T &b)const{ return &a == &b; }
    };
  }

  namespace increment {
    template <class T, int A = 1> struct ByValue {
      void operator () (std::optional<T> &v) const { *v = *v + A; }
    };
  }

  namespace dereference {
    template <class T, class R = T> struct ByValue {
      R & operator()(T & v) const { return v; }
    };
    template <class T, class R = decltype(*std::declval<T>())> struct ByDereferencedValue {
      R & operator()(T & v) const { return *v; }
    };
  }

  struct UndefinedIteratorException: public std::exception {
    const char * what()const noexcept override{ return "attempt to dereference an undefined iterator"; }
  };

  template <
    class T,
    typename D = dereference::ByValue<T>,
    typename C = compare::ByValue<T>
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
    virtual void advance(std::optional<T> &value) = 0;
    auto & operator *()const{ if (!*this) { throw UndefinedIteratorException(); } return dereferencer(*current); }
    auto * operator->()const{ return &**this; }
    Iterator &operator++(){ advance(current); return *this; }
    bool operator!=(const Iterator &other)const{ return !operator==(other); }
    bool operator==(const Iterator &other)const{
      if (!*this) { return !other; }
      if (!other) { return false; }
      return compare(*current, *other.current);
    }
    explicit operator bool()const{ return bool(current); }
  };
  
  template <
    class T,
    typename F,
    typename D = dereference::ByValue<T>,
    typename C = compare::ByValue<T>
  > class CallbackIterator: public Iterator<T,D,C> {
  protected:
    F callback;
  public:
    explicit CallbackIterator(
      const std::optional<T> & begin = std::optional<T>(),
      const F & _callback = F(),
      const D & _dereferencer = D(),
      const C & _compare = C()
    ):Iterator<T,D,C>(begin, _dereferencer, _compare), callback(_callback){ }
    void advance(std::optional<T> &value) final override {
      callback(value);
    }
  };

  template<
    class T,
    class F
  > CallbackIterator(const T &, const F &) -> CallbackIterator<T, F>;

  template<class T> using IncrementPtrIterator = CallbackIterator<
    T*,
    increment::ByValue<T*>,
    dereference::ByDereferencedValue<T*>
  >;

  template <class I> struct WrappedIterator {
    using iterator = I;
    iterator beginIterator, endIterator;
    iterator begin() const { return beginIterator; }
    iterator end() const { return endIterator; }
    WrappedIterator(iterator && begin, iterator && end):beginIterator(std::move(begin)),endIterator(std::move(end)){ }
  };

  template <class T> WrappedIterator<T> wrap(T && a, T && b) {
    return WrappedIterator{std::move(a), std::move(b)};
  }

  template <class T> auto range(T begin, T end, T increment) {
    auto incrementer = [=](std::optional<T> &p){ (*p) += increment; };
    auto actualEnd = end - ((end - begin) % increment);
    return wrap(CallbackIterator(begin, incrementer), CallbackIterator(actualEnd, incrementer));
  }

  template <class T> auto range(T begin, T end) {
    return range(begin, end, 1);
  }

  template <class T> auto range(T end) {
    return range(0, end);
  }

}
