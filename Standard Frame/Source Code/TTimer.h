#pragma once

class Clock
{
public:
unsigned int m_nHour, m_nMinute;
};

class TTimer
{
	friend void MoveClock(void * pParams);
protected:
	Clock	m_sCurrently;
	bool	m_bNightTime;
	bool	m_bStop;
	int		m_nProbab;
public:
	HANDLE TimerProcess;

	TTimer();
	
	static const Clock	GetTime();
	static bool	IsNightTime();
	static bool	CheckProbability(int Percentage);
	int		GetProb(){return m_nProbab;}
	void	CloseTimer();
	

	static void Register(lua_State * State);
};

void	MoveClock(void * pParams);

extern TTimer TheClock;