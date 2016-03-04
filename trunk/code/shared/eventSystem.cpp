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
// eventSystem.cpp
#include "eventSystem.h"
#include "eventReceiverAPI.h"
#include <assert.h>

event_c::event_c(int newExecTime, const char *eventName, const char *arg0, const char *arg1, const char *arg2, const char *arg3) {
	this->prev = this->next = 0;
	this->execAfter = newExecTime;

	this->name = eventName;
	if(arg0 == 0) {
		return;
	}
	this->args.push_back(arg0);
	if(arg1 == 0) {
		return;
	}
	this->args.push_back(arg1);
	if(arg2 == 0) {
		return;
	}
	this->args.push_back(arg2);
	if(arg3 == 0) {
		return;
	}
	this->args.push_back(arg3);
}

void eventList_c::removeEvent(event_c *e) {
	if(head == tail) {
		assert(e == head);
		assert(e->next == 0);
		assert(e->prev == 0);
		head = tail = 0;
	} else {
		if(e->prev) {
			e->prev->next = e->next;
		} else {
			head = e->next;
			head->prev = 0;
		}
		if(e->next) {
			e->next->prev = e->prev;
		} else {
			tail = e->prev;
			tail->next = 0;
		}
	}
	delete e;
}
void eventList_c::addEvent(event_c *e) {
	if(head == 0) {
		assert(tail == 0);
		head = tail = e;
		return;
	}
	event_c *tmp = head;
	while(e->getExecTime() > tmp->getExecTime()) {
		if(tmp->next == 0) {
			// add to tail
			tmp->next = e;
			e->prev = tmp;
			tail = e;
			return;
		}
		tmp = tmp->next;
	}
	// add before tmp
	e->prev = tmp->prev;
	e->next = tmp;
	tmp->prev = e;
	if(e->prev) {
		e->prev->next = e;
	} else {
		head = e;
	}
}

eventList_c::eventList_c() {
	head = 0;
	tail = 0;
}
eventList_c::~eventList_c() {
	while(head) {
		removeEvent(head);
	}
}

void eventList_c::addEvent(int execTime, const char *eventName, const char *arg0, const char *arg1, const char *arg2, const char *arg3) {
	addEvent(new event_c(execTime,eventName, arg0, arg1, arg2, arg3));
}
u32 eventList_c::executeEvents(int timeNow, eventReceiverBaseAPI_i *r) {
	u32 c_executed = 0;
	while(head && (head->getExecTime() < timeNow)) {
		r->processEvent(head);
		this->removeEvent(head);
		c_executed++;
	}
	return c_executed;
}
void eventList_c::printEventList() {
	event_c *e = head;
	while(e) {
		printf("%i: %s\n",e->getExecTime(),e->getEventName());
		e = e->next;
	}
}

//class testEventSys_c {
//public:
//	testEventSys_c() {
//		eventList_c l;
//		l.addEvent(100,"testEvent");
//		l.addEvent(30,"testEvent");
//		l.addEvent(50,"testEvent");
//		l.addEvent(60,"testEvent");
//		l.addEvent(130,"testEvent");
//		l.addEvent(140,"testEvent");
//		l.addEvent(15,"testEvent");
//		l.addEvent(200,"testEvent");
//		l.addEvent(12,"testEvent");
//		l.addEvent(300,"testEvent");
//		l.addEvent(450,"testEvent");
//		l.addEvent(600,"testEvent");
//		l.printEventList();
//	}
//} testEventSys;
//
