#include "stdafx.h"
#include "TTimer.h"

TTimer TheClock;

TTimer :: TTimer()
{
	TimerProcess = NULL;
	m_sCurrently.m_nHour = m_sCurrently.m_nMinute = 8;
	m_bNightTime = m_bStop = false;
	m_nProbab = 0;
}

const Clock TTimer :: GetTime()
{
	return TheClock.m_sCurrently;
}

bool TTimer :: IsNightTime()
{
	return TheClock.m_bNightTime;
}

void TTimer :: CloseTimer()
{
	m_bStop = true;
}

bool TTimer :: CheckProbability(int Percentage)
{
	if(TheClock.m_nProbab < Percentage)	return true;
	return false;
}

void MoveClock(void *pParams)
{
	while(!TheClock.m_bStop)
	{
		if(Key(VK_ESCAPE)) break;
		(TheClock.m_sCurrently.m_nMinute < 60)? TheClock.m_sCurrently.m_nMinute++ : TheClock.m_sCurrently.m_nMinute = 0;
		if(TheClock.m_sCurrently.m_nMinute == 0)
			if(TheClock.m_sCurrently.m_nHour < 24)
				TheClock.m_sCurrently.m_nHour++;
			else
			{
				TheClock.m_sCurrently.m_nHour = 1;
				#ifdef _2D_MODE
				for(unsigned int index = 0; index < g_Engine->Person.size(); index++)
					g_Engine->Person[index]->DailySettings();
				#endif
			}
		(TheClock.m_sCurrently.m_nHour < 6 || TheClock.m_sCurrently.m_nHour > 18)?	TheClock.m_bNightTime = true : TheClock.m_bNightTime = false;
		if(TheClock.m_sCurrently.m_nMinute % 30 == 0) 
		{
			//WriteCommandLine("Prob Factor: %i", TheClock.m_nProbab);
			TheClock.m_nProbab = rand() % 99 + 1;
		}
		if(TheClock.m_sCurrently.m_nMinute % 20 == 0)
		{
			EnterCriticalSection(&_critSection);
			#ifdef _2D_MODE
			for(unsigned int index = 0; index < g_Engine->Person.size(); index++)
				g_Engine->Person[index]->StdClockProcess();
			#endif
			LeaveCriticalSection(&_critSection);
			srand(GetTickCount());
		}
		Sleep(100);
	}
	TheClock.TimerProcess = NULL;
	_endthread();	// Safely Close
}


void TTimer :: Register(lua_State * State)
{
	luabind::module(State) 
	[
		luabind::class_<Clock>("Clock")
		.def_readwrite("Hour", &Clock::m_nHour)
		.def_readwrite("Minute", &Clock::m_nMinute)
	];

	luabind::module(State) 
	[
		luabind::def("GetTime", &TTimer::GetTime),
		luabind::def("IsNightTime", &TTimer::IsNightTime)
	];
}