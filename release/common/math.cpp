// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// The game to engine interfacing code is based on the work done by Jeffrey 'Botman' Broome
// (http://planethalflife.com/botman/)
//
// This project is partially based on the work done by Eric Bieschke in his BSDbot
// (http://gamershomepage.com/csbot/)
//
// This project is partially based on the work done by Brendan 'Spyro' McCarthy in his ODD Bot
// (http://oddbot.hlfusion.com/)
//
// This project is partially based on the work done by Alistair 'eLiTe' Stewart in his TEAMbot
// (http://www.planethalflife.com/teambot/)
//
// This project is partially based on the work done by Johannes '@$3.1415rin' Lampel in his JoeBot
// (http://www.joebot.net/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// math.cpp
//

#include "racc.h"


#define PI 3.14159265358979323846


float WrapAngle (float angle_to_wrap)
{
   // this function adds or substracts 360 enough times needed to the angle_to_wrap angle in
   // order to set it into the range -180/+180 and returns the resulting angle

   static float angle;

   angle = angle_to_wrap; // update this function's static variable with the angle to wrap

   // check for wraparound of angle
   if (angle > 180)
      angle -= 360 * ((int) (angle / 360) + 1);
   if (angle < -180)
      angle += 360 * ((int) (angle / 360) + 1);

   return (angle);
}


float WrapAngle360 (float angle_to_wrap)
{
   // this function adds or substracts 360 enough times needed to the angle_to_wrap angle in
   // order to set it into the range +0/+360 and returns the resulting angle

   static float angle;

   angle = angle_to_wrap; // update this function's static variable with the angle to wrap

   // check for wraparound of angle
   if (angle > 360)
      angle -= 360 * (int) (angle / 360);
   if (angle < 0)
      angle += 360 * ((int) (angle / 360) + 1);

   return (angle);
}


vector WrapAngles (vector &angles_to_wrap)
{
   // this function adds or substracts 360 enough times needed to every of the three components
   // of the axial angles structure angles_to_wrap in order to set them into the range -180/+180
   // and returns the resulting axial angles structure

   static vector angles;

   angles = angles_to_wrap; // update this function's static variable with the angles to wrap

   // check for wraparound of angles
   if (angles.x > 180)
      angles.x -= 360 * ((int) (angles.x / 360) + 1);
   if (angles.x < -180)
      angles.x += 360 * ((int) (angles.x / 360) + 1);
   if (angles.y > 180)
      angles.y -= 360 * ((int) (angles.y / 360) + 1);
   if (angles.y < -180)
      angles.y += 360 * ((int) (angles.y / 360) + 1);
   if (angles.z > 180)
      angles.z -= 360 * ((int) (angles.z / 360) + 1);
   if (angles.z < -180)
      angles.z += 360 * ((int) (angles.z / 360) + 1);

   return (angles);
}


vector WrapAngles360 (vector &angles_to_wrap)
{
   // this function adds or substracts 360 enough times needed to every of the three components
   // of the axial angles structure angles_to_wrap in order to set them into the range +0/+360
   // and returns the resulting axial angles structure

   static vector angles;

   angles = angles_to_wrap; // update this function's static variable with the angles to wrap

   // check for wraparound of angles
   if (angles.x > 360)
      angles.x -= 360 * (int) (angles.x / 360);
   if (angles.x < 0)
      angles.x += 360 * ((int) (angles.x / 360) + 1);
   if (angles.y > 360)
      angles.y -= 360 * (int) (angles.y / 360);
   if (angles.y < 0)
      angles.y += 360 * ((int) (angles.y / 360) + 1);
   if (angles.z > 360)
      angles.z -= 360 * (int) (angles.z / 360);
   if (angles.z < 0)
      angles.z += 360 * ((int) (angles.z / 360) + 1);

   return (angles);
}


float AngleBetweenVectors (vector &vec1, vector &vec2)
{
   // this function returns the angle in degrees between the v1 and v2 vectors, regardless of
   // the axial planes (ie, considering the plane formed by the v1 and v2 vectors themselves)

   if ((vec1.Length () == 0) || (vec2.Length () == 0))
      return (0.0); // avoid zero divide

   return (WrapAngle (acos (DotProduct (vec1, vec2) / (vec1.Length () * vec2.Length ())) * 180 / PI));
}


inline float DotProduct (vector &vec1, vector &vec2)
{
   // this function returns the dot product of the vec1 and vec2 vectors

   return (vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z); // compute and return the result
}


inline vector CrossProduct (vector &vec1, vector &vec2)
{
   // this function returns the cross product of the vec1 and vec2 vectors. Local variables have
   // been made static for speeding up recursive calls of this function.

   static vector v_crossproduct_vector;

   // just apply the formula...
   v_crossproduct_vector.x = vec1.y * vec2.z - vec1.z * vec2.y;
   v_crossproduct_vector.y = vec1.z * vec2.x - vec1.x * vec2.z;
   v_crossproduct_vector.z = vec1.x * vec2.y - vec1.y * vec2.x;

   return (v_crossproduct_vector); // and return the resulting crossproduct vector
}
