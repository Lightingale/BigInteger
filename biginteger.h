#pragma once
#include <iostream>
#include <vector>
#include <string>

class BigInteger {
public:
	BigInteger() : 
		value_(1, 0),
		sign_(true) {}
	explicit BigInteger(const std::string& value);
	BigInteger(int value);

	// возвращает строковое представление числа
	std::string toString() const;

	const BigInteger operator-() const;
	BigInteger& operator+=(const BigInteger& other);
	inline BigInteger& operator-=(const BigInteger& other);
	inline BigInteger& operator*=(const BigInteger& other);
	BigInteger& operator/=(const BigInteger& other);
	inline BigInteger& operator%=(const BigInteger& other);

	inline BigInteger& operator++();
	inline BigInteger& operator--();
	const BigInteger operator++(int);
	const BigInteger operator--(int);


	bool operator<(const BigInteger& other) const;
	inline bool operator<=(const BigInteger& other) const;
	inline bool operator>(const BigInteger& other) const;
	inline bool operator>=(const BigInteger& other) const;
	bool operator==(const BigInteger& other) const;
	bool operator!=(const BigInteger& other) const;

	// false, только если значение нулевое
	inline explicit operator bool() const;

	friend const BigInteger operator+(const BigInteger& first, const BigInteger& second);
	friend const BigInteger operator-(const BigInteger& first, const BigInteger& second);
	friend const BigInteger operator*(const BigInteger& first, const BigInteger& second);
	friend const BigInteger operator/(const BigInteger& first, const BigInteger& second);
	friend const BigInteger operator%(const BigInteger& first, const BigInteger& second);

	friend std::ostream& operator<< (std::ostream& stream, const BigInteger& bigint);
	friend std::istream& operator>> (std::istream& stream, BigInteger& bigint);

private:
	std::vector<size_t> value_;
	static const size_t base_ = 1'000'000'000;
	static const size_t max_len_ = 9;
	bool sign_;

	void AddAbs(const BigInteger& other);
	void SubstractAbs(const BigInteger& other);
	const BigInteger MultiplyByShort(const size_t value, const size_t power = 0) const;
	const BigInteger DivideAbs(const BigInteger& other);
	bool IsLessAbs(const BigInteger& other) const;
	void RemoveZeros();

};

BigInteger& BigInteger::operator-=(const BigInteger& other) {
	(*this) += (-other);
	return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& other) {
	(*this) = (*this) * other;
	return (*this);
}

BigInteger& BigInteger::operator%=(const BigInteger& other) {
	DivideAbs(other);
	return *this;
}

BigInteger& BigInteger::operator++() {
	return (*this) += 1;
}

BigInteger& BigInteger::operator--() {
	return (*this) -= 1;
}

bool BigInteger::operator<=(const BigInteger& other) const {
	return !(other < (*this));
}

bool BigInteger::operator>(const BigInteger& other) const {
	return (other < (*this));
}

bool BigInteger::operator>=(const BigInteger& other) const {
	return !((*this) < other);
}

BigInteger::operator bool() const {
	return (value_.size() > 1 || value_[0] != 0);
}