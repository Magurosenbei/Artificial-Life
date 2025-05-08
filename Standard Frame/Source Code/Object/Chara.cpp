#include "stdafx.h"
#include "Chara.h"
#include "house.h"


void FuzzyLog :: ResetValues(int LwSt, int LwEnd, int HSt, int HEnd)
{
	LowStart = LwSt;
	LowEnd = LwEnd;
	HighStart = HSt;
	HighEnd = HEnd;
}

void FuzzyLog :: GenerateRandomly()
{
	LowEnd = LowStart = rand() % 20;
	LowEnd += rand() % 20 + 10 + LowStart;
	HighEnd = HighStart = int(LowEnd + rand() % 20 * 0.85f);
	HighEnd += rand() % 20 + 10 + HighStart;
}

string FuzzyLog :: LOW_Term()
{
	if(CurrentValue >= LowEnd) return "NOT";
	if(CurrentValue <= LowStart) return "EXTREME";

	float Av = float(LowEnd - CurrentValue)  / (LowEnd - LowStart);
	if(Av >= 0.75)	return "VERY";
	else return "QUITE";
}

string FuzzyLog :: HIGH_Term()
{
	if(CurrentValue <= HighStart) return "NOT";
	if(CurrentValue >= HighEnd) return "EXTREME";

	float Av = float(CurrentValue - HighStart) / (HighEnd - HighStart);
	if(Av >= 0.75)	return "VERY";
	else return "QUITE";
}

float FuzzyLog :: LOW_Value()
{
	if(CurrentValue >= LowEnd) return 0;
	if(CurrentValue <= LowStart) return 1;

	return float(LowEnd - CurrentValue) / (LowEnd - LowStart);
}

float FuzzyLog :: HIGH_Value()
{
	if(CurrentValue <= HighStart) return 0;
	if(CurrentValue >= HighEnd) return 1;

	return float(CurrentValue - HighStart) / (HighEnd - HighStart);
}

float FuzzyLog :: ConvertTerm(std::string text)
{
	if(text == "QUITE")
		return 0.5f;
	else if(text == "VERY")
		return 0.75f;
	else return 0;
}

void FuzzyLog :: Register(lua_State * State)
{
	luabind::module(State) 
	[		
		luabind::class_<FuzzyLog>("FuzzyLog")
		.def_readwrite("Value", &FuzzyLog :: CurrentValue)
		.def("Generate", &FuzzyLog :: GenerateRandomly)
		.def("GetTerm_Low", &FuzzyLog :: LOW_Term)
		.def("GetTerm_High", &FuzzyLog :: HIGH_Term)
		.def("GetDegreeLow", &FuzzyLog :: LOW_Value)
		.def("GetDegreeHigh", &FuzzyLog :: HIGH_Value)
		.def("TermToValue", &FuzzyLog :: ConvertTerm)
	];
}

Character :: Character(const std :: string FilePath, unsigned int ID):CharaID(ID)
{
	m_nCounter = 60;
	m_nframe = 0;
	m_pHouse = NULL;
	Current_State = -1;
	
	State = lua_open();
	luabind::open(State);
	luaL_openlibs(State);

	Vector :: Register(State);
	TTimer :: Register(State);
	FuzzyLog :: Register(State);

	luabind::module(State) 
	[		
		luabind::class_<msgpacket>("Mail")
		.def_readwrite("From", &msgpacket::OwnerID)
		.def_readwrite("Doing", &msgpacket::StateID)
		.def_readwrite("Affecting", &msgpacket::AbbtributeAffected),

		luabind::class_<MessageBoard>("MsgBoard")
		.def("READMSG", &MessageBoard::GetMsg)
		.def("PostMSG", &MessageBoard::storeMsg),

		luabind::class_<Character>("Character")
		.def_readwrite("Position", &Character :: Position)
		.def_readwrite("State", &Character :: Current_State)
		.def_readwrite("Emotion", &Character :: m_nframe)
		.def_readwrite("IC_NUMBER", &Character :: CharaID)
		.def_readwrite("Hunger", &Character :: FL_Hunger)
		.def_readwrite("Hygiene", &Character :: FL_Hygiene)
		.def_readwrite("Energy", &Character :: FL_Energy)
		.def_readwrite("Boredom", &Character :: FL_Boredom)
		.def("SetEmo", &Character :: SetEmo)
		.def("GoTo", &Character :: GoTo)
		.def("At" , &Character :: At)
		.def("GetPosition" , &Character :: GetPosition),

		luabind::class_<House>("House")
		.def("CheckInLocation", &House :: CheckInLocation)
		.def("CheckBoundary", &House :: CheckBoundary)
		.def("CheckInHouse", &House :: CheckInHouse)
		.def("GetLocation", &House :: GetLocation)
		.def("IsLocationFull", &House :: IsLocationFull)
		.def("GetPosition", &House :: GetPosition),
	
		luabind::def("CheckProbability", &TTimer :: CheckProbability)
	];

	luabind::globals(State)["Brain"] = this;
	luabind::globals(State)["MailBox"] = g_pMessageBoard;

	luaL_dofile(State, FilePath.c_str());

	lua_getglobal(State, "Texture_Path");
	ASSERT( lua_isstring(State, -1) , "No Texture Path");
	m_nTexID = g_Engine->Use_TexManager()->AddTexture(lua_tostring(State, -1));
	m_nWidth = g_Engine->Use_TexManager()->GetTexData(m_nTexID)->width;
	m_nHeight = g_Engine->Use_TexManager()->GetTexData(m_nTexID)->height;

	lua_getglobal(State, "EmoIcon_Path");
	ASSERT( lua_isstring(State, -1) , "No Texture Path");
	m_nEmoIcon = g_Engine->Use_TexManager()->AddTexture(lua_tostring(State, -1));

	lua_getglobal(State, "Stats");	// Starting Stats
	ASSERT( lua_isfunction(State, -1) , "No Stats Function");
	lua_pcall(State, 0, 0, 0);

	lua_getglobal(State, "Process");	// Things that happen every 10 in game minutes
	ASSERT( lua_isfunction(State, -1) , "No Process Function");

	lua_getglobal(State, "Behave");		// Behaviour
	ASSERT( lua_isfunction(State, -1) , "No Behave Function");

	lua_getglobal(State, "D_CheckForState");		// Behaviour
	ASSERT( lua_isfunction(State, -1) , "No D_CheckForState Function");

	lua_getglobal(State, "DailySetting");		// Behaviour
	ASSERT( lua_isfunction(State, -1) , "No DailySetting Function");
}

Character :: ~Character()
{
	Goal = Vector(0,0,1);
	m_pHouse = NULL;
	lua_close(State);
}

void Character :: SetEmo(int index)
{
	m_nframe = index;
	m_nCounter = 0;
}

void Character :: GiveHouse(House * Gift)
{
	m_pHouse = Gift;
	luabind::globals(State)["Home"] = m_pHouse;
}

void Character :: MoveToGoal()
{
	if(m_nCounter < 60) m_nCounter++;
	if(Current_State != -1)	return;			// don't compute
	Vector Mov(Goal - Position);
	if(Mov.Length() < 2.0f)	Goal.z = 1;						// say it reached
	else					Position += Mov.Normalize();
}

void Character :: GoTo(Vector Destination)
{
	if(Current_State != -1 || Destination == Vector(0,0,-5.0f))	return;
	if(Goal.z == 1)	
		Goal = Destination;
}

Vector Character :: GetPosition()
{
	return Position;
}

bool Character :: At(std :: string Place)
{
	return !AtWhere.compare(Place);
}

void Character :: Render()
{
	glPushMatrix();
	glTranslatef(Position.x, Position.y, Position.z);
	g_Engine->Use_TexManager()->DrawXY(m_nTexID);
	//if(m_nCounter < 60)	
	{	
		glTranslatef(16,28,0);
		g_Engine->Use_TexManager()->DrawXY(m_nEmoIcon, m_nframe, 32);
	}
	glPopMatrix();
}

void Character :: Behave()
{
	lua_getglobal(State, "Behave");
	lua_pcall(State, 0, 0, 0);
}

void Character :: CheckForState()
{
	lua_getglobal(State, "D_CheckForState");
	lua_pcall(State, 0, 0, 0);
}

void Character :: StdClockProcess()
{
	lua_getglobal(State, "Process");
	lua_pcall(State, 0, 0, 0);
	//WriteCommandLine("State : %i Emotion : %i" , Current_State, m_nframe);
}

void Character :: DailySettings()
{
	lua_getglobal(State, "DailySetting");
	lua_pcall(State, 0, 0, 0);
}