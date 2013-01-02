

class gamePlayerModel_i {
public:
	virtual const class aabb &getStandHull() const = 0;
	//virtual const aabb &getCrouchHull() const = 0;
	virtual const class aabb &getStandViewHeight() const = 0;
	//virtual const aabb &getCrouchViewHeight() const = 0;
};

class gQ3PlayerModel_c : public gamePlayerModel_i {
	

};
