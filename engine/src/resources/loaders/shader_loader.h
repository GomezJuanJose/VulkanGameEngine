/**
 * @file shader_loader.h
 * @author Juan José (gs.juanjose.1999@gmail.com)
 * @brief A resource loader that handles shader config resources..
 * @version 1.0
 * @date 2025-05-27
 * 
 * @copyright Copyright (c) 2025
 */

 #pragma once

 #include "systems/resource_system.h"

 /**
  * @brief Creates and returns a shader resource loader.
  * 
  * @return The newly created resource loader.
  */
 resource_loader shader_resource_loader_create();