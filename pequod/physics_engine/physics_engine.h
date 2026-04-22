/**
 * physics_engine.h
 *
 * Physics system for game objects
 * The way it works is that the user has to define a "shape" to encapsulate the
 * object, along with size. For example if it's a capsule the shape has to be a
 * capsule shape with the dimensions for it.
 *
 * Then during every tick, the position is inferred from the object system and
 * the velocity is synchronized with it as well. So if the physics system
 * figures out that it's reached a wall, it will set the velocity to 0.
 *
 */
#ifndef PEQUOD_PHYSICS_ENGINE_H_
#define PEQUOD_PHYSICS_ENGINE_H_
#include <map>

#include "Jolt/Core/Core.h"
#include "Jolt/Physics/Body/Body.h"
#include "globals.h"
#include "pobject/manager.h"
#include "properties/collision_body.h"

namespace Pequod {

/**
 * @brief Function type that runs whenever bodies collide
 */
using CollisionCallbackLambda =
    std::function<void(kEntityId self, kEntityId other)>;

/**
 * @brief Provides easy-access to Physics using Jolt
 *
 * It is a basic wrapper around Jolt. It is designed to be simple and not add
 * features, rather rely completely on Jolt for most things like resource
 * management and physics simulations.
 *
 * Rather it provides a way to interface PObject's with Physics. Like allowing
 * on collision functions to be defined as lambdas that are called on collision
 * rather than complete classes that have to stay through the entire program
 * lifetime.
 */
class PhysicsEngine {
 public:
  PhysicsEngine();

  /**
   * @brief Initialize Jolt before computing
   */
  void Initialize();

  /**
   * @brief Calculate the physics for the environment such as collision
   * @param steps No. of steps to take in a computation (1 every 60hz)
   */
  void Compute(int steps);

  /**
   * @brief Register a body to the physics engine before adding properties
   */
  void RegisterBody(kEntityId id, CollisionBody body);

  /**
   * @brief Triggers when a collision first occurs
   * @param self The Entity ID of the registered PObject
   * @param callback The callback function to trigger
   */
  void SetOnEnterCollision(kEntityId self, CollisionCallbackLambda callback);

  /**
   * @brief Triggers when bodies are colliding
   * @param self The Entity ID of the registered PObject
   * @param callback The callback function to trigger
   */
  void SetOnDuringCollision(kEntityId self, CollisionCallbackLambda callback);

  /**
   * @brief Triggers when body stops colliding
   * @param self The Entity ID of the registered PObject
   * @param callback The callback function to trigger
   */
  void SetOnLeaveCollision(kEntityId self, CollisionCallbackLambda callback);

 private:
  std::map<kEntityId, CollisionCallbackLambda> on_collision_ = {};
  std::map<kEntityId, CollisionCallbackLambda> on_collision_enter_ = {};
  std::map<kEntityId, CollisionCallbackLambda> on_collision_leave_ = {};
  std::map<JPH::BodyID, kEntityId> jolt_bodies_ref_ = {};
};

}  // namespace Pequod

#endif
