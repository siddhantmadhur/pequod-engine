//
// Created by Siddhant Madhur on 27/03/2026.
//

#ifndef PEQUODENGINE_POBJECT_H
#define PEQUODENGINE_POBJECT_H
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <string>
#include "property.h"

namespace Pequod {
    class PObject {
    public:
        PObject();
        ~PObject();

        std::string name;

        template <class TProperty>
        std::shared_ptr<Property> GetProperty();
        void AddProperty();
    private:

        std::unordered_map<std::type_index, std::shared_ptr<Property>> properties;

    };
}

#endif //PEQUODENGINE_POBJECT_H