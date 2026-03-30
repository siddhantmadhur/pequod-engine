//
// Created by Siddhant Madhur on 27/03/2026.
//

#include "pobject.h"
#include <format>
#include "debugger/debugger.hh"

namespace Pequod {
    template <class TProperty>
    std::shared_ptr<TProperty> GetProperty() {
        std::shared_ptr<TProperty> property = std::make_shared<TProperty>();

    }
}
