#pragma once
#include <string.h>

#include <iostream>
#include <vector>

size_t GetRealCap(size_t size);  // для подбора вместимости
class String {
 public:
  // конструкторы
  String();
  String(size_t size, char character);
  String(const char* str);

  // правило трёх
  String(const String& str);
  String& operator=(const String& str);
  ~String();

  // дефолтные методы
  void Clear();
  void PushBack(char character);
  void PopBack();

  // методы изменения размера
  void Resize(size_t new_size);
  void Resize(size_t new_size, char character);
  void Reserve(size_t new_cap);
  void ShrinkToFit();

  void Swap(String& other);  // свапнуть с другой строкой

  // методы для доступа данных о строке
  bool Empty() const;
  size_t Size() const;
  size_t Capacity() const;
  char* Data();
  const char* Data() const;

  // операторы сложения и умножения
  String& operator+=(const String& str);
  String operator+(const String& other) const;
  String operator*(size_t k_n) const;
  String& operator*=(size_t k_n);

  // булевые операторы
  /* некостантные */
  bool operator<(const String& str_2);
  bool operator<=(const String& str_2);
  bool operator>(const String& str_2);
  bool operator>=(const String& str_2);
  bool operator==(const String& str_2);
  bool operator!=(const String& str_2);

  /* константные */
  bool operator<(const String& str_2) const;
  bool operator<=(const String& str_2) const;
  bool operator>(const String& str_2) const;
  bool operator>=(const String& str_2) const;
  bool operator==(const String& str_2) const;
  bool operator!=(const String& str_2) const;

  // операторы и методы доступа
  /* неконстантные */
  char& operator[](size_t k_index);
  char& Front();
  char& Back();
  const char& Front() const;
  const char& Back() const;
  /* константные */
  char operator[](size_t k_index) const;

  std::vector<String> Split(const String& delim = " ");
  String Join(const std::vector<String>& strings) const;

 private:
  char* string_;
  size_t size_;
  size_t capacity_;
  void Refill(size_t n_size) {
    String tmp = *this;
    delete[] string_;
    size_ = n_size;
    capacity_ = GetRealCap(size_);
    string_ = new char[capacity_ + 1];
    memcpy(string_, tmp.string_, tmp.size_);
  }
};

int Strcmp(const String& str_1, const String& str_2);

// операторы ввода и вывода
std::ostream& operator<<(std::ostream& os_out, const String& str);
std::istream& operator>>(std::istream& input, String& str);

#include "string.hpp"

// конструкторы
String::String() : string_(new char[1]) {
  string_[0] = '\0';
  size_ = 0;
  capacity_ = 0;
}

String::String(size_t size, char character) {
  size_ = size;
  capacity_ = GetRealCap(size_);
  string_ = new char[capacity_ + 1];
  memset(string_, character, size_);
  string_[size_] = '\0';
}

String::String(const char* str) {
  size_ = strlen(str);
  capacity_ = GetRealCap(size_);
  string_ = new char[capacity_ + 1];
  for (size_t i = 0; i < size_; ++i) {
    string_[i] = str[i];
  }
  string_[size_] = '\0';
}

// правило трёх
String::String(const String& str) : String::String(str.size_, '\0') {
  memcpy(string_, str.string_, size_);
}

String::~String() { delete[] string_; }

String& String::operator=(const String& str) {
  if (this == &str) {
    return *this;
  }
  delete[] string_;
  string_ = new char[str.capacity_ + 1];
  size_ = str.size_;
  capacity_ = str.capacity_;
  memcpy(string_, str.string_, size_);
  string_[size_] = '\0';
  return *this;
}

// дефолтные методы
void String::Clear() {
  delete[] string_;
  size_ = 0;
  capacity_ = 0;
  string_ = new char[capacity_ + 1];
  string_[size_] = '\0';
}

void String::PushBack(char character) {
  String tmp = *this;
  delete[] string_;
  ++size_;
  capacity_ = GetRealCap(size_);
  string_ = new char[capacity_ + 1];
  for (size_t i = 0; i < tmp.size_; ++i) {
    string_[i] = tmp.string_[i];
  }
  string_[size_ - 1] = character;
  string_[size_] = '\0';
}

void String::PopBack() {
  if (size_ == 0) {
    return;
  }
  String tmp = *this;
  delete[] string_;
  --size_;
  capacity_ = GetRealCap(size_);
  string_ = new char[capacity_ + 1];
  for (size_t i = 0; i < size_; ++i) {
    string_[i] = tmp.string_[i];
  }
  string_[size_] = '\0';
}

// методы изменения размера
void String::Resize(size_t new_size) {
  if (new_size > capacity_) {
    Refill(new_size);
    string_[size_] = '\0';
    size_ = new_size;
    return;
  }
  size_ = new_size;
  string_[size_] = '\0';
}

void String::Resize(size_t new_size, char character) {
  if (new_size > capacity_) {
    String tmp = *this;
    Refill(new_size);
    for (size_t i = tmp.size_; i < size_; ++i) {
      string_[i] = character;
    }
    string_[size_] = '\0';
    return;
  }
  if (new_size > size_) {
    for (size_t i = size_; i < new_size; ++i) {
      string_[i] = character;
    }
    size_ = new_size;
    string_[size_] = '\0';
    return;
  }
  String tmp = *this;
  delete[] string_;
  size_ = new_size;
  capacity_ = GetRealCap(size_);
  string_ = new char[capacity_ + 1];
  memcpy(string_, tmp.string_, size_);
  string_[size_] = '\0';
}

void String::Reserve(size_t new_cap) {
  capacity_ = std::max(capacity_, new_cap);
}

void String::ShrinkToFit() {
  capacity_ = (capacity_ >= size_) ? size_ : capacity_;
}

// свап строк
void String::Swap(String& other) {
  std::swap(string_, other.string_);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}

// методы для доступа данных о строке
bool String::Empty() const { return size_ == 0; }
size_t String::Size() const { return size_; }
size_t String::Capacity() const { return capacity_; }
char* String::Data() { return string_; }
const char* String::Data() const { return string_; }

// операторы сложения и умножения
String& String::operator+=(const String& str) {
  if (size_ + str.size_ <= capacity_) {
    for (size_t i = 0; i < str.size_; ++i) {
      string_[i + size_] = str.string_[i];
    }
    size_ += str.size_;
    string_[size_] = '\0';
    return *this;
  }
  String copy = *this;
  delete[] string_;
  size_ += str.size_;
  capacity_ = GetRealCap(size_);
  string_ = new char[capacity_ + 1];
  for (size_t i = 0; i < copy.size_; ++i) {
    string_[i] = copy.string_[i];
  }
  for (size_t i = 0; i < str.size_; ++i) {
    string_[i + copy.size_] = str.string_[i];
  }
  string_[size_] = '\0';
  return *this;
}

String String::operator+(const String& other) const {
  String tmp = *this;
  tmp += other;
  return tmp;
}

String String::operator*(size_t k_n) const {
  String result;
  for (size_t i = 0; i < k_n; ++i) {
    result += *this;
  }
  return result;
}

String& String::operator*=(size_t k_n) {
  *this = *this * k_n;
  return *this;
}

// булевые операторы
/* некостантные */
bool String::operator<(const String& str_2) {
  int res = Strcmp(string_, str_2.string_);
  return res < 0;
}

bool String::operator<=(const String& str_2) {
  int res = Strcmp(string_, str_2.string_);
  return res <= 0;
}

bool String::operator>(const String& str_2) {
  int res = Strcmp(string_, str_2.string_);
  return res > 0;
}

bool String::operator>=(const String& str_2) {
  int res = Strcmp(string_, str_2.string_);
  return res >= 0;
}

bool String::operator==(const String& str_2) {
  int res = Strcmp(string_, str_2.string_);
  return res == 0;
}

bool String::operator!=(const String& str_2) {
  int res = Strcmp(string_, str_2.string_);
  return res != 0;
}

/* константные */
bool String::operator<(const String& str_2) const {
  int res = Strcmp(string_, str_2.string_);
  return res < 0;
}

bool String::operator<=(const String& str_2) const {
  int res = Strcmp(string_, str_2.string_);
  return res <= 0;
}

bool String::operator>(const String& str_2) const {
  int res = Strcmp(string_, str_2.string_);
  return res > 0;
}

bool String::operator>=(const String& str_2) const {
  int res = Strcmp(string_, str_2.string_);
  return res >= 0;
}

bool String::operator==(const String& str_2) const {
  int res = Strcmp(string_, str_2.string_);
  return res == 0;
}

bool String::operator!=(const String& str_2) const {
  int res = Strcmp(string_, str_2.string_);
  return res != 0;
}

// операторы и методы доступа
/* неконстантные */
char& String::operator[](size_t k_index) { return string_[k_index]; }

char& String::Front() { return string_[0]; }

char& String::Back() { return string_[size_ - 1]; }

/* константные */
char String::operator[](size_t k_index) const { return string_[k_index]; }

const char& String::Front() const { return string_[0]; }

const char& String::Back() const { return string_[size_ - 1]; }

// операторы ввода и вывода
std::ostream& operator<<(std::ostream& os_out, const String& str) {
  for (size_t i = 0; i < str.Size(); ++i) {
    os_out << str[i];
  }
  return os_out;
}

std::istream& operator>>(std::istream& input, String& str) {
  char symbol;
  str.Clear();
  while (input.get(symbol) && (std::isspace(symbol) == 0)) {
    str.PushBack(symbol);
  }
  return input;
}

// питоновские методы
std::vector<String> String::Split(const String& delim) {
  std::vector<String> result;
  size_t tmp = 0;
  int length;
  String tmp_str;
  char* tmp_char = string_;
  while (true) {
    if (tmp_char == nullptr) {
      break;
    }
    length = strstr(string_ + tmp, delim.string_) - tmp_char;
    if (length < 0) {
      result.push_back(tmp_char);
      return result;
    }
    if (length > 0) {
      tmp_str.Resize(length);
      memcpy(tmp_str.string_, tmp_char, length);
      tmp_str.string_[length] = '\0';
      result.push_back(tmp_str);
      tmp_char += delim.size_ + length;
      tmp += delim.size_ + length;
    }
    if (length == 0) {
      result.push_back(String(""));
      tmp_char += delim.size_ + length;
      tmp += delim.size_ + length;
    }
  }
  return result;
}

String String::Join(const std::vector<String>& strings) const {
  String result;
  for (size_t i = 0; i < strings.size(); ++i) {
    result += strings[i];
    if (i != strings.size() - 1) {
      result += *this;
    }
  }
  return result;
}

// вспомогательные методы написанные ручками

size_t GetRealCap(size_t size) {
  if (size == 0) {
    return 0;
  }
  if (size == 1) {
    return 1;
  }
  size_t degre = 2;
  while (degre < size) {
    degre *= 2;
  }
  return degre;
}

// Функции Объявленые вне класса

int Strcmp(const String& str_1, const String& str_2) {
  if (str_1.Size() != str_2.Size()) {
    return (str_1.Size() < str_2.Size()) ? -1 : 1;
  }
  size_t str_sum_1 = 0;
  size_t str_sum_2 = 0;
  for (size_t i = 0; i < str_1.Size(); ++i) {
    str_sum_1 += str_1[i] - '0';
    str_sum_2 += str_2[i] - '0';
  }
  if (str_sum_1 != str_sum_2) {
    return (str_sum_1 < str_sum_2) ? -1 : 1;
  }
  return 0;
}
