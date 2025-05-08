#pragma once

class Location
{
	int m_nWidth, m_nHeight;
	unsigned int m_nTexID;	
	 
	Vector Position;
public:
	std :: string name;
	std :: string Type;
	int NumberOfPeople;
	int MaxSlot;

	Location(unsigned int texID, Vector & Pos, std :: string label, std :: string PlaceType, int Slot);
	
	void	Render();
	Vector&	GetPosition();
	bool	InSide(Vector & Pos);	// if this pos is in the location
	bool	CheckBoundary(Vector & Pos);
};