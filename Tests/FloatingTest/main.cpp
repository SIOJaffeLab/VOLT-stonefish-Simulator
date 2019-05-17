/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  main.cpp
//  FloatingTest
//
//  Created by Patryk Cieslak on 02/05/2019.
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#include <core/GraphicalSimulationApp.h>
#include "FloatingTestManager.h"

int main(int argc, const char * argv[])
{
    sf::RenderSettings s;
    s.windowW = 1200;
    s.windowH = 900;
    s.msaa = true;
    s.shadows = sf::RenderQuality::QUALITY_HIGH;
    s.ao = sf::RenderQuality::QUALITY_HIGH;
    s.atmosphere = sf::RenderQuality::QUALITY_HIGH;
    s.ocean = sf::RenderQuality::QUALITY_HIGH;
    
    sf::HelperSettings h;
    h.showFluidDynamics = false;
    h.showCoordSys = false;
    h.showBulletDebugInfo = false;
    h.showSensors = false;
    h.showActuators = false;
    h.showForces = false;
    
    FloatingTestManager* simulationManager = new FloatingTestManager(200.0);
    sf::GraphicalSimulationApp app("FloatingTest", std::string(DATA_DIR_PATH), s, h, simulationManager);
    app.Run(false);
    
    return 0;
}