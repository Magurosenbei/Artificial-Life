#include "stdafx.h"
#include "location.h"

Location :: Location(unsigned int texID, Vector & Pos, std :: string label, std :: string PlaceType, int Slot)
:m_nTexID(texID), Position(Pos), name(label), Type(PlaceType), NumberOfPeople(0), MaxSlot(Slot)
{
}

void Location :: Render()
{
	glPushMatrix();
	glTranslatef(Position.x, Position.y, Position.z);
	g_Engine->Use_TexManager()->DrawXY(m_nTexID);
	glPopMatrix();
}

Vector& Location :: GetPosition()
{
	return Position;
}

bool Location :: InSide(Vector & Pos)
{
	if((Pos - Position).Length() < 2.0f)
		return true;
	return false;
}

bool Location :: CheckBoundary(Vector & Pos)
{
	if(Pos.x > m_nWidth / 2 + Position.x && Pos.x < -m_nWidth / 2 + Position.x)
		return false;
	if(Pos.y > m_nHeight / 2 + Position.x && Pos.y < -m_nHeight / 2 + Position.x)
		return false;
	return true;
}