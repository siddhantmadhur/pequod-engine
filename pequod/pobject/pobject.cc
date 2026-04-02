//
// Created by Siddhant Madhur on 27/03/2026.
//

#include "pobject.h"
#include <format>
#include "debugger/debugger.hh"


namespace Pequod {
    PObject::PObject() {
    }

    PObject::~PObject() {
    }

    void PObject::AddChild(std::shared_ptr<PObject> child) {
        children.push_back(child);
    }


    void PObject::SetECS(std::shared_ptr<ECS> _ecs) {
        this->ecs = _ecs;
        for (auto &reg: pending_registrations) {
            reg(ecs, id);
        }
        pending_registrations.clear();
    }
}
