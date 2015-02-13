using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using tMath;

namespace shared
{
    class Camera
    {
        // spherical coordinates
        private double theta;
        private double phi;
        private double radius;
        // the X,Y,Z position of the camera, derived from spherical coordinates
        private Vec3 position;

        public Camera()
        {
            radius = 150.0;
            updateCameraData();
        }
        private void updateCameraData()
        {
            double phiRadians = (Math.PI / 180.0) * phi;
            double thetaRadians = (Math.PI / 180.0) * theta;
            position.set((radius * Math.Cos(thetaRadians) * Math.Sin(phiRadians)),
                (radius * Math.Sin(thetaRadians) * Math.Sin(phiRadians)),
                (radius * Math.Cos(phiRadians)));
        }
        // looking right/left
        public void rotateTheta(float ofs)
        {
            theta += ofs;

            updateCameraData();
        }
        // looking up/down
        public void rotatePhi(float ofs)
        {
            phi += ofs;
            // clamp the phi to the valid range
            if (phi <= 0)
                phi = 1;
            else if (phi >= 180)
                phi = 179;

            updateCameraData();
        }
        // zooming in/out
        public void deltaRadius(float ofs)
        {
            radius += ofs;
            if (radius < 0)
            {
                radius = 0.1;
            }
            updateCameraData();
        }
        public Vec3 getPosition()
        {
            return position;
        }
        public Vec3 getLookAt()
        {
            return new Vec3(0, 0, 0);
        }
        public double getPhi()
        {
            return phi;
        }
        public double getTheta()
        {
            return theta;
        }
    }
}
