#pragma once

#include <cassert>
#include <cstdint>
#include <vector>
#include <cmath>
#include <ostream>
#include <istream>

#include "serialization.hpp"

template<typename Type>
class Columns{
	Type *m_DataPtr;
	size_t m_M;
public:
	Columns(Type *data_ptr, size_t m):
		m_DataPtr(data_ptr),
		m_M(m)
	{}

	Type &operator[](size_t m){
		assert(m < m_M);
		return m_DataPtr[m];
	}

	size_t M()const {
		return m_M;
	}
};

template<typename Type>
class Matrix{
	Type *m_Data = nullptr;
	size_t m_N = 0;
	size_t m_M = 0;
public:
	Matrix(size_t n, size_t m): 
		m_N(n), 
		m_M(m) 
	{ 
		if(!n || !m)
			return;

		m_Data = new Type[m_N * m_M];

		std::memset(m_Data, 0, Count() * sizeof(Type));
	}

	Matrix(Matrix&& other) {
		*this = std::move(other);
	}

	~Matrix(){
		Clear();
	}
	Matrix &operator=(Matrix&& other) {
		Clear();
		std::swap(m_Data, other.m_Data);
		std::swap(m_N, other.m_N);
		std::swap(m_M, other.m_M);

		return *this;
	}

	    // Copy constructor
    Matrix(const Matrix& other) : m_N(other.m_N), m_M(other.m_M) {
        if (!m_N || !m_M)
            return;

        m_Data = new Type[m_N * m_M];
        std::copy(other.m_Data, other.m_Data + m_N * m_M, m_Data);
    }

    // Copy assignment operator
    Matrix& operator=(const Matrix& other) {
        if (this != &other) {
            Clear();

            m_N = other.m_N;
            m_M = other.m_M;

            if (m_N && m_M) {
                m_Data = new Type[m_N * m_M];
                std::copy(other.m_Data, other.m_Data + m_N * m_M, m_Data);
            }
        }

        return *this;
    }

	const Type &Get(size_t n, size_t m)const {
		assert(n < N() && m < M());
		return m_Data[m_M * n + m];
	}

	Type &Get(size_t n, size_t m){
		assert(n < N() && m < M());
		return m_Data[m_M * n + m];
	}

	Columns<const Type> operator[](size_t n)const{
		assert(n < m_N);
		return {&m_Data[n * m_M], m_M};
	}

	Columns<Type> operator[](size_t n){
		assert(n < m_N);
		return {&m_Data[n * m_M], m_M};
	}

	size_t N()const{
		return m_N;
	}

	size_t M()const{
		return m_M;
	}

	size_t Count()const {
		return M() * N();
	}

	void Clear() {
		delete[] m_Data;
		m_N = 0;
		m_M = 0;
		m_Data = nullptr;
	}

	Type* Data() {
		return m_Data;
	}

	const Type* Data()const{
		return m_Data;
	}
	
	template<typename Predicate>
	void ForEach(Predicate pred) {
		for (int i = 0; i < Count(); i++) {
			pred(Data()[i]);
		}
	}

	template<typename Predicate>
	void ForEachIndexed(Predicate pred) {
		for (int i = 0; i < N(); i++) {
			for(int j = 0; j<M(); j++){
				pred(Get(i, j), i, j);
			}
		}
	}
		
	static Matrix<Type> Random(size_t n, size_t m, Type min, Type max) {
		Matrix<Type> result(n, m);

		result.ForEach([&](Type& e) {
			e = GetRandom<Type>(min, max);
		});

		return result;
	}
};

template<typename T>
Matrix<T> operator*(const Matrix<T> &a, const Matrix<T> &b) {
    assert(a.M() == b.N());

    // Create a new matrix to store the result
    Matrix<T> result(a.N(), b.M());

    // Perform matrix multiplication
    for (size_t i = 0; i < a.N(); ++i) {
        for (size_t j = 0; j < b.M(); ++j) {
            T sum = 0;
            for (size_t k = 0; k < a.M(); ++k) {
                sum += a[i][k] * b[k][j];
            }
            result[i][j] = sum;
        }
    }

    return result;
}

template<typename T>
Matrix<T> operator-(const Matrix<T>& a, const Matrix<T>& b) {
    assert (!(a.N() != b.N() || a.M() != b.M()));

    // Create a new matrix to store the result
    Matrix<T> result(a.N(), a.M());

    // Perform matrix subtraction
    for (size_t i = 0; i < a.N(); ++i) {
        for (size_t j = 0; j < a.M(); ++j) {
            result[i][j] = a[i][j] - b[i][j];
        }
    }

    return result;
}


template<typename Type>
bool operator==(const Matrix<Type>& lhs, const Matrix<Type>& rhs) {
    // Check if dimensions are the same
    if (lhs.N() != rhs.N() || lhs.M() != rhs.M()) {
        return false;
    }

    // Compare each element
    for (size_t i = 0; i < lhs.N(); ++i) {
        for (size_t j = 0; j < lhs.M(); ++j) {
            if (lhs[i][j] != rhs[i][j]) {
                return false;
            }
        }
    }

    return true;
}

template<typename T>
struct Serializer<Matrix<T>>{

	static void ToStream(const Matrix<T> &matrix, std::ostream& stream){
		Serializer<std::size_t>::ToStream(matrix.N(), stream);
		Serializer<std::size_t>::ToStream(matrix.M(), stream);

		for (auto i = 0; i < matrix.Count(); i++)
			Serializer<T>::ToStream(matrix.Data()[i], stream);
	}

	static std::optional<Matrix<T>> FromStream(std::istream& stream) {

		auto n = Serializer<std::size_t>::FromStream(stream);
		auto m = Serializer<std::size_t>::FromStream(stream);

		if(!n.has_value() || !m.has_value())
			return std::nullopt;

		Matrix<T> matrix(n.value(), m.value());
		
		for (auto i = 0; i < matrix.Count(); i++){
			auto e = Serializer<T>::FromStream(stream);

			if(!e.has_value())
				return std::nullopt;

			matrix.Data()[i] = std::move(e.value());
		}
		
		return {std::move(matrix)};
	}

};
