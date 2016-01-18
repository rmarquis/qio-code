
class btConvexHullShape *BT_CModelHullToConvex(const class cmHull_i *h, const vec3_c *ofs = 0, const vec3_c *pScale = 0);
class btConvexHullShape *BT_ConvexHullShapeFromVerticesArray(const btAlignedObjectArray<btVector3> &vertices);
void BT_ConvertVerticesArrayFromQioToBullet(btAlignedObjectArray<btVector3> &vertices);
btConvexHullShape *BT_CModelTriMeshToConvex(const class cmTriMesh_i *triMesh, const vec3_c *ofs = 0, const vec3_c *pScale = 0);
btCollisionShape *BT_CModelToBulletCollisionShape(const class cMod_i *cModel, bool bIsStatic, class vec3_c *extraCenterOfMassOffset = 0, const vec3_c *pScale = 0);
void BT_AddCModelToCompoundShape(btCompoundShape *compound, const class btTransform &localTrans, const class cMod_i *cmodel);

