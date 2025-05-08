#pragma once

#define CONST_VEL Vector(0,1)


class FuzzyLog
{
protected:
	int LowStart, LowEnd;
	int HighStart, HighEnd;
public:
	int CurrentValue;
	FuzzyLog(int Start = 0): LowStart(0), LowEnd(50), HighStart(50), HighEnd(100), CurrentValue(Start){}
	void ResetValues(int LwSt, int LwEnd, int HSt, int HEnd);
	void GenerateRandomly();
	std :: string LOW_Term();
	std :: string HIGH_Term(); 

	float LOW_Value();
	float HIGH_Value();
	float ConvertTerm(std::string text);
	static void Register(lua_State * State);
};


class Character
{
	friend class CEngine;
	int m_nWidth, m_nHeight;
	unsigned int m_nTexID;
	
	lua_State * State;

	unsigned int CharaID;
protected:
	Vector Position;
	Vector Goal;
	
	House * m_pHouse;			// Set The house Address

	//	Stats
	int Hunger, Hygiene, Energy, Boredom;
	FuzzyLog FL_Hunger, FL_Hygiene, FL_Energy, FL_Boredom;

	int m_nCounter;				// 60 frame counter
	int m_nEmoIcon;
	int Current_State;
	unsigned int m_nframe;
	std :: string AtWhere;
public:
	Character(const std :: string FilePath, unsigned int ID = 0);
	~Character();
	
	void	SetEmo(int index);
	void	GiveHouse(House * Gift);
	void	MoveToGoal();
	void	GoTo(Vector Destination);
	Vector	GetPosition();
	bool	At(std :: string Place);
	
	void Render();				// Draw Character
	void Behave();				// The Brain
	void CheckForState();		// Checks Location and change state accord.
	void StdClockProcess();		// Calls every in-game 10 minutes
	void DailySettings();
};