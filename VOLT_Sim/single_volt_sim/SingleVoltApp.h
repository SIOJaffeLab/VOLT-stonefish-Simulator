#ifndef __Stonefish__SingleVoltSimApp__
#define __Stonefish__SingleVoltSimApp__

#include "SingleVoltSimulationManager.h"
#include <core/GraphicalSimulationApp.h>

class SingleVoltApp : public sf::GraphicalSimulationApp {
public:
  SingleVoltApp(std::string dataDirPath, sf::RenderSettings s,
                sf::HelperSettings h, SingleVoltSimulationManager *sim);

  void DoHUD();

private:
  SingleVoltSimulationManager *simulationManager;
};

#endif
