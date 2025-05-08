#pragma once

class Location;
class Character;

enum Place_Code {BED, BATH, PHONE, KITCHEN, STUDY, PLAY, HOME, OUTSIDE};

class House
{
	int m_nWidth, m_nHeight;
	unsigned int m_nTexID;
	
	std :: vector <Location> Places;
	Vector Position;
	
	Character * Owner;
	int m_nFoodStock;			// Used by Kitchen
	
	lua_State * State;
public:
	House(const std :: string FilePath, Character * Own);
	~House();
	
	void	AddLocation(const std :: string TexturePath, Vector & Pos, std :: string Label, int Slot);
	void	BootUp();	// Setup Script
	void	Render();	// Draw all the location + house
	
	bool	CheckInLocation(unsigned int index, Vector& Pos);
	bool	CheckBoundary(unsigned int index, Vector& Pos);
	bool	CheckInHouse(Vector & Pos);
	bool	IsLocationFull(std :: string Label);
	Vector	GetLocation(std :: string Label);
	Vector&	GetPosition();

	std :: string	AtWhichArea(Character * Person);
	void			ResetPlaceCounter();
};