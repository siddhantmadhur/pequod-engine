//
// Created by smadhur on 4/1/26.
//

#ifndef PEQUOD_SERIALIZATION_H_
#define PEQUOD_SERIALIZATION_H_
#include <tinyxml2.h>
#include <pobject/pobject.h>

using namespace tinyxml2;

namespace Pequod::Serialization {
    std::unique_ptr<XMLDocument> Marshal(std::shared_ptr<PObject>);

    std::shared_ptr<PObject> Unmarshal(std::unique_ptr<XMLDocument>);
}


#endif //
