// Copyright (C) 2025 madRat
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

#include <type_traits>

template <class T> constexpr bool isEqual(T a, typename std::make_unsigned<T>::type b) noexcept {
	static_assert(std::is_integral<T>::value);
	static_assert(std::is_signed<decltype(a)>::value);
	static_assert(std::is_unsigned<decltype(b)>::value);
	if (a < 0)
		return false;

	return static_cast<decltype(b)>(a) == b;
}

template <class T> constexpr bool isEqual(T a, typename std::make_signed<T>::type b) noexcept {
	static_assert(std::is_unsigned<decltype(a)>::value);
	return isEqual(b, a);
}
