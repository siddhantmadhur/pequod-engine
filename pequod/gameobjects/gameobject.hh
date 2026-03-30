#ifndef PEQUOD_GAMEOBJECT_IMPL_HH_
#define PEQUOD_GAMEOBJECT_IMPL_HH_

#include <typeindex>

#include "ecs/ecs.hh"
#include "ecs/mesh.hh"
#include <ecs/position.hh>
#include <vector>

#include "../pobject/property.h"

class GameObject {
public:
    GameObject();
    ~GameObject();
    //template <class TProperty>
    //std::shared_ptr<Property> GetProperty();
protected:
    //std::unordered_map<std::type_index, Property> properties;
};


#endif
