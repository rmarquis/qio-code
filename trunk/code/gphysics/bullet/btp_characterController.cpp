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
// btp_characterController.cpp
#include "btp_characterController.h"
#include "btp_headers.h"
#include "btp_world.h"
#include "btp_convert.h"

btpCharacterController_c::btpCharacterController_c() {
	this->ch = 0;
	this->characterShape = 0;
	this->myWorld = 0;
}
btpCharacterController_c::~btpCharacterController_c() {
	destroyCharacter();
}
void btpCharacterController_c::setCharacterVelocity(const class vec3_c &newVel) {

}
void btpCharacterController_c::setCharacterEntity(class BaseEntity *ent) {
	//characterShape->setUserPointer(ent);
	ch->getGhostObject()->setUserPointer(ent);
}
void btpCharacterController_c::update(const class vec3_c &dir) {
	// set the forward direction of the character controller
	btVector3 walkDir(dir[0]*QIO_TO_BULLET,dir[1]*QIO_TO_BULLET,dir[2]*QIO_TO_BULLET);
	ch->setWalkDirection(walkDir);
}
const class vec3_c &btpCharacterController_c::getPos() const {
	btVector3 c = ch->getGhostObject()->getWorldTransform().getOrigin();
	lastPos[0] = c.x()*BULLET_TO_QIO;
	lastPos[1] = c.y()*BULLET_TO_QIO;
	lastPos[2] = c.z()*BULLET_TO_QIO;
	return lastPos;
}
bool btpCharacterController_c::isOnGround() const {
	return ch->onGround();
}
bool btpCharacterController_c::tryToJump() {
	if(ch->onGround() == false)
		return false; // didnt jump
	ch->jump(); // jumped
	return true;
}


void btpCharacterController_c::init(class bulletPhysicsWorld_c *pWorld, const class vec3_c &pos, float characterHeight, float characterWidth) {
	this->myWorld = pWorld;

	btPairCachingGhostObject* ghostObject = new btPairCachingGhostObject();
	characterShape = new btCapsuleShapeZ(characterWidth*QIO_TO_BULLET,characterHeight*QIO_TO_BULLET);
	btTransform trans;
	trans.setIdentity();
	btVector3 vPos(pos[0]*QIO_TO_BULLET,pos[1]*QIO_TO_BULLET,pos[2]*QIO_TO_BULLET);
	trans.setOrigin(vPos);
	ghostObject->setWorldTransform(trans);
	ghostObject->setCollisionShape(characterShape);
	ch = new btKinematicCharacterController (ghostObject, characterShape, 16.f*QIO_TO_BULLET,2);
	//character->setMaxSlope(DEG2RAD(70));
	/*character->setJumpSpeed(200*QIO_TO_BULLET);
	character->setFallSpeed(800*QIO_TO_BULLET);
	character->setGravity(600*QIO_TO_BULLET);*/
	//character->setUseGhostSweepTest(

	myWorld->getBTDynamicsWorld()->addCollisionObject( ghostObject, btBroadphaseProxy::CharacterFilter, 
		btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter | btBroadphaseProxy::CharacterFilter);

	myWorld->getBTDynamicsWorld()->addCharacter(ch);
}
void btpCharacterController_c::destroyCharacter() {
	if(ch == 0)
		return;
	myWorld->getBTDynamicsWorld()->removeCharacter(ch);
	myWorld->getBTDynamicsWorld()->removeCollisionObject(ch->getGhostObject());
	delete ch->getGhostObject();
	delete ch;
	delete characterShape;
	ch = 0;
	characterShape = 0;
}

