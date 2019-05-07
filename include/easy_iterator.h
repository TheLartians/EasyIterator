#pragma once

#include <iterator>
#include <optional>
#include <exception>

namespace easy_iterator {

  namespace compare {
    template <class T> struct ByValue {
      bool operator()(const  T &a, const  T &b)const{ return a == b; }
    };
    template <class T> struct ByAddress {
      bool operator()(const  T &a, const  T &b)const{ return &a == &b; }
    };
  }

  struct UndefinedIteratorException: public std::exception {
    const char * what()const noexcept override{ return "attempt to dereference an undefined iterator"; }
  };

  template <class T, class C = compare::ByValue<T>> class Iterator: public std::iterator<std::input_iterator_tag, T> {
  protected:
    C comparer;
    std::optional<T> current;
  public:
    Iterator() = default;
    Iterator(const std::optional<T> &first, C && _comparer = C()):comparer(std::move(_comparer)){ current = first; }
    virtual std::optional<T> next(const T &) = 0;
    T operator *()const{ if (!*this) { throw UndefinedIteratorException(); } return *current; }
    T * operator->()const{ return &**this; }
    Iterator &operator++(){ current = next(**this); return *this; }
    bool operator==(const Iterator &other)const{ return comparer(**this, *other); }
    bool operator!=(const Iterator &other)const{ return !comparer(**this, *other); }
    explicit operator bool()const{ return bool(current); }
  };

  namespace increment {
    template <class T, T A = 1> struct ByValue {
      T operator () (const T &v) const { return v + A; }
    };
    template <class R, class T = typename R::type> struct ByAddress {
      R operator () (T &v) const { return *(&v + 1); }
    };
  }

  template <class T, typename C = compare::ByValue<T>, typename A = increment::ByValue<T>> struct AdvanceIterator: public easy_iterator::Iterator<T,C> {
    A advance;
    explicit AdvanceIterator(T && begin, A && _advance = A(), C && comparer = C()):Iterator<T,C>(std::move(begin), std::move(comparer)), advance(std::forward<A>(_advance)){ }
    explicit AdvanceIterator(const T & begin, const A & _advance = A()):Iterator<T,C>(begin), advance(_advance){ }
    explicit AdvanceIterator(const std::optional<T> & begin, const A & _advance = A()):Iterator<T>(begin), advance(_advance){ }
    AdvanceIterator copyWithValue(T && value)const{ return AdvanceIterator(std::forward<T>(value), A(advance)); }
    AdvanceIterator copyWithUndefinedValue()const{ return AdvanceIterator(std::optional<T>(), A(advance)); }
    std::optional<T> next(const T &previous)final override{ return advance(previous); }
  };

  template <class I> struct WrappedIterator {
    using iterator = I;
    iterator beginIterator, endIterator;
    iterator begin() const { return beginIterator; }
    iterator end() const { return endIterator; }
    WrappedIterator(iterator && begin, iterator && end):beginIterator(std::move(begin)),endIterator(std::move(end)){ }
  };

  template <class T> auto range(T begin, T end, T increment) {
    auto incrementer = [=](const T &p){ return p + increment; };
    auto actualEnd = end - ((end - begin) % increment);
    return WrappedIterator(AdvanceIterator(begin, incrementer), AdvanceIterator(actualEnd, incrementer));
  }

  template <class T> auto range(T begin, T end) {
    return range(begin, end, 1);
  }

  template <class T> auto range(T end) {
    return range(0, end);
  }



};
