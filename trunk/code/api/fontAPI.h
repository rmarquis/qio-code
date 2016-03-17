/*
============================================================================
Copyright (C) 2016 V.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

Qio source code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA,
or simply visit <http://www.gnu.org/licenses/>.
============================================================================
*/
// fontAPI.h
#ifndef __API_FONTAPI_H__
#define __API_FONTAPI_H__

class fontAPI_i {
public:
	virtual const char *getName() const = 0;
	virtual void drawString(float x, float y, const char *s, float scaleX = 1.f, float scaleY = 1.f, bool bUseColourCodes = true) const = 0;
	virtual float drawChar(float x, float y, char s, float sx = 1.f, float sy = 1.f) const = 0;
	virtual float getStringWidth(const char *s) const = 0;
	virtual float getStringHeight(const char *s) const = 0;
};

#endif // __API_FONTAPI_H__
