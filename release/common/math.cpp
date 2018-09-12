// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// math.cpp
//

#include "racc.h"


void BuildReferential (const Vector &v_angles)
{
   // this function builds a 3D referential from a view angle, that is to say, the relative
   // "forward", "right" and "upwards" direction that a player would have if he were facing this
   // view angle. World angles are stored in Vector structs too, the "x" component corresponding
   // to the X angle (horizontal angle), and the "y" component corresponding to the Y angle
   // (vertical angle). Beware, heavy math here.

   static float degrees_to_radians = 2 * MATH_PI / 360;
   float angle;
   float sin_pitch;
   float sin_yaw;
   float sin_roll;
   float cos_pitch;
   float cos_yaw;
   float cos_roll;
	
   // compute the sine and cosine of the pitch component
   angle = v_angles.x * degrees_to_radians;
   sin_pitch = sinf (angle);
   cos_pitch = cosf (angle);

   // compute the sine and cosine of the yaw component
   angle = v_angles.y * degrees_to_radians;
   sin_yaw = sinf (angle);
   cos_yaw = cosf (angle);

   // compute the sine and cosine of the roll component
   angle = v_angles.z * degrees_to_radians;
   sin_roll = sinf (angle);
   cos_roll = cosf (angle);

   // build the FORWARD vector
   referential.v_forward.x = cos_pitch * cos_yaw;
   referential.v_forward.y = cos_pitch * sin_yaw;
   referential.v_forward.z = -sin_pitch;

   // build the RIGHT vector
   referential.v_right.x = (-(sin_roll * sin_pitch * cos_yaw) - (cos_roll * -sin_yaw));
   referential.v_right.y = (-(sin_roll * sin_pitch * sin_yaw) - (cos_roll * cos_yaw));
   referential.v_right.z = -(sin_roll * cos_pitch);

   // build the UPWARDS vector
   referential.v_up.x = ((cos_roll * sin_pitch * cos_yaw) - (sin_roll * -sin_yaw));
   referential.v_up.y = ((cos_roll * sin_pitch * sin_yaw) - (sin_roll * cos_yaw));
   referential.v_up.z = cos_roll * cos_pitch;

   return;
}


void BuildPlayerReferential (const Vector &v_angles, player_t *pPlayer)
{
   // this function builds a 3D referential from a view angle, that is to say, the relative
   // "forward", "right" and "upwards" direction that a player would have if he were facing this
   // view angle. World angles are stored in Vector structs too, the "x" component corresponding
   // to the X angle (horizontal angle), and the "y" component corresponding to the Y angle
   // (vertical angle). Beware, heavy math here.
   // This function is basically the same as the BuildReferential() function above, except that
   // whereas BuildReferential() builds a global referential, this one builds pPlayer's own
   // referential (which is stored in his player_t structure).

   static float degrees_to_radians = 2 * MATH_PI / 360;
   float angle;
   float sin_pitch;
   float sin_yaw;
   float sin_roll;
   float cos_pitch;
   float cos_yaw;
   float cos_roll;
	
   // compute the sine and cosine of the pitch component
   angle = v_angles.x * degrees_to_radians;
   sin_pitch = sinf (angle);
   cos_pitch = cosf (angle);

   // compute the sine and cosine of the yaw component
   angle = v_angles.y * degrees_to_radians;
   sin_yaw = sinf (angle);
   cos_yaw = cosf (angle);

   // compute the sine and cosine of the roll component
   angle = v_angles.z * degrees_to_radians;
   sin_roll = sinf (angle);
   cos_roll = cosf (angle);

   // build the FORWARD vector
   pPlayer->v_forward.x = cos_pitch * cos_yaw;
   pPlayer->v_forward.y = cos_pitch * sin_yaw;
   pPlayer->v_forward.z = -sin_pitch;

   // build the RIGHT vector
   pPlayer->v_right.x = (-(sin_roll * sin_pitch * cos_yaw) - (cos_roll * -sin_yaw));
   pPlayer->v_right.y = (-(sin_roll * sin_pitch * sin_yaw) - (cos_roll * cos_yaw));
   pPlayer->v_right.z = -(sin_roll * cos_pitch);

   // build the UPWARDS vector
   pPlayer->v_up.x = ((cos_roll * sin_pitch * cos_yaw) - (sin_roll * -sin_yaw));
   pPlayer->v_up.y = ((cos_roll * sin_pitch * sin_yaw) - (sin_roll * cos_yaw));
   pPlayer->v_up.z = cos_roll * cos_pitch;

   return;
}


Vector VecToAngles (const Vector &v_forward)
{
   // this function returns the world angles towards which is directed the vector passed in by
   // v_VectorIn. World angles are how much degrees on the horizontal plane and how much on the
   // vertical plane one has to turn to face a certain direction (here, the direction the vector
   // is pointing towards). World angles are stored in Vector structs too, the "x" component
   // corresponding to the X angle (horizontal angle), and the "y" component corresponding to
   // the Y angle (vertical angle). It's roughly the opposite of the BuildReferential() function.

   Vector v_angles;

   // is the input vector absolutely vertical ?
   if ((v_forward.y == 0) && (v_forward.x == 0))
   {
      // is the input vector pointing up ?
      if (v_forward.z > 0)
         v_angles.x = 90; // look upwards
      else
         v_angles.x = -90; // look downwards

      v_angles.y = 0;
      v_angles.z = 0;
   }

   // else it's another sort of vector
   else
   {
      // compute individually the pitch and yaw corresponding to this vector
      v_angles.x = (atan2 (v_forward.z, sqrt (v_forward.x * v_forward.x + v_forward.y * v_forward.y)) * 180 / MATH_PI);
      v_angles.y = (atan2 (v_forward.y, v_forward.x) * 180 / MATH_PI);
      v_angles.z = 0;
   }

   return (WrapAngles (v_angles)); // don't forget to wrap the final angles around
}


float WrapAngle (float angle)
{
   // this function adds or substracts 360 enough times needed to the angle_to_wrap angle in
   // order to set it into the range -180/+180 and returns the resulting angle. Letting the
   // engine have a hand on angles that are outside these bounds may cause the game to freeze
   // by screwing up the engine math code.

   // check for wraparound of angle
   if (angle >= 180)
      angle -= 360 * abs (((int) angle + 180) / 360);
   else if (angle < -180)
      angle += 360 * abs (((int) angle - 180) / 360);

   // needs a 2nd pass to check for floating part truncation (rounded 180)
   if (angle == 180.0)
      angle = -180;

   return (angle);
}


float WrapAngle360 (float angle)
{
   // this function adds or substracts 360 enough times needed to the angle_to_wrap angle in
   // order to set it into the range +0/+360 and returns the resulting angle Letting the
   // engine have a hand on angles that are outside these bounds may cause the game to freeze
   // by screwing up the engine math code.

   // check for wraparound of angle
   if (angle >= 360)
      angle -= 360 * abs ((int) angle / 360);
   else if (angle < 0)
      angle += 360 * abs (((int) angle - 360) / 360);

   // needs a 2nd pass to check for floating part truncation (rounded 180)
   if (angle == 360.0)
      angle = 0;

   return (angle);
}


Vector WrapAngles (Vector &angles)
{
   // this function adds or substracts 360 enough times needed to every of the three components
   // of the axial angles structure angles_to_wrap in order to set them into the range -180/+180
   // and returns the resulting axial angles structure.

   // check for wraparound of angles
   if (angles.x >= 180)
      angles.x -= 360 * abs (((int) angles.x + 180) / 360);
   else if (angles.x < -180)
      angles.x += 360 * abs (((int) angles.x - 180) / 360);
   if (angles.y >= 180)
      angles.y -= 360 * abs (((int) angles.y + 180) / 360);
   else if (angles.y < -180)
      angles.y += 360 * abs (((int) angles.y - 180) / 360);
   if (angles.z >= 180)
      angles.z -= 360 * abs (((int) angles.z + 180) / 360);
   else if (angles.z < -180)
      angles.z += 360 * abs (((int) angles.z - 180) / 360);

   // needs a 2nd pass to check for floating part truncation (rounded 180)
   if (angles.x == 180.0)
      angles.x = -180;
   if (angles.y == 180.0)
      angles.y = -180;
   if (angles.z == 180.0)
      angles.z = -180;

   return (angles);
}


Vector WrapAngles360 (Vector &angles)
{
   // this function adds or substracts 360 enough times needed to every of the three components
   // of the axial angles structure angles_to_wrap in order to set them into the range +0/+360
   // and returns the resulting axial angles structure.

   // check for wraparound of angles
   if (angles.x >= 360)
      angles.x -= 360 * abs (((int) angles.x) / 360);
   else if (angles.x < 0)
      angles.x += 360 * abs (((int) angles.x - 360) / 360);
   if (angles.y >= 360)
      angles.y -= 360 * abs (((int) angles.y) / 360);
   else if (angles.y < 0)
      angles.y += 360 * abs (((int) angles.y - 360) / 360);
   if (angles.z >= 360)
      angles.z -= 360 * abs (((int) angles.z) / 360);
   else if (angles.z < 0)
      angles.z += 360 * abs (((int) angles.z - 360) / 360);

   // needs a 2nd pass to check for floating part truncation (rounded 360)
   if (angles.x == 360.0)
      angles.x = 0;
   if (angles.y == 360.0)
      angles.y = 0;
   if (angles.z == 360.0)
      angles.z = 0;

   return (angles);
}


float AngleOfVectors (Vector v1, Vector v2)
{
   // this function returns the angle in degrees between the v1 and v2 vectors, regardless of
   // the axial planes (ie, considering the plane formed by the v1 and v2 vectors themselves)

   static float v1norm_dotprod_v2norm;
   static Vector normalized_v1, normalized_v2;

   if ((v1 == g_vecZero) || (v2 == g_vecZero))
      return (0); // reliability check (avoid zero divide)

   // normalize v1 and v2 (tip from botman)
   normalized_v1 = v1.Normalize ();
   normalized_v2 = v2.Normalize ();

   // reminder: dot product = (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z)
   v1norm_dotprod_v2norm = normalized_v1.x * normalized_v2.x
                           + normalized_v1.y * normalized_v2.y
                           + normalized_v1.z * normalized_v2.z;

   // how on Earth come that a dotproduct of normalized vectors can outbound [-1, 1] ???
   // A couple 'forestry worker' casts to double seem to make the problem occur less often, but
   // still, we have to check the value to ensure it will be in range...

   if (v1norm_dotprod_v2norm < -1)
      return (180); // reliability check (avoid acos range error)
   else if (v1norm_dotprod_v2norm > 1)
      return (0); // reliability check (avoid acos range error)

   return (WrapAngle (acos (v1norm_dotprod_v2norm) * 180 / MATH_PI));
}
