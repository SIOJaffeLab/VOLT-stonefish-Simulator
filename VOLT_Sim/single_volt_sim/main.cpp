#include "SingleVoltApp.h"
#include "SingleVoltSimulationManager.h"
#include <Stonefish/core/GraphicalSimulationApp.h>

int main(int argc, char **argv) {
  // Using default settings
  sf::RenderSettings s;
  sf::HelperSettings h;
  s.aa = sf::RenderQuality::HIGH;
  s.shadows = sf::RenderQuality::HIGH;
  s.ao = sf::RenderQuality::HIGH;
  s.atmosphere = sf::RenderQuality::MEDIUM;
  s.ocean = sf::RenderQuality::HIGH;
  s.ssr = sf::RenderQuality::HIGH;

  h.showFluidDynamics = false;
  h.showCoordSys = false;
  h.showBulletDebugInfo = false;
  h.showSensors = false;
  h.showActuators = false;
  h.showForces = false;

  SingleVoltSimulationManager manager(500.0);
  manager.setRealtimeFactor(1.0);
  SingleVoltApp app(std::string(DATA_DIR_PATH), s, h, &manager);
  printf("Data Dir %s\n", DATA_DIR_PATH);
  app.Run();

  return 0;
}
