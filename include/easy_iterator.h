#pragma once

#include <exception>
#include <functional>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>

namespace easy_iterator {

  /**
   * The end state for self-contained iterators.
   */
  struct IterationEnd {
    const IterationEnd &operator*() const { return *this; }
  };

  /**
   * Helper functions for comparing iterators.
   */
  namespace compare {

    struct ByValue {
      template <class T> bool operator()(const T &a, const T &b) const { return a == b; }
    };

    struct ByAddress {
      template <class T> bool operator()(const T &a, const T &b) const { return &a == &b; }
    };

    struct ByLastTupleElementMatch {
      template <typename... ArgsA, typename... ArgsB>
      bool operator()(const std::tuple<ArgsA...> &a, const std::tuple<ArgsB...> &b) const {
        static_assert(sizeof...(ArgsA) == sizeof...(ArgsB), "comparing invalid tuples");
        return std::get<sizeof...(ArgsA) - 1>(a) == std::get<sizeof...(ArgsB) - 1>(b);
      }
    };

    struct Never {
      template <class A, class B> bool operator()(const A &, const B &) const { return false; }
    };
  }  // namespace compare

  /**
   * Helper functions for incrementing iterators.
   */
  namespace increment {
    template <int A> struct ByValue {
      template <class T> void operator()(T &v) const { v = v + A; }
    };

    struct ByTupleIncrement {
      template <typename... Args> void dummy(Args &&...) {}
      template <class T, size_t... Idx> void updateValues(T &v, std::index_sequence<Idx...>) {
        dummy(++std::get<Idx>(v)...);
      }
      template <typename... Args> void operator()(std::tuple<Args...> &v) {
        updateValues(v, std::make_index_sequence<sizeof...(Args)>());
      }
    };

    template <typename T, typename M, M Method> struct ByMemberCall {
      using R = decltype((std::declval<T &>().*Method)());
      R operator()(T &v) const { return (v.*Method)(); }
    };

  }  // namespace increment

  /**
   * Helper functions for dereferencing iterators.
   */
  namespace dereference {
    struct ByValue {
      template <class T> T operator()(T &v) const { return v; }
    };

    struct ByConstValueReference {
      template <class T> const T &operator()(T &v) const { return v; }
    };

    struct ByValueReference {
      template <class T> T &operator()(T &v) const { return v; }
    };

    struct ByValueDereference {
      template <class T> auto &operator()(T &v) const { return *v; }
    };

    struct ByTupleDereference {
      template <size_t Idx, class T> auto constexpr getElement(T &v) const {
        if constexpr (std::is_reference<decltype(*std::get<Idx>(v))>::value) {
          return std::reference_wrapper(*std::get<Idx>(v));
        } else {
          return *std::get<Idx>(v);
        }
      }
      template <size_t... Idx, class T>
      auto getReferenceTuple(T &v, std::index_sequence<Idx...>) const {
        return std::make_tuple(getElement<Idx>(v)...);
      }
      template <typename... Args> auto operator()(std::tuple<Args...> &v) const {
        return getReferenceTuple(v, std::make_index_sequence<sizeof...(Args)>());
      }
    };

    template <typename T, typename M, M Method> struct ByMemberCall {
      using R = decltype((std::declval<T &>().*Method)());
      R operator()(T &v) const { return (v.*Method)(); }
    };

  }  // namespace dereference

  /**
   * Exception when dereferencing an undefined iterator value.
   */
  struct UndefinedIteratorException : public std::exception {
    const char *what() const noexcept override {
      return "attempt to dereference an undefined iterator";
    }
  };

  /**
   * Base class for simple iterators. Takes several template parameters.
   * Implementations must define `operator++()` to update the value of `value`.
   * @param `T` - The data type held by the iterator
   * @param `D` - A functional that dereferences the data. Determines the value type of the
   * iterator.
   * @param `C` - A function that compares two values of type `T`. Used to determine if two
   * iterators are equal.
   */
  template <class T, typename D = dereference::ByValueReference, typename C = compare::ByValue>
  class IteratorPrototype {
  public:
    // iterator traits
    using iterator_category = std::input_iterator_tag;
    using reference = decltype(std::declval<D>()(std::declval<T &>()));
    using value_type = typename std::decay<reference>::type;
    using pointer = void;
    using difference_type = void;

  protected:
    D dereferencer;
    C compare;

  public:
    T value;
    using DereferencedType = decltype(dereferencer(value));

    IteratorPrototype() = delete;
    template <class F, class AD = D, class AC = C>
    explicit IteratorPrototype(F &&first, AD &&_dereferencer = D(), AC &&_compare = C())
        : dereferencer(std::forward<AD>(_dereferencer)),
          compare(std::forward<AC>(_compare)),
          value(std::forward<F>(first)) {}

    DereferencedType operator*() { return dereferencer(value); }
    auto *operator-> () const { return &**this; }
    template <typename... Args> bool operator==(const IteratorPrototype<Args...> &other) const {
      return compare(value, other.value);
    }
    template <typename... Args> bool operator!=(const IteratorPrototype<Args...> &other) const {
      return !operator==(other);
    }
  };

  template <class T> IteratorPrototype(const T &)->IteratorPrototype<T>;

  template <class T, typename D> IteratorPrototype(const T &, const D &)->IteratorPrototype<T, D>;

  template <class T, typename D, typename C>
  IteratorPrototype(const T &, const D &, const C &)->IteratorPrototype<T, D, C>;

  namespace iterator_detail {
    struct WithState {
      constexpr static bool hasState = true;
      bool state = true;
    };
    struct WithoutState {
      constexpr static bool hasState = false;
    };

    template <class F, class T> static constexpr bool needsState
        = !std::is_same<void, decltype(std::declval<F>()(std::declval<T &>()))>::value;
  }  // namespace iterator_detail

  /**
   * IteratorPrototype where advance is defined by the functional held by `F`.
   */
  template <class T, typename F = increment::ByValue<1>, typename D = dereference::ByValueReference,
            typename C = compare::ByValue>
  class Iterator final
      : public IteratorPrototype<T, D, C>,
        public std::conditional<iterator_detail::needsState<F, T>, iterator_detail::WithState,
                                iterator_detail::WithoutState>::type {
  protected:
    using Base = IteratorPrototype<T, D, C>;
    F callback;

  public:
    template <typename TT, typename TF = F, typename TD = D, typename TC = C>
    explicit Iterator(TT &&begin, TF &&_callback = F(), TD &&_dereferencer = D(),
                      TC &&_compare = C())
        : IteratorPrototype<T, D, C>(std::forward<TT>(begin), std::forward<TD>(_dereferencer),
                                     std::forward<TC>(_compare)),
          callback(_callback) {}
    Iterator &operator++() {
      if constexpr (Iterator::hasState) {
        if (Iterator::state) {
          Iterator::state = callback(Base::value);
        }
      } else {
        callback(Base::value);
      }
      return *this;
    }
    typename Base::DereferencedType operator*() {
      if constexpr (Iterator::hasState) {
        if (!Iterator::state) {
          throw UndefinedIteratorException();
        }
      }
      return Base::dereferencer(Base::value);
    }
    using Base::operator==;
    using Base::operator!=;
    bool operator==(const IterationEnd &other) const { return !operator!=(other); }
    bool operator!=(const IterationEnd &) const {
      if constexpr (Iterator::hasState) {
        return Iterator::state;
      } else {
        return true;
      }
    }
    explicit operator bool() const {
      if constexpr (Iterator::hasState) {
        return Iterator::state;
      } else {
        return true;
      }
    }
  };

  template <class T> Iterator(const T &)->Iterator<T>;

  template <class T, typename F> Iterator(const T &, const F &)->Iterator<T, F>;

  template <class T, typename F, typename D>
  Iterator(const T &, const F &, const D &)->Iterator<T, F, D>;

  template <class T, typename F, typename D, typename C>
  Iterator(const T &, const F &, const D &, const C &)->Iterator<T, F, D, C>;

  template <class T, typename F = increment::ByValue<1>, typename D = dereference::ByValueReference,
            typename C = compare::ByValue>
  Iterator<T, F, D, C> makeIterator(T &&t, F f = F(), D &&d = D(), C &&c = C()) {
    return Iterator<T, F, D, C>(t, f, d, c);
  }

  /**
   * Iterates by incrementing a pointer value. Returns the dereferenced pointer.
   */
  template <class T, class A = increment::ByValue<1>> using ReferenceIterator
      = Iterator<T *, A, dereference::ByValueDereference>;

  /**
   * Helper class for `wrap()`.
   */
  template <class IB, class IE = IB> struct WrappedIterator {
    mutable IB beginIterator;
    mutable IE endIterator;
    IB &&begin() const { return std::move(beginIterator); }
    IE &&end() const { return std::move(endIterator); }
    WrappedIterator(IB &&begin, IE &&end)
        : beginIterator(std::move(begin)), endIterator(std::move(end)) {}
  };

  /**
   * Wraps two iterators into a single-use container with begin/end methods to match the C++
   * iterator convention.
   */
  template <class IB, class IE> auto wrap(IB &&a, IE &&b) {
    return WrappedIterator<IB, IE>(std::forward<IB>(a), std::forward<IE>(b));
  }

  /**
   * Helper class for `range()`.
   */
  template <class T> struct RangeIterator : public IteratorPrototype<T, dereference::ByValue> {
    T increment;

    RangeIterator(const T &start, const T &_increment = 1)
        : IteratorPrototype<T, dereference::ByValue>(start), increment(_increment) {}

    RangeIterator &operator++() {
      RangeIterator::value += increment;
      return *this;
    }
  };

  template <class T> RangeIterator<T> rangeValue(T v, T i = 1) { return RangeIterator<T>(v, i); }

  /**
   * Returns an iterator that increases it's value from `begin` to the first value <= `end` by
   * `increment` for each step.
   */
  template <class T> auto range(T begin, T end, T increment) {
    auto actualEnd = end - ((end - begin) % increment);
    return wrap(rangeValue(begin, increment), rangeValue(actualEnd, increment));
  }

  /**
   * Returns an iterator that increases it's value from `begin` to `end` by `1` for each step.
   */
  template <class T> auto range(T begin, T end) { return range<T>(begin, end, 1); }

  /**
   * Returns an iterator that increases it's value from `0` to `end` by `1` for each step.
   */
  template <class T> auto range(T end) { return range<T>(0, end); }

  /**
   * Wrappes the `rbegin` and `rend` iterators.
   */
  template <class T> auto reverse(T &v) { return wrap(v.rbegin(), v.rend()); }

  /**
   * Returns an iterable object where all argument iterators are traversed simultaneously.
   * Behaviour is undefined if the iterators do not have the same length.
   */
  template <typename... Args> auto zip(Args &&... args) {
    auto begin = Iterator(std::make_tuple(args.begin()...), increment::ByTupleIncrement(),
                          dereference::ByTupleDereference(), compare::ByLastTupleElementMatch());
    auto end = Iterator(std::make_tuple(args.end()...), increment::ByTupleIncrement(),
                        dereference::ByTupleDereference(), compare::ByLastTupleElementMatch());
    return wrap(std::move(begin), std::move(end));
  }

  /**
   * Returns an object that is iterated as `[index, value]`.
   */
  template <class T> auto enumerate(T &&t) {
    return zip(wrap(RangeIterator<size_t>(0), IterationEnd()), t);
  }

  /**
   * When used as a base class for a iterator type, `MakeIterable` will call the `bool init()`
   * member before iteration. If `init()` returns false, the iterator is empty.
   */
  struct InitializedIterable {};

  /**
   * Take a class `T` with that defines the methods `T::advance()` and `O T::value()` for any type
   * `O` and wraps it into a single-use iterable class. The return value of `T::advance()` is used
   * to indicate the state of the iterator.
   */
  template <class T> struct MakeIterable {
    mutable Iterator<T, increment::ByMemberCall<T, decltype(&T::advance), &T::advance>,
                     dereference::ByMemberCall<T, decltype(&T::value), &T::value>, compare::ByValue>
        start;

    auto &&begin() const {
      if constexpr (std::is_base_of<InitializedIterable, T>::value) {
        start.state = start.value.init();
      }
      return std::move(start);
    }
    auto end() const { return IterationEnd(); }

    explicit MakeIterable(T &&value) : start(std::move(value)) {}
    template <typename... Args> explicit MakeIterable(Args &&... args)
        : start(T(std::forward<Args>(args)...)) {}
  };

  /**
   * Iterates over the dereferenced values between `begin` and `end`.
   */
  template <class T, class I = increment::ByValue<1>> auto valuesBetween(T *begin, T *end) {
    return wrap(ReferenceIterator<T, I>(begin), Iterator(end));
  }

  /**
   * copy-assigns the given value to every element in a container
   */
  template <class T, class A> void fill(A &arr, const T &value) {
    for (auto &v : arr) {
      v = value;
    }
  }

  /**
   * copies values from one container to another.
   * @param `a` - the container with values to be copies.
   * @param `b` - the target container.
   * @param `f` (optional) - a function to transform values before copying.
   * Behaviour is undefined if `a` and `b` do not have the same size.
   */
  template <class A, class B, class T = dereference::ByValueReference>
  void copy(const A &a, B &b, T &&t = T()) {
    for (auto [v1, v2] : zip(a, b)) {
      v2 = t(v1);
    }
  }

  /**
   * Returns a pointer to the value if found, otherwise `nullptr`.
   * Usage: `if(auto v = found(map.find(key), map)) { do_something(v); }`
   */
  template <class I, class C> decltype(&*std::declval<I>()) found(const I &it, C &container) {
    if (it != container.end()) {
      return &*it;
    } else {
      return nullptr;
    }
  }

  /**
   * Removes a value from a container with `find` method.
   * Usage: `eraseIfFound(map.find(key), map);`
   */
  template <class I, class C> bool eraseIfFound(const I &it, C &container) {
    if (it != container.end()) {
      container.erase(it);
      return true;
    } else {
      return false;
    }
  }

  /**
   * Returns a pointer to the value if found, otherwise `nullptr`.
   * Usage: `if(auto v = find(map, key)) { do_something(v); }`
   */
  template <class C, class V> auto find(C &c, V &&v) {
    auto it = c.find(v);
    return found(it, c);
  }

}  // namespace easy_iterator
