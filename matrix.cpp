#pragma once

#include <algorithm>
#include <vector>

template <size_t N, size_t M, typename T = int64_t>
class Matrix {
 private:
  std::vector<std::vector<T>> matrix_;

 public:
  Matrix() { matrix_.resize(N, std::vector<T>(M)); }
  Matrix(const std::vector<std::vector<T>>& matrix) { matrix_ = matrix; }
  Matrix(const T& elem) { matrix_.resize(N, std::vector<T>(M, elem)); }
  Matrix(const Matrix<N, M, T>& mtx) { matrix_ = mtx.matrix_; }

  Matrix operator+(const Matrix<N, M, T>& mtx_1) {
    Matrix<N, M, T> result(matrix_);
    result += mtx_1;
    return result;
  }
  Matrix operator-(const Matrix<N, M, T>& mtx_1) {
    Matrix<N, M, T> result(matrix_);
    result -= mtx_1;
    return result;
  }
  Matrix operator*(const T& elem) {
    Matrix<N, M, T> result(*this);
    result *= elem;
    return result;
  }
  Matrix& operator+=(const Matrix<N, M, T>& mtx);
  Matrix& operator-=(const Matrix<N, M, T>& mtx);
  Matrix& operator*=(const T& elem);

  Matrix<M, N, T> Transposed();

  T& operator()(const size_t kIndex1, const size_t kIndex2) {
    return matrix_[kIndex1][kIndex2];
  }
  T operator()(const size_t kIndex1, const size_t kIndex2) const {
    return matrix_[kIndex1][kIndex2];
  }

  bool operator==(const Matrix<N, M, T>& mtx) { return matrix_ == mtx.matrix_; }
};

template <size_t N, typename T>
class Matrix<N, N, T> {
 private:
  std::vector<std::vector<T>> matrix_;

 public:
  Matrix() { matrix_.resize(N, std::vector<T>(N)); }
  Matrix(const std::vector<std::vector<T>>& matrix) { matrix_ = matrix; }
  Matrix(const T& elem) { matrix_.resize(N, std::vector<T>(N, elem)); }
  Matrix(const Matrix<N, N, T>& mtx) { matrix_ = mtx.matrix_; }

  Matrix operator+(const Matrix<N, N, T>& mtx_1) {
    Matrix<N, N, T> result(matrix_);
    result += mtx_1;
    return result;
  }
  Matrix operator-(const Matrix<N, N, T>& mtx_1) {
    Matrix<N, N, T> result(matrix_);
    result -= mtx_1;
    return result;
  }
  Matrix operator*(const T& elem) {
    Matrix<N, N, T> result(*this);
    result *= elem;
    return result;
  }
  Matrix& operator+=(const Matrix<N, N, T>& mtx);
  Matrix& operator-=(const Matrix<N, N, T>& mtx);
  Matrix& operator*=(const T& elem);

  Matrix<N, N, T> Transposed();
  T Trace() const;

  T& operator()(const size_t kIndex1, const size_t kIndex2) {
    return matrix_[kIndex1][kIndex2];
  }
  T operator()(const size_t kIndex1, const size_t kIndex2) const {
    return matrix_[kIndex1][kIndex2];
  }

  bool operator==(const Matrix<N, N, T>& mtx) { return matrix_ == mtx.matrix_; }
};

template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator+=(const Matrix<N, M, T>& mtx) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      matrix_[i][j] += mtx.matrix_[i][j];
    }
  }
  return *this;
}

template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator-=(const Matrix<N, M, T>& mtx) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      matrix_[i][j] -= mtx.matrix_[i][j];
    }
  }
  return *this;
}

template <size_t N, size_t M, typename T>
Matrix<M, N, T> Matrix<N, M, T>::Transposed() {
  Matrix<M, N, T> result;
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      result(j, i) = matrix_[i][j];
    }
  }
  return result;
}

template <size_t N, typename T>
Matrix<N, N, T> Matrix<N, N, T>::Transposed() {
  Matrix<N, N, T> result;
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      result(j, i) = matrix_[i][j];
    }
  }
  return result;
}

template <size_t N, typename T>
T Matrix<N, N, T>::Trace() const {
  T ans = T(0);
  for (size_t i = 0; i < N; ++i) {
    ans += matrix_[i][i];
  }
  return ans;
}

template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator*=(const T& elem) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      matrix_[i][j] *= elem;
    }
  }
  return *this;
}

template <size_t N, size_t M, size_t U, typename T>
Matrix<N, U, T> operator*(const Matrix<N, M, T>& matrix_1,
                          const Matrix<M, U, T>& matrix_2) {
  Matrix<N, U, T> result;
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < U; ++j) {
      for (size_t k = 0; k < M; ++k) {
        result(i, j) += matrix_1(i, k) * matrix_2(k, j);
      }
    }
  }
  return result;
}
