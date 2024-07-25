#include "SingleVoltApp.h"
#include "SingleVoltSimulationManager.h"
#include <core/Console.h>
#include <core/Robot.h>
#include <graphics/IMGUI.h>
#include <unistd.h>

SingleVoltApp::SingleVoltApp(std::string dataDirPath, sf::RenderSettings s,
                             sf::HelperSettings h,
                             SingleVoltSimulationManager *sim)
    : GraphicalSimulationApp("Single Volt Sim", dataDirPath, s, h, sim),
      simulationManager(sim) {}

void SingleVoltApp::DoHUD() {
  GraphicalSimulationApp::DoHUD(); // Keep standard GUI

  /*  sf::Uid button;
    button.owner = 1; // e.g. id of a panel
    button.item = 0;  // e.q. id of an option on a list
    if (getGUI()->DoButton(button, 200, 10, 200, 50, "Press me")) {
      if (simulationManager) {
        printf("Adding Ball\n");
        simulationManager->AddBall();
      } else {
        printf("Can't add Ball\n");
      }
    }
  */
}
