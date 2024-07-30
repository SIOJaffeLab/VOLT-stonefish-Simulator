#include "SingleVoltSimulationManager.h"
#include <Stonefish/core/SimulationManager.h>
#include <Stonefish/entities/solids/Sphere.h>
#include <Stonefish/entities/statics/Plane.h>
#include <actuators/Light.h>
#include <actuators/Servo.h>
#include <actuators/Thruster.h>
#include <actuators/VariableBuoyancy.h>
#include <comms/AcousticModem.h>
#include <core/FeatherstoneRobot.h>
#include <core/NED.h>
#include <core/ScenarioParser.h>
#include <entities/AnimatedEntity.h>
#include <entities/FeatherstoneEntity.h>
#include <entities/forcefields/Jet.h>
#include <entities/forcefields/Pipe.h>
#include <entities/forcefields/Trigger.h>
#include <entities/forcefields/Uniform.h>
#include <entities/solids/Box.h>
#include <entities/solids/Compound.h>
#include <entities/solids/Cylinder.h>
#include <entities/solids/Polyhedron.h>
#include <entities/solids/Sphere.h>
#include <entities/solids/Torus.h>
#include <entities/solids/Wing.h>
#include <entities/statics/Obstacle.h>
#include <entities/statics/Plane.h>
#include <entities/statics/Terrain.h>
#include <graphics/OpenGLPointLight.h>
#include <graphics/OpenGLSpotLight.h>
#include <graphics/OpenGLTrackball.h>
#include <sensors/Contact.h>
#include <sensors/Sample.h>
#include <sensors/scalar/Accelerometer.h>
#include <sensors/scalar/Compass.h>
#include <sensors/scalar/DVL.h>
#include <sensors/scalar/GPS.h>
#include <sensors/scalar/IMU.h>
#include <sensors/scalar/Multibeam.h>
#include <sensors/scalar/Odometry.h>
#include <sensors/scalar/Pressure.h>
#include <sensors/scalar/Profiler.h>
#include <sensors/scalar/RotaryEncoder.h>
#include <sensors/vision/ColorCamera.h>
#include <sensors/vision/DepthCamera.h>
#include <sensors/vision/FLS.h>
#include <sensors/vision/MSIS.h>
#include <sensors/vision/Multibeam2.h>
#include <sensors/vision/SSS.h>
#include <utils/SystemUtil.hpp>
#include <utils/UnitSystem.h>

SingleVoltSimulationManager::SingleVoltSimulationManager(
    sf::Scalar stepsPerSecond)
    : SimulationManager(stepsPerSecond) {}

void SingleVoltSimulationManager::BuildScenario() {
  // Physical materials
  CreateMaterial("Aluminium", 700.0, 0.8);
  CreateMaterial("Steel", 7810.0, 0.9);
  SetMaterialsInteraction("Aluminium", "Aluminium", 0.7, 0.5);
  SetMaterialsInteraction("Steel", "Steel", 0.4, 0.2);
  CreateMaterial("Rock", sf::UnitSystem::Density(sf::CGS, sf::MKS, 3.0), 0.6);
  SetMaterialsInteraction("Aluminium", "Steel", 0.6, 0.4);
  SetMaterialsInteraction("Rock", "Aluminium", 0.6, 0.4);

  // Graphical materials (looks)
  CreateLook("gray", sf::Color::Gray(0.5f), 0.3f, 0.2f);
  CreateLook("red", sf::Color::RGB(1.f, 0.f, 0.f), 0.1f, 0.f);
  CreateLook("seabed", sf::Color::RGB(0.7f, 0.7f, 0.5f), 0.9f, 0.f, 0.f, "",
             sf::GetDataPath() + "sand_normal.png");

  // Create environment
  //  sf::Plane *plane = new sf::Plane("Ground", 10000.0, "Steel", "gray");
  // AddStaticEntity(plane, sf::I4());

  phy.mode = sf::BodyPhysicsMode::FLOATING;
  phy.collisions = true;
  phy.buoyancy = true;

  // Create object
  sf::Sphere *sph = new sf::Sphere("Sphere", phy, sf::Scalar(0.1), sf::I4(),
                                   "Aluminium", "seabed");
  AddSolidEntity(sph, sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, -9.0)));

  EnableOcean(0.0);
  getOcean()->setWaterType(0.0);
  getOcean()->AddVelocityField(new sf::Uniform(sf::Vector3(1.0, 1.0, 0.0)));
  getOcean()->EnableCurrents();
  getAtmosphere()->SetupSunPosition(0.0, 60.0);

  sf::Terrain *seabed =
      new sf::Terrain("Seabed", sf::GetDataPath() + "terrain.png", 1.0, 1.0,
                      5.0, "Rock", "seabed", 5.f);
  AddStaticEntity(seabed, sf::Transform(sf::IQ(), sf::Vector3(0, 0, 15.0)));

  // create VOLT
  sf::Polyhedron *volt_body =
      new sf::Polyhedron("VoltBody", phy, sf::GetDataPath() + "volt.obj",
                         sf::Scalar(1), sf::I4(), "Aluminium", "Yellow");

  sf::Compound *vehicle = new sf::Compound("Vehicle", phy, volt_body, sf::I4());

  sf::Robot *volt = new sf::FeatherstoneRobot("");
  AddRobot(volt,
           sf::Transform(sf::Quaternion(0, 0, 0), sf::Vector3(0.0, 0.0, 2.0)));
}

void SingleVoltSimulationManager::AddBall() {
  sf::Sphere *sph = new sf::Sphere("Sphere", phy, sf::Scalar(0.1), sf::I4(),
                                   "Aluminium", "red");
  AddSolidEntity(sph, sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, -1.0)));
}
