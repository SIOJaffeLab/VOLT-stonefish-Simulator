#ifndef SINGLE_VOLT_SIMULATION_MANAGER_H
#define SINGLE_VOLT_SIMULATION_MANAGER_H

#include <Stonefish/core/SimulationManager.h>

class SingleVoltSimulationManager : public sf::SimulationManager {
public:
  sf::BodyPhysicsSettings phy;
  SingleVoltSimulationManager(sf::Scalar stepsPerSecond);
  void BuildScenario();
  void AddBall();
};

#endif
