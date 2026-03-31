/**
* ecs.hh
* 
* This is the ECS for an entire "scene" 
* It is a *singleton* class and hence only created and used once (again per scene)
* It handles object properties like meshes, collisions and gravity
* It does NOT handle engine properties like ticks or key events
**/

#ifndef PEQUOD_ECS_IMPL_HH_
#define PEQUOD_ECS_IMPL_HH_

#include <iostream>
#include <memory>
#include <typeindex>
#include <sokol/sokol_gfx.h>

#include "gameobjects/camera.hh"
#include <unordered_map>
#include <vector>

#include <ecs/entity.hh> 
#include <ecs/mesh.hh> 
#include <ecs/position.hh> 

#include <Jolt/Jolt.h>
#include <Jolt/Core/Memory.h>
#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Collision/CollideShape.h"
#include "Jolt/Physics/Body/MotionType.h"

#include "Jolt/Core/Factory.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h"
#include "Jolt/Physics/Collision/ContactListener.h"
#include "Jolt/Physics/Collision/ObjectLayer.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/RegisterTypes.h"

#include "../pobject/properties/rigidbody.hh"

#include <debugger/debugger.hh>

#include "physics_engine/physics_engine.h"


namespace Pequod {


class ECS {
public:
    ECS();
    ~ECS();
    entity_id current_id;


    void initializeJolt(); // run this once
    entity_id createEntity(); // use to create new entities that are part of the engine

    //void addPosition(entity_id, std::shared_ptr<Position>);

    //std::shared_ptr<Mesh> getMesh(entity_id);
    //std::shared_ptr<Position> getPosition(entity_id);

    void setVelocity(entity_id, glm::vec3);
    glm::vec3 GetVelocity(entity_id);
    void SetPosition(entity_id, glm::vec3);
    void SetRestitution(entity_id, float);
    void SetFriction(entity_id, float);

	template <class TProperty>
	void AddProperty(entity_id, std::shared_ptr<TProperty>); // Add property

	template <class TProperty>
	std::shared_ptr<TProperty> GetProperty(entity_id); // Add property

    void SetMotionType(entity_id, JPH::EMotionType);

    void Disable(entity_id);

    JPH::BodyID* getJoltId(entity_id);

    void setupRender(sg_bindings& bind);
    void render(Camera& cam, float delta_t);

    void processOnTick(float delta_t);
    void processOnFrame(float delta_t);

protected:
	template <class TProperty>
	static std::unordered_map<std::type_index, std::unordered_map<entity_id, std::shared_ptr<TProperty>>> properties;

    std::vector<vertex_t> vertices;
    std::vector<uint16_t> indices;



private:
    void addMesh(entity_id, std::shared_ptr<Mesh>);
	std::shared_ptr<PhysicsEngine> physics_engine;
};

}

#endif