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
// Weapon_QioFlashLight.h - Qio FlashLight weapon
#include "Weapon_QioFlashLight.h"

DEFINE_CLASS(Weapon_QioFlashLight, "Weapon");

Weapon_QioFlashLight::Weapon_QioFlashLight() {
	this->setDelayBetweenShots(0);
	this->fillClip(0);
}
Weapon_QioFlashLight::~Weapon_QioFlashLight() {
	disableFlashLight();
}

void Weapon_QioFlashLight::disableFlashLight() {
	delete myLight.getPtr();
}
void Weapon_QioFlashLight::enableFlashLight() {
	if(myLight.getPtr())
		return;
	Light *l = new Light;
	l->setSpotLightRadius(32.f);
	l->setRadius(400.f);
	l->setParent(this,0);
	myLight = l;
}

void Weapon_QioFlashLight::onFireKeyDown() {
	// toggle the light
	if(myLight.getPtr()) {
		disableFlashLight();
		this->fillClip(0);
	} else {
		enableFlashLight();
		this->fillClip(1);
	}
}
