// Copyright (C) 2017 A. madRat L. Berenblit <beranat@users.noreply.github.com>
//------------------------------------------------------------------------------
// This file is part of XAniLogo application.
//------------------------------------------------------------------------------
// XAniLogo is  free software:  you can redistribute it  and/or modify it  under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option)
// any later version.
//------------------------------------------------------------------------------
// XAniLogo distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//------------------------------------------------------------------------------
// You should have received a copy of the GNU General Public License along with
// application. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

// vim: noexpandtab shiftwidth=4 tabstop=4 :

#pragma once
#include <cassert>
#include <cstddef>
#include <limits>

template <typename T> struct Vector4 {
	static constexpr size_t dimension = 4;
	typedef T type;
	type v[dimension];

	constexpr Vector4() noexcept : Vector4(0) {
	}

	explicit constexpr Vector4(const T &x, const T &y = 0, const T &z = 0, const T &w = 1) noexcept {
		v[0] = x;
		v[1] = y;
		v[2] = z;
		v[3] = w;
	}

	explicit constexpr Vector4(const XPoint &p) noexcept : Vector4(p.x, p.y) {
	}

	constexpr const T &operator[](size_t ind) const noexcept {
		assert(ind < dimension);
		return v[ind];
	}

	T& operator[](size_t ind) noexcept {
		assert(ind < dimension);
		return v[ind];
	}

	template<typename R> static R convert(const T v) {
		if (std::numeric_limits<R>::min() > v)
			return std::numeric_limits<R>::min();
		else if (std::numeric_limits<R>::max() < v)
			return std::numeric_limits<R>::max();
		return static_cast<R>(v);
	}

	constexpr operator XPoint() const noexcept {
		XPoint p;
		p.x = convert<decltype(p.x)>(v[0]);
		p.y = convert<decltype(p.y)>(v[1]);
		return p;
	}

	Vector4<T> operator~() const {
		const type w = v[dimension-1];
		if (fabs(w) < std::numeric_limits<T>::epsilon())
			return *this;

		Vector4<T> p;
		for (size_t i = 0; i < dimension; ++i)
			p.v[i] = v[i] / w;
		return p;
	}

	constexpr Vector4<T> operator+(const Vector4<T> &r) const noexcept {
		Vector4<T> l(*this);
		return (l+=r);
	}

	Vector4<T>& operator+=(const Vector4<T> &r) noexcept {
		for (size_t i = 0; i < dimension; ++i)
			v[i] += r.v[i];
		return *this;
	}

	constexpr T operator*(const Vector4<T> &r) const noexcept {
		T scale = 0;
		for (size_t i = 0; i < dimension; ++i)
			scale += v[i]*r.v[i];
		return scale;
	}

	constexpr T length() const noexcept {
		T l = 0;
		for (size_t i =0; i < dimension; ++i)
			l += v[i]*v[i];
		return sqrt(l);
	}

	constexpr Vector4<T> normalized() const {
		const T l = length();
		Vector4<T> t(*this);
		if (l > 2*std::numeric_limits<T>::epsilon()) {
			for (size_t i = 0; i < dimension; ++i)
				t.v[i] /= l;
		}
		return t;
	}
};

template<typename T> Vector4<T> operator*(const T &r1, const Vector4<T> &r2) {
	Vector4<T> l;
	for (size_t i =0; i < r2.dimension; ++i)
		l.v[i] = r1*r2.v[i];
	return l;
}

template <typename T> std::ostream &operator<<(std::ostream &s, const Vector4<T> &v) {
	for (size_t i =0; i < v.dimension; ++i) {
		if (0 != i)
			s<<", ";
		s<<v[i];
	}
	return s;
}

template <typename T> struct Matrix4x4 {
	static const size_t dimension = 4;
	typedef T Type;
	Type v[dimension][dimension];

	constexpr Matrix4x4() noexcept {
		for (size_t i =0; i < dimension; ++i)
			for (size_t j =0; j < dimension; ++j)
				v[i][j]=0;
	}

	explicit constexpr Matrix4x4(T r) noexcept : Matrix4x4() {
		for (size_t i =0; i < dimension; ++i)
			v[i][i] = r;
	}

	constexpr Vector4<T> operator*(const Vector4<T> &r) const {
		Vector4<T> res;
		for (size_t i = 0; i < dimension; ++i) {
			T val = 0;
			for (size_t j = 0; j < dimension; ++j)
				val += v[i][j]*r.v[j];
			res.v[i] = val;
		}
		return res;
	}

	constexpr Matrix4x4<T> operator*(const Matrix4x4<T> &r) const {
		Matrix4x4<T> m;
		for (size_t i =0; i < dimension; ++i)
			for (size_t j =0; j < dimension; ++j) {
				T val = 0;
				for (size_t k =0; k < dimension; ++k)
					val += v[i][k]*r.v[k][j];
				m.v[i][j] = val;
			}
		return m;
	}
	constexpr const T *const operator[](size_t ind) const noexcept {
		assert(ind < dimension);
		return v[ind];
	}

	T *const operator[](size_t ind) {
		assert(ind < dimension);
		return v[ind];
	}
};

template <typename T> std::ostream &operator<<(std::ostream &s, const Matrix4x4<T> &v) {
	for (size_t i =0; i < v.dimension; ++i) {
		if (0 != i)
			s<<std::endl;
		for (size_t j =0; j < v.dimension; ++j) {
			if (0 != j)
				s<<", ";
			s<<v.v[i][j];
		}
	}
	return s;
}

template <typename T> Matrix4x4<T> setScale(const Vector4<T> &v) noexcept {
	Matrix4x4<T> m(1);
	m.v[0][0] = v[0];
	m.v[1][1] = v[1];
	m.v[2][2] = v[2];
	return m;
}

template <typename T> Matrix4x4<T> setTranslation(const Vector4<T> &v) noexcept {
	Matrix4x4<T> m(1);
	m.v[0][3] = v[0];
	m.v[1][3] = v[1];
	m.v[2][3] = v[2];
	return m;
}

template <typename T> Matrix4x4<T> setRotation(const T &angle, const Vector4<T> &v) {
	Matrix4x4<T> m(1);

	const T l = v.length();
	if (l <= std::numeric_limits<T>::epsilon())
		return m;

	const T c = std::cos(angle),
			s = std::sin(angle),
			p1 = v[0] / l,
			p2 = v[1] / l,
			p3 = v[2] / l;

	m.v[0][0] = c+(1-c)*p1*p1;
	m.v[1][0] = (1-c)*p2*p1 + s*p3;
	m.v[2][0] = (1-c)*p3*p1 - s*p2;

	m.v[0][1] = (1-c)*p1*p2-s*p3;
	m.v[1][1] = c+(1-c)*p2*p2;
	m.v[2][1] = (1-c)*p3*p2 + s*p1;

	m.v[0][2] = (1-c)*p1*p3 + s*p2;
	m.v[1][2] = (1-c)*p2*p3-s*p1;
	m.v[2][2] = c+(1-c)*p3*p3;

	return m;
}

template<typename T> Matrix4x4<T> setOrthographic() noexcept {
	Matrix4x4<T> m(1);
	m[2][2] = 0;
	return m;
}

template<typename T> Matrix4x4<T> setPerspective(const T &n, const T &f) {
	Matrix4x4<T> m(0);
	m[0][0] = 1;
	m[1][1] = 1;
	m[2][2] = (n+f)/(n-f);
	m[2][3] = 2*n*f/(n-f);
	m[3][2] = -1;
	m[3][3] = 0;
	return m;
}

template<typename T> Matrix4x4<T> setViewport(const int x, const int y, const int w, const int h) noexcept {
	Matrix4x4<T> m(0);
	const T two = 2;
	m.v[0][0] = w/two;
	m.v[1][1] = h/two;
	m.v[0][3] = x+w/two;
	m.v[1][3] = y+h/two;
	return m;
}

