
#include <iostream>
#include <utility>

#include <easy_iterator.h>

using integer = unsigned long long;

template <class T> class MyArray {
private:
  T * data;
  size_t size;
public:
  
  using iterator = easy_iterator::ReferenceIterator<int>;
  using const_iterator = easy_iterator::ReferenceIterator<const int>;

  explicit MyArray(size_t _size):size(_size),data(new T[_size]){ }
  MyArray(const MyArray &) = delete;
  ~MyArray(){ delete[] data; }

  int &operator[](size_t idx){ return data[idx]; }
  const T &operator[](size_t idx)const{ return data[idx]; }

  iterator begin() { return iterator(data); }
  iterator end() { return iterator(data + size); }
  const_iterator begin() const { return const_iterator(data); }
  const_iterator end() const { return const_iterator(data + size); }

};

using namespace easy_iterator;

int main(){
  MyArray<int> arr(10);

  for (auto [i, v]: enumerate(arr)) {
    v = i * i;
  }
  
  for (auto [i, v]: enumerate(std::as_const(arr))) {
    std::cout << "arr[" << i << "] = " << v << std::endl;
  }

  return 0;
}
