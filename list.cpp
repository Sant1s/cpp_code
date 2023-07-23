//
// Created by mikiele on 17.04.23.
//

#include <iostream>

template <typename T, typename Allocator = std::allocator<T>>
class List {
 protected:
  struct BaseNode {
    BaseNode* next;
    BaseNode* prev;
    BaseNode() : next(nullptr), prev(nullptr) {}
    BaseNode(BaseNode* prev_node, BaseNode* next_node)
        : next(next_node), prev(prev_node) {}
    ~BaseNode() = default;
  };
  struct Node : public BaseNode {
    T value;
    using BaseNode::next;
    using BaseNode::prev;
    Node() : BaseNode(), value() {}
    Node(const T& val) : BaseNode(), value(val) {}
    Node(T&& val) : BaseNode(), value(std::move(val)) {}
    ~Node() = default;
  };

 public:
  using value_type = T;
  using allocator_type = Allocator;

  List();
  List(size_t count, const T& value, const Allocator& alloc = Allocator());
  explicit List(size_t count, const Allocator& alloc = Allocator());
  List(const List& other);
  List(std::initializer_list<T> init, const Allocator& alloc = Allocator());
  List<T, Allocator>& operator=(const List<T, Allocator>& other);
  ~List() {
    clear();
    basenode_traits::deallocate(base_alloc_, head_, 1);
  }
  Allocator& get_allocator() { return alloc_; }

  using alloc_traits = std::allocator_traits<Allocator>;
  using node_allocator = typename alloc_traits::template rebind_alloc<Node>;
  using basenode_allocator = typename std::allocator_traits<
      Allocator>::template rebind_alloc<BaseNode>;
  using node_traits = std::allocator_traits<node_allocator>;
  using basenode_traits = std::allocator_traits<basenode_allocator>;

  template <bool IsConst>
  class CommonIterator {
   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = std::conditional_t<IsConst, const T*, T*>;
    using reference = std::conditional_t<IsConst, const T&, T&>;
    using const_reference = const T&;
    using difference_type = std::ptrdiff_t;
    using value_type = std::conditional_t<IsConst, const T, T>;

    CommonIterator() : current_(nullptr) {}
    CommonIterator(const CommonIterator* other)
        : current_(const_cast<BaseNode*>(other->current_)) {}
    CommonIterator(Node* node) : current_(node) {}
    CommonIterator& operator++();
    CommonIterator& operator--();
    CommonIterator operator++(int);
    CommonIterator operator--(int);
    std::conditional_t<IsConst, const T&, T&> operator*() {
      return static_cast<std::conditional_t<IsConst, const Node*, Node*>>(
                 current_)
          ->value;
    }
    std::conditional_t<IsConst, const T*, T*> operator->() {
      return &(*(*this));
    }
    bool operator!=(const CommonIterator<IsConst>& argument) const {
      return !(*this == argument);
    }
    bool operator==(const CommonIterator<IsConst>& argument) const {
      return current_ == argument.current_;
    }

   private:
    std::conditional_t<IsConst, const Node*, Node*> current_;
    CommonIterator<true> cast() { return {const_cast<BaseNode*>(current_)}; }
  };

  bool empty() const { return size_ == 0; }
  size_t size() const { return size_; }
  void push_back(const T& elem);
  void push_front(const T& elem);
  void push_back(T&& elem);
  void push_front(T&& elem);
  void pop_back();
  void pop_front();
  T& front() { return *(head_->prev); }
  const T& front() const { return *(head_->prev); }
  T& back() { return *(head_->next); }
  const T& back() const { return *(head_->next); }

  using iterator = CommonIterator<false>;
  using const_iterator = CommonIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  iterator begin() const { return iterator(static_cast<Node*>(head_->next)); }
  const_iterator cbegin() {
    return const_iterator(static_cast<Node*>(head_->next));
  }
  iterator end() const { return iterator(static_cast<Node*>(head_)); }
  const_iterator cend() const {
    return const_iterator(static_cast<Node*>(head_));
  }
  reverse_iterator rbegin() const { return reverse_iterator(--end()); }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(--end());
  }
  reverse_iterator rend() const { return reverse_iterator(--begin()); }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(--begin());
  }

 private:
  void clear();
  BaseNode* head_;
  size_t size_ = 0;
  Allocator alloc_;
  basenode_allocator base_alloc_;
  node_allocator node_alloc_;
};

template <typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(const List& other) {
  if (this == &other) {
    return *this;
  }
  alloc_ = alloc_traits::propagate_on_container_copy_assignment::value
               ? other.alloc_
               : alloc_;
  node_alloc_ = node_traits::propagate_on_container_copy_assignment::value
                    ? other.node_alloc_
                    : node_alloc_;
  base_alloc_ = basenode_traits::propagate_on_container_copy_assignment::value
                    ? other.base_alloc_
                    : base_alloc_;
  if (size_ == other.size_) {
    auto iter_this = begin();
    for (const auto& iter : other) {
      *iter_this = iter;
      ++iter_this;
    }
  } else if (size_ < other.size_) {
    auto iter_other = other.begin();
    for (auto& iter : *this) {
      iter = *iter_other;
      ++iter_other;
    }
    for (; iter_other != other.end(); ++iter_other) {
      push_back(*iter_other);
    }
    size_ = other.size_;
  } else {
    auto iter_this = begin();
    for (const auto& iter : other) {
      *iter_this = iter;
      ++iter_this;
    }
    size_t tmp_size = size_;
    for (size_t i = tmp_size; i > other.size_; --i) {
      pop_back();
    }
  }
  return *this;
}

template <typename T, typename Allocator>
List<T, Allocator>::List(std::initializer_list<T> init, const Allocator& alloc)
    : size_(0), alloc_(alloc), node_alloc_(alloc), base_alloc_(alloc) {
  head_ = basenode_traits::allocate(base_alloc_, 1);
  head_->next = head_;
  head_->prev = head_;
  try {
    for (const auto& iter : init) {
      try {
        push_back(iter);
      } catch (...) {
        clear();
        throw;
      }
    }
  } catch (...) {
    basenode_traits::destroy(base_alloc_, head_);
    basenode_traits::deallocate(base_alloc_, head_, 1);
    throw;
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(const List& other)
    : size_(0),
      alloc_(alloc_traits::select_on_container_copy_construction(other.alloc_)),
      node_alloc_(node_traits::select_on_container_copy_construction(
          other.node_alloc_)),
      base_alloc_(basenode_traits::select_on_container_copy_construction(
          other.base_alloc_)) {
  head_ = basenode_traits::allocate(base_alloc_, 1);
  head_->next = head_;
  head_->prev = head_;
  try {
    for (const auto& iter : other) {
      try {
        push_back(iter);
      } catch (...) {
        clear();
        throw;
      }
    }
  } catch (...) {
    basenode_traits::destroy(base_alloc_, head_);
    basenode_traits::deallocate(base_alloc_, head_, 1);
    throw;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
  Node* tmp = static_cast<Node*>(head_->next);
  head_->next = tmp->next;
  tmp->next->prev = head_;
  node_traits::destroy(node_alloc_, tmp);
  node_traits::deallocate(node_alloc_, tmp, 1);
  --size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
  BaseNode* tmp = head_->prev;
  head_->prev = tmp->prev;
  tmp->prev->next = head_;
  node_traits::destroy(node_alloc_, static_cast<Node*>(tmp));
  node_traits::deallocate(node_alloc_, static_cast<Node*>(tmp), 1);
  --size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_front(T&& elem) {
  Node* tmp = node_traits::allocate(node_alloc_, 1);
  try {
    node_traits::construct(node_alloc_, tmp, std::move(elem));
    BaseNode* temp = head_->next;
    temp->prev = tmp;
    tmp->next = temp;
    tmp->prev = head_;
    head_->next = tmp;
    ++size_;
  } catch (...) {
    node_traits::deallocate(node_alloc_, tmp, 1);
    throw;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_front(const T& elem) {
  Node* tmp = node_traits::allocate(node_alloc_, 1);
  try {
    node_traits::construct(node_alloc_, tmp, elem);
    BaseNode* temp = head_->next;
    temp->prev = tmp;
    tmp->next = temp;
    tmp->prev = head_;
    head_->next = tmp;
    ++size_;
  } catch (...) {
    node_traits::deallocate(node_alloc_, tmp, 1);
    throw;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::clear() {
  size_t tmp = size_;
  for (size_t i = 0; i < tmp; ++i) {
    pop_back();
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back(T&& elem) {
  Node* tmp = node_traits::allocate(node_alloc_, 1);
  try {
    node_traits::construct(node_alloc_, tmp, std::move(elem));
    BaseNode* temp = head_->prev;
    temp->next = tmp;
    tmp->prev = temp;
    tmp->next = head_;
    head_->prev = tmp;
    ++size_;
  } catch (...) {
    node_traits::deallocate(node_alloc_, tmp, 1);
    throw;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back(const T& elem) {
  Node* tmp = node_traits::allocate(node_alloc_, 1);
  try {
    node_traits::construct(node_alloc_, tmp, elem);
    BaseNode* temp = head_->prev;
    temp->next = tmp;
    tmp->prev = temp;
    tmp->next = head_;
    head_->prev = tmp;
    ++size_;
  } catch (...) {
    node_traits::deallocate(node_alloc_, tmp, 1);
    throw;
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const Allocator& alloc)
    : size_(0), alloc_(alloc), node_alloc_(alloc), base_alloc_(alloc) {
  head_ = basenode_traits::allocate(base_alloc_, 1);
  head_->next = head_;
  head_->prev = head_;
  try {
    for (size_t i = 0; i < count; ++i) {
      Node* tmp = node_traits::allocate(node_alloc_, 1);
      try {
        node_traits::construct(node_alloc_, tmp);
        BaseNode* temp = head_->prev;
        temp->next = tmp;
        tmp->prev = temp;
        tmp->next = head_;
        head_->prev = tmp;
        ++size_;
      } catch (...) {
        node_traits::deallocate(node_alloc_, tmp, 1);
        throw;
      }
    }
  } catch (...) {
    clear();
    basenode_traits::destroy(base_alloc_, head_);
    basenode_traits::deallocate(base_alloc_, head_, 1);
    throw;
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const T& value, const Allocator& alloc)
    : size_(0), alloc_(alloc), node_alloc_(alloc), base_alloc_(alloc) {
  head_ = basenode_traits::allocate(base_alloc_, 1);
  head_->next = head_;
  head_->prev = head_;
  try {
    for (size_t i = 0; i < count; ++i) {
      try {
        push_back(value);
      } catch (...) {
        clear();
        throw;
      }
    }
  } catch (...) {
    clear();
    basenode_traits::destroy(base_alloc_, head_);
    basenode_traits::deallocate(base_alloc_, head_, 1);
    throw;
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List()
    : size_(0),
      alloc_(Allocator()),
      node_alloc_(Allocator()),
      base_alloc_(Allocator()) {
  head_ = basenode_traits::allocate(base_alloc_, 1);
  head_->next = head_;
  head_->prev = head_;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename List<T, Allocator>::template CommonIterator<IsConst>
List<T, Allocator>::CommonIterator<IsConst>::operator--(int) {
  CommonIterator tmp = *this;
  current_ = reinterpret_cast<Node*>(current_->prev);
  return tmp;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename List<T, Allocator>::template CommonIterator<IsConst>
List<T, Allocator>::CommonIterator<IsConst>::operator++(int) {
  CommonIterator tmp = *this;
  current_ = reinterpret_cast<Node*>(current_->next);
  return tmp;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename List<T, Allocator>::template CommonIterator<IsConst>&
List<T, Allocator>::CommonIterator<IsConst>::operator--() {
  current_ = reinterpret_cast<Node*>(current_->prev);
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename List<T, Allocator>::template CommonIterator<IsConst>&
List<T, Allocator>::CommonIterator<IsConst>::operator++() {
  current_ = reinterpret_cast<Node*>(current_->next);
  return *this;
}
