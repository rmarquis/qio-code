/*
============================================================================
Copyright (C) 2013 V.

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
// eventSystem.h
#ifndef __EVENTSYSTEM_H__
#define __EVENTSYSTEM_H__

#include <shared/str.h>
#include <shared/array.h>
#include <shared/eventBaseAPI.h>

class event_c : public eventBaseAPI_i {
friend class eventList_c;
	str name;
	arraySTD_c<str> args;
	int execAfter;

	// double linked list
	event_c *next;
	event_c *prev;
public:
	event_c(int newExecTime, const char *eventName, const char *arg0 = 0, const char *arg1 = 0, const char *arg2 = 0, const char *arg3 = 0);

	virtual const char *getEventName() const {
		return name;
	}
	virtual u32 getEventArgsCount() const {
		return args.size();
	}
	virtual const char *getArg(u32 num) const {
		return args[num];
	}
	virtual int getExecTime() const {
		return execAfter;
	}
};

class eventList_c {
	event_c *head;
	event_c *tail;

	void removeEvent(event_c *e);
	void addEvent(event_c *e);
public:
	eventList_c();
	~eventList_c();

	void addEvent(int execTime, const char *eventName, const char *arg0 = 0, const char *arg1 = 0, const char *arg2 = 0, const char *arg3 = 0);
	u32 executeEvents(int timeNow, class eventReceiverBaseAPI_i *r);

	void printEventList();
};

#endif // __EVENTSYSTEM_H__


