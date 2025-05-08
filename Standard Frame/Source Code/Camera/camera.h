#pragma once
/* Notes
This is the Camera
It possess FPS Capabilities
It may support 3rd person but you need to specify the target position every frame
You will need to know the following:

StrafeCamera()		//W A S D
Mouse_Move()		//detect mouse movements and apply changes to view
Keyboard_Input()	//name suggest
Look()				//just put this once in every frame preferbably before rendering anything
Position Camera()	//you have to put this in InitEngine at the start, only have to use once unless you want to change.

To make the camera move faster, change the defination from 0.08f to other number make sure its float-type
The rest can ignore	
*/

#define CAMERASPEED		0.1f				// The Camera Speed
#define CAMERAROLL_SPD	0.01f
#define CAMERAPITC_SPD	0.01f
#define CAMERAROLL_LMT	0.7f
#define CAMERAPITC_LMT 0.54911f			//0.52366f
#define CAMERAPITC_STD 0.0f			//0.7855f

/* Radar Method Frustum Values */
#define NEAR_DIST_Z 1.0f
#define FAR_DIST_Z	1000.0f
#define PreCal_Tan	0.82842f			// Based on 2 * tan(PI / 8)
#define SCREEN_RES	1.333333f			// 1.333333 4:3 Aspect
#define ALLOWANCE_ANGLE 0.78539f

#include "Vector3D.h"

class Camera
{
	POINT mousePos;
protected:
	Vector mPos;	
	Vector mView;		
	Vector mUp;

	Vector Rel_UP, Rel_LEFT, Rel_FRONT;

	float m_YfAngle;
	float m_XfAngle;
	float m_ZfAngle;
public:
	Camera()
		:m_YfAngle(0), m_XfAngle(0), m_ZfAngle(CAMERAPITC_STD)
	{
	}
	~Camera()
	{
	}
	void Strafe_Camera(float speed);
	void Mouse_Move(int wndWidth, int wndHeight);
	void Keyboard_Input();
	void Move_Camera(float speed);
	void Rotate_View(float speed);
	void Rotate(float angle, float x, float y, float z);
	void Look();
	void Position_Camera(float pos_x, float pos_y, float pos_z, float view_x, float view_y, float view_z, float up_x, float up_y, float up_z);

	float Roll_Camera(float Fact);	// Put Values to roll the camera gives out the angle if needed (*note : < 0 indicate opposite)
	float Pitch_Camera(float Fact);
	inline float GetRollAngle() {return m_XfAngle;}
	inline float GetYawAngle()	{return m_YfAngle;}
	inline float GetPitchAngle(){return m_ZfAngle;}

	inline void SetPosition(Vector & Pos){mPos = Pos;}
	inline void SetViewTarget(Vector & Pos){mView = Pos;}
	inline Vector GetRelativeFront() {return Rel_FRONT;}

	bool IsInView(const Vector & Point, float SphereRadius = 0.0f) const;
};
