// Copyright (C) 2002 madRat
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

#include <X11/Xlib.h>
#include <iostream>

std::ostream &operator<<(std::ostream &s, const XPoint &p);
std::ostream &operator<<(std::ostream &s, const XRectangle &p);
std::ostream &operator<<(std::ostream &s, const XColor &c);
