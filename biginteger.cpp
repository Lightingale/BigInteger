#include "biginteger.h"

BigInteger::BigInteger(const std::string& value) :
	sign_(true) {
	// позиция, с которой начинается число
	size_t start_pos = 0;
	if (value[0] == '-') {
		sign_ = false;
		start_pos = 1;
	}
	size_t pos = value.length() - 1;
	while (pos >= start_pos + max_len_) {
		value_.push_back(static_cast<size_t>(std::stoul(value.substr(pos - max_len_ + 1,
			max_len_))));
		pos -= max_len_;
	}
	value_.push_back(static_cast<size_t>(std::stoul(value.substr(start_pos,
		pos - start_pos + 1))));
}

BigInteger::BigInteger(int value) :
	value_(1, value >= 0 ? value : -value),
	sign_(value >= 0) {}

std::string BigInteger::toString() const {
	std::string result = "";
	if (!sign_ && (*this)) {
		result.push_back('-');
	}
	size_t i = value_.size() - 1;
	result += std::to_string(value_[i]);
	while (i-- > 0) {
		std::string digit = std::to_string(value_[i]);
		// добавляем лидирующие нули
		result += std::string(max_len_ - digit.size(), '0') + digit;
	}
	return result;
}

const BigInteger BigInteger::operator-() const {
	BigInteger result;
	result.value_ = value_;
	if (result) {
		result.sign_ = !sign_;
	}
	return result;
}

BigInteger& BigInteger::operator+=(const BigInteger& other) {
	if (sign_ == other.sign_) {
		AddAbs(other);
	}
	else {
		if (IsLessAbs(other)) {
			// если число по модулю меньше второго, вычитаем его из второго
			BigInteger other_copy = other;
			other_copy.SubstractAbs((*this));
			// если получили не 0, меняем знак
			if (other_copy) {
				other_copy.sign_ = !sign_;
			}
			(*this) = std::move(other_copy);
		}
		else {
			SubstractAbs(other);
		}
	}
	return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& other) {
	(*this) = DivideAbs(other);
	sign_ = !(sign_ ^ other.sign_);
	// true, если знаки делимого и делителя совпадают
	return *this;
}

const BigInteger BigInteger::operator++(int) {
	BigInteger copy = (*this);
	(*this) += 1;
	return copy;
}

const BigInteger BigInteger::operator--(int) {
	BigInteger copy = (*this);
	(*this) -= 1;
	return copy;
}

bool BigInteger::operator<(const BigInteger& other) const {
	if (sign_ == other.sign_) {
		if (sign_) {
			return IsLessAbs(other);
		}
		// если числа отрицательные, большее по модулю окажется меньше
		else {
			return other.IsLessAbs(*this);
		}
	}
	// если числа разного знака, первое меньше, когда второе положительно
	return other.sign_;
}

bool BigInteger::operator==(const BigInteger& other) const {
	return (value_ == other.value_ && sign_ == other.sign_);
}

bool BigInteger::operator!=(const BigInteger& other) const {
	return (value_ != other.value_ || sign_ != other.sign_);
}

const BigInteger operator+(const BigInteger& first, const BigInteger& second) {
	BigInteger first_copy = first;
	first_copy += second;
	return first_copy;
}

const BigInteger operator-(const BigInteger& first, const BigInteger& second) {
	BigInteger first_copy = first;
	first_copy -= second;
	return first_copy;
}

const BigInteger operator*(const BigInteger& first, const BigInteger& second) {
	BigInteger product(0);
	// если хотя бы один множитель нулевой, возвращаем 0
	if (!first || !second) {
		return product;
	}
	for (size_t i = 0; i < second.value_.size(); ++i) {
		product += first.MultiplyByShort(second.value_[i], i);
	}
	product.sign_ = !(first.sign_ ^ second.sign_); // true, если знаки совпадают
	return product;
}

const BigInteger operator/(const BigInteger& first, const BigInteger& second) {
	BigInteger copy = first;
	return (copy /= second);
}

const BigInteger operator%(const BigInteger& first, const BigInteger& second) {
	BigInteger copy = first;
	return (copy %= second);
}

std::istream& operator>> (std::istream& stream, BigInteger& bigint) {
	std::string value;
	stream >> value;
	bigint = BigInteger(value);
	return stream;
}

std::ostream& operator<< (std::ostream& stream, const BigInteger& bigint) {
	stream << bigint.toString();
	return stream;
}

// сложение модулей чисел
void BigInteger::AddAbs(const BigInteger& other) {
	// остаток, переносимый в следующий разряд
	size_t remain = 0;
	for (size_t i = 0; i < value_.size() || i < other.value_.size() || remain; ++i) {
		// размер суммы может быть больше размеров слагаемых
		if (value_.size() == i) {
			value_.push_back(0);
		}
		value_[i] += remain + (i < other.value_.size() ? other.value_[i] : 0);
		remain = value_[i] / base_;
		// если остаток ненулевой, нужно уменьшить текущий разряд
		value_[i] -= remain * base_;
	}
}

// вычитание модуля второго числа из модуля данного
void BigInteger::SubstractAbs(const BigInteger& other) {
	// остаток, переносимый в следующий разряд
	size_t remain = 0;
	for (size_t i = 0; i < other.value_.size() || remain; ++i) {
		size_t subtrahend = remain + (i < other.value_.size() ? other.value_[i] : 0);
		// если вычитаемое больше числа, занимаем у предыдущего разряда
		if (subtrahend > value_[i]) {
			remain = 1;
			value_[i] += base_ - subtrahend;
		}
		else {
			remain = 0;
			value_[i] -= subtrahend;
		}
	}
	RemoveZeros();
}

// умножение длинного числа на короткое и на base_ в степени power
const BigInteger BigInteger::MultiplyByShort(const size_t value, const size_t power) const {
	BigInteger product;
	// т.к. число хранится в обратном порядке, добавление нулей в начало эквивалентно 
	// умножению на степень base_
	product.value_.resize(power, 0);
	if (!value) {
		product.value_.push_back(0);
		return product;
	}
	size_t remain = 0;
	for (size_t i = 0; i < value_.size() || remain; ++i) {
		product.value_.push_back(0);
		unsigned long long result = static_cast<unsigned long long>(value) *
			(i < value_.size() ? value_[i] : 0) + remain;
		remain = static_cast<size_t>(result / base_);
		product.value_.back() = static_cast<size_t>(result - (base_ * remain));
	}
	return product;
}

// деление модуля первого числа на модуль второго, this становится остатком по модулю
const BigInteger BigInteger::DivideAbs(const BigInteger& other) {
	if (!other) {
		throw std::runtime_error("Division by zero");
	}
	if (this == &other) {
		(*this) = BigInteger(0);
		BigInteger result(1);
		return result;
	}
	if (IsLessAbs(other)) {
		return BigInteger(0);
	}
	size_t len = other.value_.size();
	size_t i = value_.size() - len;
	BigInteger result;
	result.value_ = std::vector<size_t>(i + 1, 0);
	result.sign_ = sign_;
	// деление в столбик 
	while (!IsLessAbs(other)) {
		size_t left = 0, right = base_;
		// находим делитель для каждого разряда бинарным поиском
		while (left < right) {
			size_t mid = (left + right) >> 1;
			if (IsLessAbs(other.MultiplyByShort(mid, i))) {
				right = mid;
			}
			else {
				left = mid + 1;
			}
		}
		// поиск возвращает минимальное число, строго большее искомого 
		SubstractAbs(other.MultiplyByShort(--left, i));
		result.value_[i] = left;
		--i;
	}
	result.RemoveZeros();
	return result;
}

// сравнение модулей чисел
bool BigInteger::IsLessAbs(const BigInteger& other) const {
	if (value_.size() == other.value_.size()) {
		for (size_t i = value_.size(); i-- > 0; ) {
			if (value_[i] < other.value_[i]) {
				return true;
			}
			if (value_[i] > other.value_[i]) {
				return false;
			}
		}
	}
	return (value_.size() < other.value_.size());
}

// удаление лидирующих нулей 
void BigInteger::RemoveZeros() {
	while (value_.size() > 1 && value_.back() == 0) {
		value_.pop_back();
	}
}
