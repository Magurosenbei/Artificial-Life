#pragma once

class msgpacket
{
public:
	unsigned int OwnerID;
	unsigned int StateID;
	unsigned int AbbtributeAffected;
	msgpacket(unsigned int OID = 0 , unsigned int SID = 0, unsigned int AAf = 0 )
	{
		OwnerID = OID;
		StateID = SID;
		AbbtributeAffected = AAf;
	}
};

class MessageBoard
{
	//std::string Message;
	std::vector <std :: deque <msgpacket>> Toclear;
	typedef std :: deque <msgpacket> MessageBankList;
	MessageBankList mbl_;
	float PosX, PosY;

public:
	MessageBoard();			// Set
	~MessageBoard();
	
	msgpacket GetMsg(unsigned int ID);
	void addMessage(unsigned int OwnerID, unsigned int State, unsigned int AbbtributeAffected);
	void storeMsg(unsigned int FromOwnerID = 0, unsigned int CurrentStateID = 0, unsigned int AbbtributeBeingAffected = 0);
	void Render();
};

extern MessageBoard * g_pMessageBoard;