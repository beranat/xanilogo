// Copyright (C) 2002 madRat
//------------------------------------------------------------------------------
// This file is part of XAniLogo application.
//------------------------------------------------------------------------------
// XAniLogo is  free software:  you can redistribute it  and/or modify it  under
// the terms of the GNU General Public License as published by the Free Software
// Foundation version 3 of the License.
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

#include <iomanip>
#include "xostream.h"

std::ostream &operator<<(std::ostream &s, const XPoint &p) {
	return s<<"("<<p.x<<", "<<p.y<<")";
}

std::ostream &operator<<(std::ostream &s, const XRectangle &p) {
	return s<<"["<<p.x<<", "<<p.y<<" - "<<p.x+p.width<<", "<<p.y+p.height<<"]";
}

std::ostream &operator<<(std::ostream &s, const XColor &c) {
	std::ios_base::fmtflags flags = s.flags();
	s.setf(std::ios_base::hex | std::ios_base::showbase | std::ios_base::right);
	const char f = s.fill();
	s.fill('0');
	s<<"rgb:/"
        <<((c.flags & DoRed)?"":"*")<<std::setw(4)<<c.red
	    <<","<<((c.flags & DoGreen)?"":"*")<<std::setw(4)<<c.green
	    <<","<<((c.flags & DoBlue)?"":"*")<<std::setw(4)<<c.blue;
	s.fill(f);
	s.flags(flags);
	return s;
}
