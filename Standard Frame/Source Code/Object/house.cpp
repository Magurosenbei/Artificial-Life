#include "stdafx.h"
#include "house.h"

House :: House(const std :: string FilePath, Character * Own)
{
	Owner = Own;
	State = lua_open();
	luabind::open(State);
	luaL_openlibs(State);
	Vector :: Register(State);

	luabind::module(State) 
	[
		luabind::class_<House>("House")
		.def_readwrite("Position", &House :: Position)
		.def_readwrite("Food_Stock", &House :: m_nFoodStock)
		.def("AddLocation", &House :: AddLocation)
	];
	luabind::globals(State)["House"] = this;
	
	luaL_dofile(State, FilePath.c_str());
	
	lua_getglobal(State, "Texture_Path");
	ASSERT( lua_isstring(State, -1) , "No Texture Path");
	m_nTexID = g_Engine->Use_TexManager()->AddTexture(lua_tostring(State, -1));
	m_nWidth = g_Engine->Use_TexManager()->GetTexData(m_nTexID)->width;
	m_nHeight = g_Engine->Use_TexManager()->GetTexData(m_nTexID)->height;

	lua_close(State);
}

House :: ~House()
{
	Owner = NULL;
	Places.clear();
}

void House :: AddLocation(const std :: string TexturePath, Vector & Pos, std :: string Label, int Slot)
{
	Places.push_back(Location(g_Engine->Use_TexManager()->AddTexture(TexturePath), Pos + Position, Label, "", Slot));
}

void House :: BootUp()
{
	
}

void House :: Render()
{
	glPushMatrix();
	glTranslatef(Position.x, Position.y, Position.z);
	g_Engine->Use_TexManager()->DrawXY(m_nTexID);
	glPopMatrix();
	
	for(unsigned int index = 0; index < Places.size(); index++)
		Places[index].Render();
}

bool House :: CheckInLocation(unsigned int index, Vector & Pos)
{
	if(Places[index].InSide(Pos))
		return true;
	return false;
}

bool House :: CheckBoundary(unsigned int index, Vector& Pos)
{
	if(Places[index].CheckBoundary(Pos))
		return true;
	return false;
}

bool House :: CheckInHouse(Vector & Pos)
{
	if(Pos.x > m_nWidth / 2 + Position.x && Pos.x < -m_nWidth / 2 + Position.x)
		return false;
	if(Pos.y > m_nHeight / 2 + Position.x && Pos.y < -m_nHeight / 2 + Position.x)
		return false;
	return true;
}

bool House ::	IsLocationFull(std :: string Label)
{
	for(unsigned int index = 0; index < Places.size(); index++)
		if(!Places[index].name.compare(Label))
		{
			WriteCommandLine("%i", Places[index].NumberOfPeople);
			if(Places[index].NumberOfPeople < Places[index].MaxSlot)
				return false;
			return true;
		}
	return false;
}

Vector House ::	GetLocation(std :: string Label)
{
	for(unsigned int index = 0; index < Places.size(); index++)
		if(!Places[index].name.compare(Label))
		{
			if(Places[index].NumberOfPeople == Places[index].MaxSlot)
				continue;
			return Places[index].GetPosition();
		}
	return Vector(0, 0, -5.0f);	// not found
}

Vector& House :: GetPosition()
{
	return Position;
}

std :: string House :: AtWhichArea(Character * Person)
{
	for(unsigned int index = 0; index < Places.size(); index++)
		if(Places[index].InSide(Person->GetPosition()))
		{
			Places[index].NumberOfPeople++;
			return Places[index].name;
		}
	return "NOWHERE";
}

void House :: ResetPlaceCounter()
{	
	for(unsigned int index = 0; index < Places.size(); index++)
		Places[index].NumberOfPeople = 0;
}