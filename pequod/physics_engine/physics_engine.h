/**
 * physics_engine.h
 **/
#ifndef PEQUOD_PHYSICS_H_
#define PEQUOD_PHYSICS_H_

#include <memory>
#include <Jolt/Jolt.h>

#include "ecs/position.hh"
#include "Jolt/Core/Core.h"
#include "pobject/properties/rigidbody.hh"

using namespace JPH;

namespace Pequod {
    namespace {
        class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
        {
        public:
            virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
            {
                switch (inObject1)
                {
                    case Layers::NON_MOVING:
                        return inObject2 == Layers::MOVING; // Non moving only collides with moving
                    case Layers::MOVING:
                        return true; // Moving collides with everything
                    default:
                        JPH_ASSERT(false);
                        return false;
                }
            }
        };

        namespace BroadPhaseLayers
        {
            static constexpr BroadPhaseLayer NON_MOVING(0);
            static constexpr BroadPhaseLayer MOVING(1);
            static constexpr uint NUM_LAYERS(2);
        };

        class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
        {
        public:
            BPLayerInterfaceImpl()
            {
                // Create a mapping table from object to broad phase layer
                mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
                mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
            }

            virtual uint GetNumBroadPhaseLayers() const override
            {
                return BroadPhaseLayers::NUM_LAYERS;
            }

            virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
            {
                JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
                return mObjectToBroadPhase[inLayer];
            }


        private:
            BroadPhaseLayer	mObjectToBroadPhase[Layers::NUM_LAYERS];
        };

        class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
        {
        public:
            virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
            {
                switch (inLayer1)
                {
                    case Layers::NON_MOVING:
                        PDebug::info("Non moving!");
                        return inLayer2 == BroadPhaseLayers::MOVING;
                    case Layers::MOVING:
                        return true;
                    default:
                        JPH_ASSERT(false);
                        return false;
                }
            }
        };

        class MyBodyActivationListener : public JPH::BodyActivationListener
        {
        public:
            virtual void OnBodyActivated(const JPH::BodyID &inBodyID, JPH::uint64 inBodyUserData) override {}
            virtual void OnBodyDeactivated(const JPH::BodyID &inBodyID, JPH::uint64 inBodyUserData) override {}
        };
    }

    class PhysicsEngine {

    public:
        /*
         * @param rigid_bodies ref to the rigidbodies component to simulate entities
         */
        PhysicsEngine(std::unordered_map<entity_id, std::shared_ptr<RigidBody>>&);

        void Simulate(int steps);
        void Register(uint64_t, std::shared_ptr<RigidBody>);

    private:
        std::unordered_map<JPH::BodyID, entity_id> jolt_bodies;

        TempAllocator *temp_allocator; // 10MB
        JobSystemThreadPool *job_system;

        PhysicsSystem physics_system;

        BPLayerInterfaceImpl broad_phase_layer_interface;
        ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
        ObjectLayerPairFilterImpl object_vs_object_layer_filter;
        MyBodyActivationListener body_activation_listener;
    };
}

#endif //