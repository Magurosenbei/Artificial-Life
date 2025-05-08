#include "stdafx.h"
#include "camera.h"

/*
GetCursor and SetCursor seems to leak memory. take note

*/

void Camera::Position_Camera(float pos_x,  float pos_y,  float pos_z,
							  float view_x, float view_y, float view_z,
							  float up_x,   float up_y,   float up_z)
{
	mPos	= Vector(pos_x,  pos_y,  pos_z ); // set position
	mView	= Vector(view_x, view_y, view_z); // set view
	mUp		= Vector(up_x,   up_y,   up_z  ); // set the up vector	
}

void Camera::Move_Camera(float speed)
{
	Vector vVector = mView - mPos;	// Get the view vector
	
	// forward positive cameraspeed and backward negative -cameraspeed.
	mPos.x  = mPos.x  + vVector.x * speed;
	mPos.z  = mPos.z  + vVector.z * speed;
	mView.x = mView.x + vVector.x * speed;
	mView.z = mView.z + vVector.z * speed;
}

void Camera::Rotate_View(float speed)
{
	Vector vVector = mView - mPos;	// Get the view vector

	mView.z = (float)(mPos.z + sin(speed)*vVector.x + cos(speed)*vVector.z);
	mView.x = (float)(mPos.x + cos(speed)*vVector.x - sin(speed)*vVector.z);
}

void Camera::Strafe_Camera(float speed)
{
	Vector vVector = mView - mPos;	// Get the view vector
	Vector vOrthoVector;              // Orthogonal vector for the view vector

	vOrthoVector.x = -vVector.z;
	vOrthoVector.z =  vVector.x;

	// left positive cameraspeed and right negative -cameraspeed.
	mPos.x  = mPos.x  + vOrthoVector.x * speed;
	mPos.z  = mPos.z  + vOrthoVector.z * speed;
	mView.x = mView.x + vOrthoVector.x * speed;
	mView.z = mView.z + vOrthoVector.z * speed;
}

void Camera::Look()
{
	gluLookAt(mPos.x,  mPos.y,  mPos.z,	
			  mView.x, mView.y, mView.z,	
			  mUp.x,   mUp.y,   mUp.z);
	
	Rel_FRONT = (mView - mPos).Normalize();
	Rel_LEFT = Ref_Vec_Y.Cross(Rel_FRONT).Normalize();
	Rel_UP = Rel_FRONT.Cross(Rel_LEFT).Normalize();
}

void Camera::Mouse_Move(int wndWidth, int wndHeight)
{
	int mid_x = wndWidth  >> 1;	
	int mid_y = wndHeight >> 1;	
	float angle_y  = 0.0f;				
	float angle_z  = 0.0f;							
		
	GetCursorPos(&mousePos);	// Get the 2D mouse cursor (x,y) position					
	
	if((mousePos.x == mid_x) && (mousePos.y == mid_y)) return;

	SetCursorPos(mid_x, mid_y);	// Set the mouse cursor in the center of the window						

	// Get the direction from the mouse cursor, set a resonable maneuvering speed
	angle_y = (float)((mid_x - mousePos.x)) / 1000;		
	angle_z = (float)((mid_y - mousePos.y)) / 100;

	// The higher the value is the faster the camera looks around.
	mView.y += angle_z * 2;

	// limit the rotation around the x-axis
	if((mView.y - mPos.y) > 100)  mView.y = mPos.y + 100;
	if((mView.y - mPos.y) <-100)  mView.y = mPos.y - 100;
	
	Rotate_View(-angle_y * 2.0f); // Rotate
	//m_YfAngle = Rel_FRONT.AngleBetween_XZ(Ref_Vec_Z);
}

void Camera :: Rotate(float angle, float x, float y, float z)
{
	// get direction facing
	Vector dir = mPos - mView ;
	// calculate sine, cosine of angle
	float cos_angle = (float) cos( angle );
	float sin_angle = (float) sin( angle );
	// calculate new dir after rotation
	Vector newdir;
	newdir.x = (cos_angle + (1 - cos_angle) * x) * dir.x;
	newdir.x += ((1 - cos_angle) * x * y - z * sin_angle) * dir.y;
	newdir.x += ((1 - cos_angle) * x * z + y * sin_angle) * dir.z;
	newdir.y = ((1 - cos_angle) * x * y + z * sin_angle) * dir.x;
	newdir.y += (cos_angle + (1 - cos_angle) * y) * dir.y;
	newdir.y += ((1 - cos_angle) * y * z - x * sin_angle) * dir.z;
	newdir.z = ((1 - cos_angle) * x * z - y * sin_angle) * dir.x;
	newdir.z += ((1 - cos_angle) * y * z + x * sin_angle) * dir.y;
	newdir.z += (cos_angle + (1 - cos_angle) * z) * dir.z;
	mPos = mView + newdir;
}

void Camera :: Keyboard_Input()
{
	if(Key('W'))	Move_Camera(CAMERASPEED);
	if(Key('S'))	Move_Camera(-CAMERASPEED);
	if(Key('A'))	Strafe_Camera(-CAMERASPEED);
	if(Key('D'))	Strafe_Camera(CAMERASPEED);
	
	if(Key(VK_RIGHT))		Roll_Camera(CAMERAROLL_SPD);
	else if(Key(VK_LEFT))	Roll_Camera(-CAMERAROLL_SPD);
	else					Roll_Camera(0);

	if(Key(VK_UP))			Pitch_Camera(CAMERAPITC_SPD);
	else if(Key(VK_DOWN))	Pitch_Camera(-CAMERAPITC_SPD);
	else					Pitch_Camera(0);
	m_YfAngle = Rel_FRONT.AngleBetween_XZ(Ref_Vec_Z);
}

float Camera :: Roll_Camera(float Fact)
{
	if(Fact == 0)
	{
		(m_XfAngle > 0)? Fact = - CAMERAROLL_SPD : Fact = CAMERAROLL_SPD;
		if(m_XfAngle - Fact < 0 && m_XfAngle + Fact > 0)	// no need to rotate if its angle 0
		{
			Fact = 0;
			mUp = Ref_Vec_Y;
			m_XfAngle = 0;
			return m_XfAngle;
		}
	}
	if((m_XfAngle > CAMERAROLL_LMT && Fact > 0) || (m_XfAngle < -CAMERAROLL_LMT && Fact < 0))
		Fact = 0;

	m_XfAngle += Fact;

	mUp = Ref_Vec_Y.RotateFromPoint_Z_POLE(Vector(), m_XfAngle);
	mUp = mUp.RotateFromPoint_Y_POLE(Vector(), m_YfAngle);
	
	return m_XfAngle;
}

float Camera :: Pitch_Camera(float Fact)
{
	if(Fact == 0)
	{
		(m_ZfAngle > CAMERAPITC_STD)? Fact = - CAMERAPITC_SPD : Fact = CAMERAPITC_SPD;
		if(m_ZfAngle - Fact < CAMERAPITC_STD && m_ZfAngle + Fact > CAMERAPITC_STD)	// no need to rotate if its angle 0
		{
			Fact = 0;
			m_ZfAngle = CAMERAPITC_STD;
			return m_XfAngle;
		}
	}
	if((m_ZfAngle > CAMERAPITC_LMT + CAMERAPITC_STD && Fact > 0) || (m_ZfAngle < CAMERAPITC_STD - CAMERAPITC_LMT && Fact < 0))
		Fact = 0;

	m_ZfAngle += Fact;
	return m_ZfAngle;
}

bool Camera :: IsInView(const Vector & Point, float SphereRadius) const
{
	// Skip usage of operator by going construct directly
	Vector Relative_Point(Point - mPos);

	float Dist_F = Rel_FRONT.Dot(Relative_Point);
	if(Dist_F + SphereRadius > FAR_DIST_Z || Dist_F + SphereRadius < NEAR_DIST_Z)
		return false;

	float Height = Dist_F * PreCal_Tan; // 2 * tan(45 / 2)
	Dist_F = Relative_Point.Dot(Rel_UP);
	if(Dist_F > Height / 2 + SphereRadius || Dist_F < -Height / 2 - SphereRadius)
		return false;

	Height = Height * SCREEN_RES;
	Dist_F = Relative_Point.Dot(Rel_LEFT);

	if(Dist_F > Height / 2 + SphereRadius || Dist_F < - Height / 2 - SphereRadius)
		return false;
	return true;
}
