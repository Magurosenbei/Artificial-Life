#pragma once

#include "./Texture/texture.h"
#include "./Camera/camera.h"
#include "./Interface/font.h"

#include "./Object/Messageboard.h"
#include "./Object/location.h"
#include "./Object/House.h"
#include "./Object/Chara.h"


class CEngine
{
	tbb ::	task_scheduler_init * TaskManager;	// typical tbb start up, requires self context
	tbb	::	affinity_partitioner partition;		// partition accrod to cache
	Camera		* m_pCamera; 
	Texture		* m_pTexManager;

protected:
	friend void MoveClock(void * pParams);
	tbb :: concurrent_vector <Character *>	Person;		// One house has 1 Chara
	House *		Houses;		

public:
	CEngine();
	~CEngine();
	
	// Memeber Bootups
	void BootUp();					//	Starts up
	void ContextPreProcess();		//	Just after context is created
	void BootDown();				//	Shutdown
	
	// Member Functions	
	void RenderObjects();			// Draw
	void UpdateObjects();			// Move etc
	void ProcessInput();			// Input funcitons


	// Memeber Accessors
	Texture *	Use_TexManager() const;	// See Texture.h for functions
	Camera	*	Use_Camera() const;
};

extern CEngine * g_Engine;					// Call this anywhere to use functions