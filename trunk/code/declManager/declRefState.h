#ifndef __DECLREFSTATE_H__
#define __DECLREFSTATE_H__

class declRefState_c {
	bool referencedByClient;
	bool referencedByServer;
public:
	declRefState_c() {
		referencedByClient = false;
		referencedByServer = false;
	}
	void setReferencedByClient() {
		referencedByClient = true;
	}
	void setReferencedByServer() {
		referencedByServer = true;
	}
	void clearServerRef() {
		referencedByServer = false;
	}
	void clearClientRef() {
		referencedByClient = false;
	}
	bool isReferenced() const {
		if(referencedByClient)
			return true;
		if(referencedByServer)
			return true;
		return false;
	}
};

#endif // __DECLREFSTATE_H__
