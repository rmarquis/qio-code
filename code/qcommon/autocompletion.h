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
// autocompletion.h
void AC_AutoComplete(char *buf, int maxSize, int *cursor);
void AC_CompleteKeyname();
void AC_CompleteFilename( const char *dir,
		const char *ext, const char *ext2, const char *ext3,
		bool stripExt, bool allowNonPureFilesOnDisk );

void AC_CompleteCommand( char *cmd,
		bool doCommands, bool doCvars );

void AC_CompleteEntityDefName();
void AC_CompleteEmitterName();
void AC_CompleteMaterialName();
void AC_CompleteMaterialFileName();