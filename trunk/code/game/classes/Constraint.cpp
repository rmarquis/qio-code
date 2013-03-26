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
// Constraint.cpp
#include "Constraint.h"
#include "../g_local.h"
#include <api/physAPI.h>

DEFINE_CLASS(Constraint, "BaseEntity");
DEFINE_CLASS_ALIAS(Constraint, func_constraint);

Constraint::Constraint() {
	type = PCT_BALL;
}
void Constraint::postSpawn2() {
	//e0 = G_FindFirstEntityWithTargetName(e0TargetName);
	//e1 = G_FindFirstEntityWithTargetName(e1TargetName);
	//if(e0 == 0) {
	//	if(e1 == 0) {
	//		return;
	//	}
	//	e0 = e1;
	//	e1 = 0;
	//}
	//if(e0->getRigidBody() == 0)
	//	return;
	//vec3_c anchor = this->getOrigin();
	//matrix_c cMat;
	//cMat.setupOrigin(anchor.x,anchor.y,anchor.z);
	//matrix_c b0Mat = e0->getMatrix().getInversed()*cMat;
	//btTransform frameA;
	//frameA.setFromOpenGLMatrix(b0Mat);
	//frameA.scaleOrigin(QIO_TO_BULLET);
	//if(type == PCT_BALL) {
	//	btGeneric6DofConstraint *bc;
	//	if(e1 && e0) {
	//		matrix_c b1Mat = e1->getMatrix().getInversed()*cMat;
	//		btTransform frameB;
	//		frameB.setFromOpenGLMatrix(b1Mat);
	//		frameB.scaleOrigin(QIO_TO_BULLET);
	//		bc = new btGeneric6DofConstraint(*e0->getRigidBody(),*e1->getRigidBody(),frameA,frameB,false);
	//	} else {
	//		bc = new btGeneric6DofConstraint(*e0->getRigidBody(),frameA,false);
	//	}
	//	// lock linear transforms
	//	bc->setLimit(0,0,0);
	//	bc->setLimit(1,0,0);
	//	bc->setLimit(2,0,0);
	//	// free the angular axes
	//	bc->setLimit(3,-1,0);
	//	bc->setLimit(4,-1,0);
	//	bc->setLimit(5,-1,0);
	//	dynamicsWorld->addConstraint(bc);
	//} else if(type == PCT_HINGE) {
	//	btHingeConstraint *hinge;
	//	if(e1 && e0) {
	//		matrix_c b1Mat = e1->getMatrix().getInversed()*cMat;
	//		btTransform frameB;
	//		frameB.setFromOpenGLMatrix(b1Mat);
	//		frameB.scaleOrigin(QIO_TO_BULLET);
	//		hinge = new btHingeConstraint(*e0->getRigidBody(),*e1->getRigidBody(),frameA,frameB,false);
	//	} else {
	//		hinge = new btHingeConstraint(*e0->getRigidBody(),frameA,false);
	//	}
	//	btVector3 ax(0,0,1);
	//	hinge->setAxis(ax);
	//	dynamicsWorld->addConstraint(hinge);
	//}
}
void Constraint::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"body0")) {
		e0TargetName = value;
	} else if(!stricmp(key,"body1")) {
		e1TargetName = value;
	} else if(!stricmp(key,"type")) {
		if(!stricmp(value,"hinge")) {
			type = PCT_HINGE;
		}	
	} else {
		BaseEntity::setKeyValue(key,value);
	}
}
