#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

const uint64_t kBase = 10;
const int64_t kNumm = INT64_MIN;

class BigInt {
 private:
  std::vector<int64_t> value_;
  bool negative_ = false;
  void Del();

 public:
  // constructors and destructor
  BigInt();
  BigInt(int64_t number);
  BigInt(std::string number);
  BigInt(const BigInt& number);
  ~BigInt();

  // equality operator
  BigInt& operator=(const BigInt& number);

  // arithmetic operator's overloading
  BigInt operator+(const BigInt& number) const;
  BigInt operator-(const BigInt& number) const;
  BigInt operator*(const BigInt& number) const;
  BigInt operator/(const BigInt& number) const;
  BigInt operator%(const BigInt& number) const;

  // assignment version of arithmetic operator's overloading
  BigInt& operator+=(const BigInt& number);
  BigInt& operator-=(const BigInt& number);
  BigInt& operator*=(const BigInt& number);
  BigInt& operator/=(const BigInt& number);
  BigInt& operator%=(const BigInt& number);

  // prefix increment & decrement operator's overloading
  BigInt& operator++();
  BigInt& operator--();

  // postfix increment & decrement operator's overloading
  BigInt operator++(int);
  BigInt operator--(int);

  // unary minus
  BigInt operator-() const;

  // logical operator's overloading
  bool operator==(const BigInt& number) const;
  bool operator!=(const BigInt& number) const;
  bool operator<=(const BigInt& number) const;
  bool operator>=(const BigInt& number) const;
  bool operator<(const BigInt& number) const;
  bool operator>(const BigInt& number) const;

  // input & output operator's overloading
  friend std::istream& operator>>(std::istream& in, BigInt& number);
  friend std::ostream& operator<<(std::ostream& os, const BigInt& number);
};

// constructors & destructor
BigInt::BigInt() = default;

BigInt::BigInt(int64_t number) {
  if (number == 0 || number == -0) {
    value_.push_back(0);
    negative_ = false;
    return;
  }
  negative_ = (number < 0);
  number = (negative_) ? -number : number;
  while (number != 0) {
    value_.push_back(number % kBase);
    number /= kBase;
  }
}

BigInt::BigInt(std::string number) {
  if (number == "0" || number == "-0") {
    value_.push_back(0);
    negative_ = false;
    return;
  }
  if (number[0] == '-') {
    number = number.substr(1);
    negative_ = true;
  } else {
    negative_ = false;
  }
  for (auto i : number) {
    value_.push_back(i - '0');
  }
  std::reverse(value_.begin(), value_.end());
}

BigInt::BigInt(const BigInt& number) {
  this->negative_ = number.negative_;
  this->value_ = number.value_;
}

BigInt::~BigInt() = default;

// equality operator
BigInt& BigInt::operator=(const BigInt& number) {
  this->value_.clear();
  this->negative_ = number.negative_;
  this->value_ = number.value_;
  return *this;
}

// arithmetic operator's overloading
BigInt BigInt::operator+(const BigInt& number) const {
  if (negative_) {
    return (number.negative_) ? -(-*this + (-number)) : number - (-*this);
  }
  if (number.negative_) {
    return *this - (-number);
  }
  BigInt result;
  uint64_t current = 0;
  uint64_t current_sum = 0;
  for (uint64_t i = 0; i < std::min(value_.size(), number.value_.size()); ++i) {
    current_sum = current + number.value_[i] + value_[i];
    result.value_.push_back(current_sum % kBase);
    current = current_sum / kBase;
  }
  for (uint64_t i = std::min(value_.size(), number.value_.size());
       i < std::max(value_.size(), number.value_.size()); ++i) {
    current_sum = (number.value_.size() > value_.size())
                      ? current + number.value_[i]
                      : current + value_[i];
    result.value_.push_back(current_sum % kBase);
    current = current_sum / kBase;
  }
  if (current != 0) {
    result.value_.push_back(current);
  }
  return result;
}

BigInt BigInt::operator-(const BigInt& number) const {
  if (negative_) {
    return -(-*this + number);
  }
  if (number.negative_) {
    return *this + (-number);
  }
  if (*this < number) {
    return -(number - *this);
  }
  BigInt result;
  int64_t current = 0;
  for (uint64_t i = 0; i < number.value_.size(); ++i) {
    int64_t current_diff = value_[i] - number.value_[i] - current;
    if (current_diff < 0) {
      current = 1;
      current_diff += kBase;
    } else {
      current = 0;
    }
    result.value_.push_back(current_diff);
  }
  for (uint64_t i = number.value_.size(); i < value_.size(); ++i) {
    result.value_.push_back((value_[i] - current < 0)
                                ? value_[i] + kBase - current
                                : value_[i] - current);
    current = (value_[i] - current < 0) ? 1 : 0;
  }
  result.Del();
  return result;
}

BigInt BigInt::operator*(const BigInt& number) const {
  if (number == 0 || *this == 0) {
    return 0;
  }
  std::vector<int64_t> result(value_.size() * number.value_.size(), 0);
  int64_t current = 0;
  int64_t current_mul = 0;
  uint64_t curr = 0;
  for (uint64_t i = 0; i < value_.size(); ++i) {
    current = 0;
    for (uint64_t j = 0; j < number.value_.size(); ++j) {
      current_mul = result[i + j] + value_[i] * number.value_[j] + current;
      current = current_mul / kBase;
      result[i + j] = current_mul % kBase;
    }
    curr = number.value_.size();
    while (current != 0) {
      result[i + curr] = current % kBase;
      ++curr;
      current /= kBase;
    }
  }
  while (result.size() > 1 && result.back() == 0) {
    result.pop_back();
  }
  BigInt ret_val;
  ret_val.value_ = result;
  ret_val.negative_ = negative_ xor number.negative_;
  return ret_val;
}

BigInt BigInt::operator/(const BigInt& number) const {
  BigInt copy1(*this);
  BigInt copy2(number);
  copy1.negative_ = copy2.negative_ = false;
  if (copy1 < copy2) {
    return BigInt(0);
  }
  if (*this == kNumm) {
    return BigInt(-1);
  }
  BigInt result;
  result.negative_ = negative_ xor number.negative_;
  result.value_.resize(value_.size() - number.value_.size() + 1);
  for (uint64_t i = 0; i < result.value_.size(); ++i) {
    result.value_[i] = 0;
  }
  for (int64_t i = result.value_.size() - 1; i >= 0; --i) {
    while (result * number <= *this) {
      ++result.value_[i];
    }
    --result.value_[i];
  }
  result.Del();
  return result;
}

BigInt BigInt::operator%(const BigInt& number) const {
  BigInt result = *this - ((*this / number) * number);
  return result;
}

// assignment version of arithmetic operator's overloading
BigInt& BigInt::operator+=(const BigInt& number) {
  return *this = *this + number;
}

BigInt& BigInt::operator-=(const BigInt& number) {
  return *this = *this - number;
}

BigInt& BigInt::operator*=(const BigInt& number) {
  return *this = *this * number;
}

BigInt& BigInt::operator/=(const BigInt& number) {
  return *this = *this / number;
}

BigInt& BigInt::operator%=(const BigInt& number) {
  return *this = *this % number;
}

// prefix increment & decrement operator's overloading
BigInt& BigInt::operator++() { return *this += 1; }

BigInt& BigInt::operator--() { return *this -= 1; }

// postfix increment & decrement operator's overloading
BigInt BigInt::operator++(int) {
  BigInt copy(*this);
  *this += 1;
  return copy;
}

BigInt BigInt::operator--(int) {
  BigInt copy(*this);
  *this -= 1;
  return copy;
}

// unary minus
BigInt BigInt::operator-() const {
  if (*this == BigInt(0)) {
    return *this;
  }
  BigInt result(*this);
  result.negative_ = !result.negative_;
  return result;
}

// logical operator's overloading
bool BigInt::operator==(const BigInt& number) const {
  return (number.negative_ == negative_) && (number.value_ == value_);
}

bool BigInt::operator!=(const BigInt& number) const {
  return !(*this == number);
}

bool BigInt::operator<=(const BigInt& number) const {
  if (negative_ != number.negative_) {
    return negative_;
  }
  if (!negative_) {
    if (value_.size() != number.value_.size()) {
      return value_.size() < number.value_.size();
    }
    for (int64_t i = value_.size() - 1; i >= 0; --i) {
      if (value_[i] > number.value_[i]) {
        return false;
      }
      if (value_[i] < number.value_[i]) {
        return true;
      }
    }
    return true;
  }
  if (value_.size() != number.value_.size()) {
    return !(value_.size() < number.value_.size());
  }
  for (int64_t i = value_.size() - 1; i >= 0; --i) {
    if (value_[i] < number.value_[i]) {
      return false;
    }
    if (value_[i] > number.value_[i]) {
      return true;
    }
  }
  return true;
}

bool BigInt::operator>=(const BigInt& number) const {
  return !(*this < number);
}

bool BigInt::operator<(const BigInt& number) const {
  return (*this <= number) && (*this != number);
}

bool BigInt::operator>(const BigInt& number) const {
  return !(*this <= number);
}

//  input & output operator's overloading
std::istream& operator>>(std::istream& in, BigInt& number) {
  char symbol;
  number.value_.clear();
  while (in.get(symbol) && (std::isspace(symbol) == 0)) {
    if (symbol == '-') {
      number.negative_ = true;
      continue;
    }
    number.value_.push_back(symbol - '0');
  }
  return in;
}

std::ostream& operator<<(std::ostream& os, const BigInt& number) {
  if (number.negative_) {
    os << '-';
  }
  for (uint64_t i = 0; i < number.value_.size(); ++i) {
    os << number.value_[i];
  }
  return os;
}

void BigInt::Del() {
  while (value_.size() > 1 && value_.back() == 0) {
    value_.pop_back();
  }
}
