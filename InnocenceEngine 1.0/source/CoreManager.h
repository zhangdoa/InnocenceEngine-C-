#pragma once
#include "IEventManager.h"

#include "TimeManager.h"
#include "UIManager.h"
#include "RenderingManager.h"

class CoreManager : public IEventManager
{
public:
	CoreManager();
	~CoreManager();

private:
	void init() override;
	void update() override;
	void shutdown() override;
	
	std::vector<std::auto_ptr<IEventManager>> m_childManager;

	TimeManager m_timeManager;
	UIManager m_uiManager;
	RenderingManager m_renderingManager;

	const double m_frameTime = (1.0 / 60.0) * 1000.0;
	double m_unprocessedTime;
	
};

