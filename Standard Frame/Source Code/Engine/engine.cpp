#include "stdafx.h"
#include "engine.h"

CEngine * g_Engine = new CEngine;

CEngine :: CEngine()
{
	m_pTexManager = new Texture;
	m_pCamera = new Camera;
}

CEngine :: ~CEngine()
{
	TaskManager->terminate();
	delete TaskManager;
	
	m_pTexManager->ClearAllTextures();
	delete m_pTexManager;

	delete m_pCamera;
	delete g_Font;
}

void CEngine :: BootUp()
{
	m_pCamera->Position_Camera(	0.0f, 0.0f, 10.0f,
								0.0f, 0.0f, 0.0f,
								0.0f, 1.0f, 0.0f);

	g_pMessageBoard = new MessageBoard;
	if(MessageBoxW(NULL, L"Show StandAlone FuzzyProb?", L"Mode", MB_YESNO) == IDNO)
	{
		Person.push_back(new Character("Data/AI/Life Simulation/Brain.file", 1));
		Houses = new House("Data/AI/Life Simulation/House.file", Person[0]);
		Person[0]->GiveHouse(Houses);
		Person.push_back(new Character("Data/AI/Life Simulation/Brain2.file", 2));
		Person[1]->GiveHouse(Houses);
	}
	else
	{
		Person.push_back(new Character("Data/AI/Life Simulation/FuzzyProbBrain.file", 1));
		Houses = new House("Data/AI/Life Simulation/House.file", Person[0]);
		Person[0]->GiveHouse(Houses);
	}
	g_Font = new Font("Data/Textures/Hud/font.png");
}

void CEngine :: ContextPreProcess()
{
	m_pTexManager->Build();
	TheClock.TimerProcess = (HANDLE)_beginthread(&MoveClock, 0, NULL);
	TaskManager = new tbb :: task_scheduler_init;
}

void CEngine :: BootDown()
{
	for(unsigned int index = 0; index < Person.size(); index++)
		delete Person[index];
	delete Houses;
	Person.clear();
	m_pTexManager->ClearAllTextures();

	delete g_pMessageBoard;
	while(TheClock.TimerProcess != NULL)
		TheClock.CloseTimer();
	CloseHandle(TheClock.TimerProcess);
}	

void CEngine :: RenderObjects()
{
	glColor3ub(255, 255, 255);		// Reset Colorback

	Houses->Render();
	for(unsigned int index = 0; index < Person.size(); index++)
		Person[index]->Render();

	static char szBuffer[512];
	if(TheClock.IsNightTime())
		_snprintf(szBuffer, 511, "Prob > %i Time : %i H %i Min (Night)", TheClock.GetProb(), TheClock.GetTime().m_nHour, TheClock.GetTime().m_nMinute);
	else
		_snprintf(szBuffer, 511, "Prob > %i Time : %i H %i Min (Day)", TheClock.GetProb(), TheClock.GetTime().m_nHour, TheClock.GetTime().m_nMinute);
	g_Font->Print(szBuffer, -400, -300);

	_snprintf(szBuffer, 511, "P1 -> State : %i At:%s", Person[0]->Current_State, &Person[0]->AtWhere[0]);
	g_Font->Print(szBuffer, -400, -220);
	_snprintf(szBuffer, 511, "P1 -> Energy: %i Hunger: %i Hygiene: %i Boredom: %i", Person[0]->FL_Energy.CurrentValue, Person[0]->FL_Hunger.CurrentValue, Person[0]->FL_Hygiene.CurrentValue, Person[0]->FL_Boredom.CurrentValue);
	g_Font->Print(szBuffer, -400, -240);

	_snprintf(szBuffer, 511, "Press F1 ~ F 8 for P1 <1> to <8> for P2");
	g_Font->Print(szBuffer, -400, 280);

	if(Person.size() < 2)
	{
		_snprintf(szBuffer, 511, "Terms :");
		g_Font->Print(szBuffer, -400, 260);
		_snprintf(szBuffer, 511, "Energy : %s Low l %s High Hunger : %s Low l %s High", &Person[0]->FL_Energy.LOW_Term()[0], &Person[0]->FL_Energy.HIGH_Term()[0], &Person[0]->FL_Hunger.LOW_Term()[0], &Person[0]->FL_Hunger.HIGH_Term()[0]);
		g_Font->Print(szBuffer, -400, 240);
		_snprintf(szBuffer, 511, "Hygiene: %s Low l %s High Boredom: %s Low l %s High", &Person[0]->FL_Hygiene.LOW_Term()[0],  &Person[0]->FL_Hygiene.HIGH_Term()[0], &Person[0]->FL_Boredom.LOW_Term()[0], &Person[0]->FL_Boredom.HIGH_Term()[0]);
		g_Font->Print(szBuffer, -400, 220);
	}
	else
	{
		_snprintf(szBuffer, 511, "P2 -> State : %i At:%s", Person[1]->Current_State, &Person[1]->AtWhere[0]);
		g_Font->Print(szBuffer, -400, -260);
		_snprintf(szBuffer, 511, "P2 -> Energy: %i Hunger: %i Hygiene: %i Boredom: %i", Person[1]->FL_Energy.CurrentValue, Person[1]->FL_Hunger.CurrentValue, Person[1]->FL_Hygiene.CurrentValue, Person[1]->FL_Boredom.CurrentValue);
		g_Font->Print(szBuffer, -400, -280);
	}

	g_pMessageBoard->Render();
}

void CEngine :: UpdateObjects()
{
	/* Objects are updated this portion */
	EnterCriticalSection(&_critSection);
	for(unsigned int index = 0; index < Person.size(); index++)
	{
		Person[index]->AtWhere = Houses->AtWhichArea(Person[index]);
	}
	for(unsigned int index = 0; index < Person.size(); index++)
	{
		Person[index]->MoveToGoal();
		Person[index]->Behave();
		Person[index]->CheckForState();
	}
	Houses->ResetPlaceCounter();
	LeaveCriticalSection(&_critSection);
}

void CEngine :: ProcessInput()
{
	/* Inputs, mouse etc comes under here */
	if(Key(VK_F1))	Person[0]->FL_Energy.CurrentValue++;
	if(Key(VK_F2))	Person[0]->FL_Energy.CurrentValue--;
	if(Key(VK_F3))	Person[0]->FL_Hunger.CurrentValue++;
	if(Key(VK_F4))	Person[0]->FL_Hunger.CurrentValue--;
	if(Key(VK_F5))	Person[0]->FL_Hygiene.CurrentValue++;
	if(Key(VK_F6))	Person[0]->FL_Hygiene.CurrentValue--;
	if(Key(VK_F7))	Person[0]->FL_Boredom.CurrentValue++;
	if(Key(VK_F8))	Person[0]->FL_Boredom.CurrentValue--;

	if(Person.size() < 2) return;

	if(Key('1'))	Person[1]->FL_Energy.CurrentValue++;
	if(Key('2'))	Person[1]->FL_Energy.CurrentValue--;
	if(Key('3'))	Person[1]->FL_Hunger.CurrentValue++;
	if(Key('4'))	Person[1]->FL_Hunger.CurrentValue--;
	if(Key('5'))	Person[1]->FL_Hygiene.CurrentValue++;
	if(Key('6'))	Person[1]->FL_Hygiene.CurrentValue--;
	if(Key('7'))	Person[1]->FL_Boredom.CurrentValue++;
	if(Key('8'))	Person[1]->FL_Boredom.CurrentValue--;
}

Texture * CEngine :: Use_TexManager() const
{
	return m_pTexManager;
}

Camera	* CEngine :: Use_Camera() const
{
	return m_pCamera;
}