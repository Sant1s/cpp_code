#include <iostream>
#include <vector>

const size_t kChunkSize = 512;

template <typename T, typename Allocator = std::allocator<T>>
class Deque {
 public:
  Deque();
  Deque(const Allocator& alloc);
  Deque(const Deque& other);
  Deque(size_t count, const Allocator& alloc = Allocator());
  Deque(size_t elem_number, const T& default_elem,
        const Allocator& alloc = Allocator());
  Deque(Deque&& other);
  Deque(std::initializer_list<T> init, const Allocator& alloc = Allocator());
  ~Deque() { clear(); }
  Deque& operator=(const Deque& other);
  Deque& operator=(Deque&& other);

  size_t size() const { return end_ - begin_; }
  bool empty() const { return size() == 0; }
  T& operator[](size_t idx) { return begin_[idx]; }
  const T& operator[](size_t idx) const { return begin_[idx]; }
  T& at(size_t idx) {
    if (idx >= size()) {
      throw std::out_of_range("out_of_range");
    }
    return begin_[idx];
  }
  const T& at(size_t idx) const {
    if (idx >= size()) {
      throw std::out_of_range("out_of_range");
    }
    return begin_[idx];
  }

  void push_back(const T& elem);
  void push_back(T&& elem);
  void push_front(const T& elem);
  void push_front(T&& elem);
  void pop_back() {
    --end_;
    alloc_traits::destroy(alloc_, &*end_);
  }
  void pop_front() {
    alloc_traits::destroy(alloc_, &*begin_);
    ++begin_;
  }
  Allocator& get_allocator() { return alloc_; }

  template <bool IsConst>
  class CommonIterator
      : public std::iterator<std::random_access_iterator_tag,
                             std::conditional_t<IsConst, const T, T>> {
   public:
    CommonIterator() = default;
    CommonIterator(CommonIterator<!IsConst> const& other)
        : curr_elem_(other.curr_elem_),
          curr_chunk_(other.curr_chunk_),
          curr_chunk_begin_(other.curr_chunk_begin_),
          curr_chunk_end_(other.curr_chunk_end_) {}
    CommonIterator<IsConst>& operator++();
    CommonIterator<IsConst>& operator--();
    CommonIterator<IsConst> operator++(int);
    CommonIterator<IsConst> operator--(int);
    CommonIterator<IsConst> operator+(int64_t idx) const;
    CommonIterator<IsConst> operator-(int64_t idx) const;
    std::ptrdiff_t operator-(const CommonIterator<IsConst>& rhs) const;
    std::conditional_t<IsConst, const T&, T&> operator*() const {
      return *curr_elem_;
    }
    std::conditional_t<IsConst, const T*, T*> operator->() const {
      return curr_elem_;
    }
    bool operator==(const CommonIterator& rhs) const;
    bool operator!=(const CommonIterator& rhs) const { return !(rhs == *this); }
    bool operator<(const CommonIterator& rhs) const;
    bool operator>(const CommonIterator& rhs) const { return rhs < *this; }
    bool operator<=(const CommonIterator& rhs) const { return !(rhs < *this); }
    bool operator>=(const CommonIterator& rhs) const { return !(*this < rhs); }
    CommonIterator<IsConst>& operator+=(int64_t idx);

   private:
    friend class Deque;
    void set_chunk(T** new_chunk);
    std::conditional_t<IsConst, const T&, T&> operator[](size_t idx) const {
      return *(*this + idx);
    }
    T** curr_chunk_ = nullptr;
    T* curr_elem_ = nullptr;
    T* curr_chunk_end_ = nullptr;
    T* curr_chunk_begin_ = nullptr;
  };

  CommonIterator<false> begin() const { return begin_; }
  CommonIterator<false> end() const { return end_; }
  CommonIterator<true> cbegin() const { return const_iterator(begin_); }
  CommonIterator<true> cend() const { return const_iterator(end_); }
  std::reverse_iterator<CommonIterator<false>> rbegin() const {
    return std::reverse_iterator<iterator>(end_);
  }
  std::reverse_iterator<CommonIterator<false>> rend() const {
    return std::reverse_iterator<iterator>(begin_);
  }
  std::reverse_iterator<CommonIterator<true>> crbegin() const {
    return std::reverse_iterator<CommonIterator<true>>(cend());
  }
  std::reverse_iterator<CommonIterator<true>> crend() const {
    return std::reverse_iterator<CommonIterator<true>>(cbegin());
  }

  using iterator = CommonIterator<false>;
  using const_iterator = CommonIterator<true>;
  using reverse_iterator = std::reverse_iterator<CommonIterator<false>>;
  using const_reverse_iterator = std::reverse_iterator<CommonIterator<true>>;

  template <typename... Args>
  void emplace(iterator iter, Args&&... args);

  template <typename... Args>
  void emplace_back(Args&&... args);

  template <typename... Args>
  void emplace_front(Args&&... args);

  void insert(iterator&& iter, T elem);
  void erase(iterator&& iter);
  using alloc_traits = std::allocator_traits<Allocator>;

 private:
  void clear();
  void resize(size_t count);
  std::vector<T*> map_;
  size_t cap_begin_;
  size_t cap_end_;
  iterator begin_;
  iterator end_;
  Allocator alloc_;
};

template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace(Deque::iterator iter, Args&&... args) {
  if (iter == end_) {
    emplace_back(args...);
    return;
  }
  emplace_back(args...);
  iterator tmp_iter = end() - 1;
  while (tmp_iter != iter) {
    std::swap(*tmp_iter, *(tmp_iter - 1));
    --tmp_iter;
  }
}

template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_back(Args&&... args) {
  if (cap_end_ == map_.size() - 1 &&
      end_.curr_elem_ + 1 == end_.curr_chunk_end_) {
    resize(map_.size() * 3);
    map_[++cap_end_] = alloc_traits::allocate(alloc_, kChunkSize);
    try {
      alloc_traits::construct(alloc_, end_.curr_elem_,
                              std::forward<Args>(args)...);
    } catch (...) {
      throw;
    }
    end_.set_chunk(&map_[cap_end_]);
    end_.curr_elem_ = end_.curr_chunk_begin_;
    return;
  }
  if (end_.curr_elem_ + 1 == end_.curr_chunk_end_) {
    if (cap_end_ != map_.size()) {
      ++cap_end_;
      map_[cap_end_] = alloc_traits::allocate(alloc_, kChunkSize);
    }
    try {
      alloc_traits::construct(alloc_, end_.curr_elem_,
                              std::forward<Args>(args)...);
      ++end_;
    } catch (...) {
      throw;
    }
    return;
  }
  try {
    alloc_traits::construct(alloc_, end_.curr_elem_,
                            std::forward<Args>(args)...);
    ++end_;
  } catch (...) {
    throw;
  }
}

template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_front(Args&&... args) {
  if (cap_begin_ == 0 && begin_.curr_elem_ == begin_.curr_chunk_begin_) {
    resize(map_.size() * 3);
    try {
      --begin_;
      alloc_traits::construct(alloc_, begin_.curr_elem_,
                              std::forward<Args>(args)...);
    } catch (...) {
      ++begin_;
      throw;
    }
    return;
  }
  if (begin_.curr_elem_ == begin_.curr_chunk_begin_) {
    --cap_begin_;
    map_[cap_begin_] = alloc_traits::allocate(alloc_, kChunkSize);
    try {
      --begin_;
      alloc_traits::construct(alloc_, begin_.curr_elem_,
                              std::forward<Args>(args)...);
    } catch (...) {
      ++begin_;
      throw;
    }
    return;
  }
  try {
    --begin_;
    alloc_traits::construct(alloc_, begin_.curr_elem_,
                            std::forward<Args>(args)...);
  } catch (...) {
    ++begin_;
    throw;
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::insert(Deque::iterator&& iter, T elem) {
  if (iter == end_) {
    push_back(elem);
    return;
  }
  push_back(elem);
  iterator tmp_iter = end() - 1;
  while (tmp_iter != iter) {
    std::swap(*tmp_iter, *(tmp_iter - 1));
    --tmp_iter;
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::erase(Deque::iterator&& iter) {
  if (iter == end()) {
    pop_back();
    return;
  }
  while (iter + 1 != end_) {
    std::swap(*iter, *(iter + 1));
    ++iter;
  }
  pop_back();
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(T&& elem) {
  if (cap_begin_ == 0 && begin_.curr_elem_ == begin_.curr_chunk_begin_) {
    resize(map_.size() * 3);
    try {
      --begin_;
      alloc_traits::construct(alloc_, begin_.curr_elem_, std::move(elem));
    } catch (...) {
      ++begin_;
      throw;
    }
    return;
  }
  if (begin_.curr_elem_ == begin_.curr_chunk_begin_) {
    --cap_begin_;
    map_[cap_begin_] = alloc_traits::allocate(alloc_, kChunkSize);
    try {
      --begin_;
      alloc_traits::construct(alloc_, begin_.curr_elem_, std::move(elem));
    } catch (...) {
      ++begin_;
      throw;
    }
    return;
  }
  try {
    --begin_;
    alloc_traits::construct(alloc_, begin_.curr_elem_, std::move(elem));
  } catch (...) {
    ++begin_;
    throw;
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(const T& elem) {
  if (cap_begin_ == 0 && begin_.curr_elem_ == begin_.curr_chunk_begin_) {
    resize(map_.size() * 3);
    try {
      --begin_;
      alloc_traits::construct(alloc_, begin_.curr_elem_, elem);
    } catch (...) {
      ++begin_;
      throw;
    }
    return;
  }
  if (begin_.curr_elem_ == begin_.curr_chunk_begin_) {
    --cap_begin_;
    map_[cap_begin_] = alloc_traits::allocate(alloc_, kChunkSize);
    try {
      --begin_;
      alloc_traits::construct(alloc_, begin_.curr_elem_, elem);
    } catch (...) {
      ++begin_;
      throw;
    }
    return;
  }
  try {
    --begin_;
    alloc_traits::construct(alloc_, begin_.curr_elem_, elem);
  } catch (...) {
    ++begin_;
    throw;
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(T&& elem) {
  if (cap_end_ == map_.size() - 1 &&
      end_.curr_elem_ + 1 == end_.curr_chunk_end_) {
    resize(map_.size() * 3);
    map_[++cap_end_] = alloc_traits::allocate(alloc_, kChunkSize);
    try {
      alloc_traits::construct(alloc_, (end_).curr_elem_, std::move(elem));
      ++end_;
    } catch (...) {
      throw;
    }
    end_.set_chunk(&map_[cap_end_]);
    end_.curr_elem_ = end_.curr_chunk_begin_;
    return;
  }
  if (end_.curr_elem_ + 1 == end_.curr_chunk_end_) {
    if (cap_end_ != map_.size()) {
      map_[++cap_end_] = alloc_traits::allocate(alloc_, kChunkSize);
    }
    try {
      alloc_traits::construct(alloc_, (end_).curr_elem_, std::move(elem));
      ++end_;
    } catch (...) {
      throw;
    }
    return;
  }
  try {
    alloc_traits::construct(alloc_, end_.curr_elem_, std::move(elem));
    ++end_;
  } catch (...) {
    throw;
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(const T& elem) {
  if (cap_end_ == map_.size() - 1 &&
      end_.curr_elem_ + 1 == end_.curr_chunk_end_) {
    resize(map_.size() * 3);
    map_[++cap_end_] = alloc_traits::allocate(alloc_, kChunkSize);
    try {
      alloc_traits::construct(alloc_, end_.curr_elem_, elem);
      ++end_;
    } catch (...) {
      throw;
    }
    end_.set_chunk(&map_[cap_end_]);
    end_.curr_elem_ = end_.curr_chunk_begin_;
    return;
  }
  if (end_.curr_elem_ + 1 == end_.curr_chunk_end_) {
    if (cap_end_ != map_.size()) {
      ++cap_end_;
      map_[cap_end_] = alloc_traits::allocate(alloc_, kChunkSize);
    }
    try {
      alloc_traits::construct(alloc_, end_.curr_elem_, elem);
      ++end_;
    } catch (...) {
      throw;
    }
    return;
  }
  try {
    alloc_traits::construct(alloc_, end_.curr_elem_, elem);
    ++end_;
  } catch (...) {
    throw;
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::clear() {
  for (size_t i = 0; i < size(); ++i) {
    alloc_traits::destroy(alloc_, &begin_[i]);
  }
  for (size_t i = cap_begin_; i <= cap_end_; ++i) {
    alloc_traits::deallocate(alloc_, map_[i], kChunkSize);
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::resize(size_t count) {
  std::vector<T*> tmp(count, nullptr);
  for (size_t i = cap_begin_; i <= cap_end_; ++i) {
    tmp[i - cap_begin_ + (count / 3)] = map_[i];
    map_[i] = nullptr;
  }
  cap_begin_ = (count / 3);
  cap_end_ = cap_begin_ + (size() / kChunkSize);
  std::swap(tmp, map_);
  begin_.set_chunk(&map_[cap_begin_]);
  end_.set_chunk(&map_[cap_end_]);
}

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(Deque&& other) {
  alloc_ = alloc_traits::propagate_on_container_move_assignment::value
               ? other.alloc_
               : alloc_;
  if (size() == other.size()) {
    for (size_t i = 0; i < size(); ++i) {
      begin_[i] = std::move(other.begin_[i]);
    }
  }
  if (size() > other.size()) {
    for (size_t i = 0; i < other.size(); ++i) {
      begin_[i] = std::move(other.begin_[i]);
    }
    while (size() != other.size()) {
      pop_back();
    }
  }
  if (size() < other.size()) {
    resize(other.map_.size());
    size_t iter = 0;
    for (; iter < size(); ++iter) {
      begin_[iter] = std::move(other.begin_[iter]);
    }
    for (; iter < other.size(); ++iter) {
      push_back(std::move(other.begin_[iter]));
    }
  }
  return *this;
}

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(
    const Deque<T, Allocator>& other) {
  alloc_ = alloc_traits::propagate_on_container_copy_assignment::value
               ? other.alloc_
               : alloc_;
  if (size() == other.size()) {
    for (size_t i = 0; i < size(); ++i) {
      begin_[i] = other.begin_[i];
    }
  }
  if (size() > other.size()) {
    for (size_t i = 0; i < other.size(); ++i) {
      begin_[i] = other.begin_[i];
    }
    while (size() != other.size()) {
      pop_back();
    }
  }
  if (size() < other.size()) {
    resize(other.map_.size());
    size_t iter = 0;
    for (; iter < size(); ++iter) {
      begin_[iter] = other.begin_[iter];
    }
    for (; iter < other.size(); ++iter) {
      emplace_back(other.begin_[iter]);
    }
  }
  return *this;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(std::initializer_list<T> init,
                           const Allocator& alloc)
    : alloc_(alloc) {
  map_.resize(3, nullptr);
  map_[1] = alloc_traits::allocate(alloc_, kChunkSize);
  cap_begin_ = 1;
  cap_end_ = 1;
  begin_.set_chunk(&map_[1]);
  begin_.curr_elem_ = begin_.curr_chunk_begin_;
  end_.set_chunk(&map_[1]);
  end_.curr_elem_ = end_.curr_chunk_begin_;
  for (const auto& iter : init) {
    emplace_back(iter);
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(Deque&& other)
    : map_(std::vector<T*>(other.map_.size(), nullptr)),
      cap_begin_(other.cap_begin_),
      cap_end_(other.cap_end_),
      alloc_(
          alloc_traits::select_on_container_copy_construction(other.alloc_)) {
  for (size_t i = cap_begin_; i <= cap_end_; ++i) {
    map_[i] = other.map_[i];
    other.map_[i] = nullptr;
  }
  other.map_.resize(3, nullptr);
  begin_ = std::move(other.begin_);
  end_ = std::move(other.end_);
  other.map_[1] = other.get_allocator().allocate(kChunkSize);
  other.begin_.set_chunk(&other.map_[1]);
  other.begin_.curr_elem_ = other.begin_.curr_chunk_begin_;
  other.end_.set_chunk(&other.map_[1]);
  other.end_.curr_elem_ = other.end_.curr_chunk_begin_;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Deque<T, Allocator>& other)
    : Deque(alloc_traits::select_on_container_copy_construction(other.alloc_)) {
  for (const auto& iter : other) {
    emplace_back(iter);
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const Allocator& alloc)
    : Deque(alloc) {
  for (size_t i = 0; i < count; ++i) {
    try {
      emplace_back();
    } catch (...) {
      throw;
    }
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t elem_number, const T& default_elem,
                           const Allocator& alloc)
    : alloc_(alloc) {
  size_t chunk_number = (elem_number / kChunkSize) + 1;
  map_.resize(chunk_number * 3, nullptr);
  cap_begin_ = chunk_number;
  cap_end_ = (chunk_number * 2) - 1;
  for (size_t i = cap_begin_; i <= cap_end_; ++i) {
    map_[i] = alloc_traits::allocate(alloc_, kChunkSize);
  }
  begin_.set_chunk(&map_[cap_begin_]);
  begin_.curr_elem_ = begin_.curr_chunk_begin_;
  end_.set_chunk(&map_[cap_end_]);
  end_.curr_elem_ = end_.curr_chunk_begin_ + (elem_number % kChunkSize);
  for (auto& iter : *this) {
    try {
      alloc_traits::construct(alloc_, &iter, std::move(default_elem));
    } catch (...) {
      this->clear();
      throw;
    }
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Allocator& alloc) : alloc_(alloc) {
  map_.resize(3, nullptr);
  map_[1] = alloc_traits::allocate(alloc_, kChunkSize);
  cap_begin_ = 1;
  cap_end_ = 1;
  begin_.set_chunk(&map_[1]);
  begin_.curr_elem_ = begin_.curr_chunk_begin_;
  end_.set_chunk(&map_[1]);
  end_.curr_elem_ = end_.curr_chunk_begin_;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque() : alloc_(Allocator()) {
  map_.resize(3, nullptr);
  map_[1] = alloc_traits::allocate(alloc_, kChunkSize);
  cap_begin_ = 1;
  cap_end_ = 1;
  begin_.set_chunk(&map_[1]);
  begin_.curr_elem_ = begin_.curr_chunk_begin_;
  end_.set_chunk(&map_[1]);
  end_.curr_elem_ = end_.curr_chunk_begin_;
}

template <typename T, typename Allocator>
template <bool IsConst>
void Deque<T, Allocator>::CommonIterator<IsConst>::set_chunk(T** new_chunk) {
  curr_chunk_ = new_chunk;
  curr_chunk_begin_ = *curr_chunk_;
  curr_chunk_end_ = curr_chunk_begin_ + kChunkSize;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template CommonIterator<IsConst>&
Deque<T, Allocator>::CommonIterator<IsConst>::operator+=(int64_t idx) {
  const int64_t kOffset = idx + (curr_elem_ - curr_chunk_begin_);
  if (kOffset >= 0 && kOffset < (int64_t)kChunkSize) {
    curr_elem_ += idx;
  } else {
    const int64_t kChunkOffset =
        kOffset > 0 ? kOffset / kChunkSize : -((-kOffset - 1) / kChunkSize) - 1;
    set_chunk(curr_chunk_ + kChunkOffset);
    curr_elem_ = curr_chunk_begin_ + (kOffset - kChunkOffset * kChunkSize);
  }
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
bool Deque<T, Allocator>::CommonIterator<IsConst>::operator<(
    const Deque<T, Allocator>::CommonIterator<IsConst>& rhs) const {
  return (curr_chunk_ == rhs.curr_chunk_) ? (curr_elem_ < rhs.curr_elem_)
                                          : (curr_chunk_ < rhs.curr_chunk_);
}

template <typename T, typename Allocator>
template <bool IsConst>
bool Deque<T, Allocator>::CommonIterator<IsConst>::operator==(
    const Deque<T, Allocator>::CommonIterator<IsConst>& rhs) const {
  return curr_chunk_ == rhs.curr_chunk_ && curr_elem_ == rhs.curr_elem_ &&
         curr_chunk_end_ == rhs.curr_chunk_end_ &&
         curr_chunk_begin_ == rhs.curr_chunk_begin_;
}

template <typename T, typename Allocator>
template <bool IsConst>
std::ptrdiff_t Deque<T, Allocator>::CommonIterator<IsConst>::operator-(
    const Deque::CommonIterator<IsConst>& rhs) const {
  return kChunkSize * (curr_chunk_ - rhs.curr_chunk_ - int(curr_chunk_ != 0)) +
         (curr_elem_ - curr_chunk_begin_) +
         (rhs.curr_chunk_end_ - rhs.curr_elem_);
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template CommonIterator<IsConst>
Deque<T, Allocator>::CommonIterator<IsConst>::operator-(int64_t idx) const {
  CommonIterator<IsConst> tmp = *this;
  return tmp += -idx;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template CommonIterator<IsConst>
Deque<T, Allocator>::CommonIterator<IsConst>::operator+(int64_t idx) const {
  CommonIterator<IsConst> tmp = *this;
  return tmp += idx;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template CommonIterator<IsConst>
Deque<T, Allocator>::CommonIterator<IsConst>::operator--(int) {
  CommonIterator<IsConst> tmp = *this;
  --*this;
  return tmp;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template CommonIterator<IsConst>
Deque<T, Allocator>::CommonIterator<IsConst>::operator++(int) {
  CommonIterator<IsConst> tmp = *this;
  ++*this;
  return tmp;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template CommonIterator<IsConst>&
Deque<T, Allocator>::CommonIterator<IsConst>::operator--() {
  if (curr_elem_ == curr_chunk_begin_) {
    set_chunk(--curr_chunk_);
    curr_elem_ = curr_chunk_end_;
  }
  --curr_elem_;
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template CommonIterator<IsConst>&
Deque<T, Allocator>::CommonIterator<IsConst>::operator++() {
  ++curr_elem_;
  if (curr_elem_ == curr_chunk_end_) {
    set_chunk(++curr_chunk_);
    curr_elem_ = curr_chunk_begin_;
  }
  return *this;
}
