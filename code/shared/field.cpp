#include "field.h"
#include <string.h>
#include <api/rAPI.h>
#include <api/coreAPI.h>
#include <shared/str.h>
#include <client/keycodes.h>
#include <client/client.h>

bool key_overstrikeMode;

void field_s::clear() {
	memset(this->buffer, 0, MAX_EDIT_LINE);
	this->cursor = 0;
	this->scroll = 0;
}
void field_s::variableSizeDraw(int x, int y, int width, int size, bool showCursor, bool noColorEscape) {
	int drawLen = this->widthInChars - 1; // - 1 so there is always a space for the cursor
	int len = strlen( this->buffer );

	int prestep;
	// guarantee that cursor will be visible
	if ( len <= drawLen ) {
		prestep = 0;
	} else {
		if ( this->scroll + drawLen > len ) {
			this->scroll = len - drawLen;
			if ( this->scroll < 0 ) {
				this->scroll = 0;
			}
		}
		prestep = this->scroll;
	}

	if ( prestep + drawLen > len ) {
		drawLen = len - prestep;
	}

	str tmp = this->buffer + prestep;
	tmp.capLen(drawLen);

	// draw the text
	if (!showCursor) {
		rf->drawString(x, y, tmp);
	} else {
		if ((int)( cls.realtime >> 8 ) & 1) {
			tmp.insertAt(this->cursor,'|');
		} else {
			tmp.insertAt(this->cursor,' ');
		}
		rf->drawString(x, y, tmp);
	}
}

void field_s::draw(int x, int y, int width, bool showCursor, bool noColorEscape ) {
	variableSizeDraw(x, y, width, 16, showCursor, noColorEscape );
}

void field_s::paste() {
	char *cbd = Sys_GetClipboardData();

	if ( !cbd ) {
		return;
	}

	// send as if typed, so insert / overstrike works properly
	int pasteLen = strlen( cbd );
	for (int i = 0 ; i < pasteLen ; i++ ) {
		charEvent(cbd[i]);
	}

	free( cbd );
}
void field_s::keyDownEvent(int key) {
	int		len;

	// shift-insert is paste
	if ( ( ( key == K_INS ) || ( key == K_KP_INS ) ) && keys[K_SHIFT].down ) {
		paste();
		return;
	}

	key = tolower( key );
	len = strlen( this->buffer );

	switch ( key ) {
		case K_DEL:
			if ( this->cursor < len ) {
				memmove( this->buffer + this->cursor, 
					this->buffer + this->cursor + 1, len - this->cursor );
			}
			break;

		case K_RIGHTARROW:
			if ( this->cursor < len ) {
				this->cursor++;
			}
			break;

		case K_LEFTARROW:
			if ( this->cursor > 0 ) {
				this->cursor--;
			}
			break;

		case K_HOME:
			this->cursor = 0;
			break;

		case K_END:
			this->cursor = len;
			break;

		case K_INS:
			key_overstrikeMode = !key_overstrikeMode;
			break;

		default:
			break;
	}

	// Change scroll if cursor is no longer visible
	if ( this->cursor < this->scroll ) {
		this->scroll = this->cursor;
	} else if ( this->cursor >= this->scroll + this->widthInChars && this->cursor <= len ) {
		this->scroll = this->cursor - this->widthInChars + 1;
	}
}
void field_s::charEvent(int ch) {
	int		len;

	if ( ch == 'v' - 'a' + 1 ) {	// ctrl-v is paste
		paste();
		return;
	}

	if ( ch == 'c' - 'a' + 1 ) {	// ctrl-c clears the field
		clear();
		return;
	}

	len = strlen( this->buffer );

	if ( ch == 'h' - 'a' + 1 )	{	// ctrl-h is backspace
		if ( this->cursor > 0 ) {
			memmove( this->buffer + this->cursor - 1, 
				this->buffer + this->cursor, len + 1 - this->cursor );
			this->cursor--;
			if ( this->cursor < this->scroll )
			{
				this->scroll--;
			}
		}
		return;
	}

	if ( ch == 'a' - 'a' + 1 ) {	// ctrl-a is home
		this->cursor = 0;
		this->scroll = 0;
		return;
	}

	if ( ch == 'e' - 'a' + 1 ) {	// ctrl-e is end
		this->cursor = len;
		this->scroll = this->cursor - this->widthInChars;
		return;
	}

	//
	// ignore any other non printable chars
	//
	if ( ch < 32 ) {
		return;
	}

	if ( key_overstrikeMode ) {	
		// - 2 to leave room for the leading slash and trailing \0
		if ( this->cursor == MAX_EDIT_LINE - 2 )
			return;
		this->buffer[this->cursor] = ch;
		this->cursor++;
	} else {	// insert mode
		// - 2 to leave room for the leading slash and trailing \0
		if ( len == MAX_EDIT_LINE - 2 ) {
			return; // all full
		}
		memmove( this->buffer + this->cursor + 1, 
			this->buffer + this->cursor, len + 1 - this->cursor );
		this->buffer[this->cursor] = ch;
		this->cursor++;
	}


	if ( this->cursor >= this->widthInChars ) {
		this->scroll++;
	}

	if ( this->cursor == len + 1) {
		this->buffer[this->cursor] = 0;
	}
}