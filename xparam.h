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

#include <string>
#include <X11/Xresource.h>

bool getValue(XrmDatabase db, const char *name, std::string &value, const char *def = nullptr);
bool getValue(XrmDatabase db, const char *name, int &value, int def);
bool getValue(XrmDatabase db, const char *name, bool &value, bool def);
bool getValue(XrmDatabase db, const char *name, XRectangle &value);
bool getValue(XrmDatabase db, const char *name, XColor &value);
bool getValue(XrmDatabase db, const char *name, XID &value);

inline bool getValue(XrmDatabase db, const char *name, int &value) {
	return getValue(db, name, value, value);
}

inline bool getValue(XrmDatabase db, const char *name, bool &value) {
	return getValue(db, name, value, value);
}
