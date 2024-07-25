#include "SingleVoltApp.h"
#include "SingleVoltSimulationManager.h"
#include <Stonefish/core/GraphicalSimulationApp.h>

int main(int argc, char **argv) {
  // Using default settings
  sf::RenderSettings s;
  sf::HelperSettings h;

  SingleVoltSimulationManager manager(500.0);
  SingleVoltApp app(std::string(DATA_DIR_PATH), s, h, &manager);
  app.Run();

  return 0;
}
