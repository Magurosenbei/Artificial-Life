#include "stdafx.h"
#include "MessageBoard.h"

MessageBoard * g_pMessageBoard;

MessageBoard::MessageBoard()
{
	PosX = 120;
	PosY = 250;
}

MessageBoard::~MessageBoard()
{
}

void MessageBoard::addMessage(unsigned int OwnerID, unsigned int State, unsigned int AbbtributeAffected)
{
	if(mbl_.size() > 5)
		mbl_.pop_front();
	mbl_.push_back(msgpacket(OwnerID, State, AbbtributeAffected));
}

msgpacket MessageBoard :: GetMsg(unsigned int ID)
{
	/*if(ID > Toclear.size()) return msgpacket();
	msgpacket temp;
	temp = Toclear[ID].front();
	Toclear[ID].pop_front();
	mbl_.pop_front();*/
	ID;
	if(mbl_.size() < 1)
		return msgpacket();
	msgpacket temp = mbl_[0];
	return temp;
}

void MessageBoard :: storeMsg(unsigned int FromOwnerID, unsigned int CurrentStateID, unsigned int AbbtributeBeingAffected)
{
	addMessage(FromOwnerID, CurrentStateID,AbbtributeBeingAffected );
	//mbl_.push_back(new msgpacket(FromOwnerID, CurrentStateID,AbbtributeBeingAffected ));
	/*if(FromOwnerID == 1)
	{
		//mbl_.push_back(new msgpacket(FromOwnerID, CurrentStateID,AbbtributeBeingAffected ));
		addMessage(FromOwnerID, CurrentStateID,AbbtributeBeingAffected);
	//	Toclear[1].push_back(msgpacket(FromOwnerID, CurrentStateID,AbbtributeBeingAffected ));
	}
	else if(FromOwnerID == 2)
	{
		//mbl_.push_back(new msgpacket(FromOwnerID, CurrentStateID,AbbtributeBeingAffected ));
		addMessage(FromOwnerID, CurrentStateID,AbbtributeBeingAffected);
//		Toclear[0].push_back(msgpacket(FromOwnerID, CurrentStateID, AbbtributeBeingAffected));
	}*/
}

void MessageBoard ::Render()
{
	glPushMatrix();
	static char szBuffer[512];
	float tempPosX = PosX, tempPosY = PosY;
	//for(MessageBankList::iterator mb = mbl_.begin(); mb != mbl_.end(); mb++)
	for(int a = 0; a != int(mbl_.size()); a++)
	{
		switch(mbl_[a].StateID)
		{
		case 0:	_snprintf(szBuffer, 511, "Player %u is Eating", mbl_[a].OwnerID); break;
		case 1:	_snprintf(szBuffer, 511, "Player %u is Bathing", mbl_[a].OwnerID); break;
		case 2:	_snprintf(szBuffer, 511, "Player %u is Playing", mbl_[a].OwnerID); break;
		case 3:	_snprintf(szBuffer, 511, "Player %u is Studying", mbl_[a].OwnerID); break;
		case 4:	_snprintf(szBuffer, 511, "Player %u is NotDoing anything", mbl_[a].OwnerID); break;
		case 5:	_snprintf(szBuffer, 511, "Player %u is Sleeping", mbl_[a].OwnerID); break;
		case 6:	_snprintf(szBuffer, 511, "Player %u is Resting", mbl_[a].OwnerID); break;
		}
		g_Font->Print(szBuffer, tempPosX, tempPosY);
		tempPosY -= 15;
	}
	glPopMatrix();
}