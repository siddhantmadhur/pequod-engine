//
// Created by smadhur on 4/1/26.
//

#include "serialization.h"

namespace Pequod::Serialization {
    std::unique_ptr<XMLDocument> Marshal(std::shared_ptr<PObject> object) {
        if (object == nullptr) {
            PDebug::warn("Marshalling empty pointer...");
            return nullptr;
        };
        PDebug::info("Marshalling object...");
        auto doc = std::make_unique<XMLDocument>();

        XMLElement* pRoot = doc->NewElement("object");
        pRoot->SetAttribute("name", object->name.c_str());
        doc->InsertFirstChild(pRoot);

        { // Position
            auto property = object->Get<Position>();
            if (property) {
                auto pElement = doc->NewElement("position");
                auto pos = property->Get();
                pElement->SetAttribute("x", pos.x);
                pElement->SetAttribute("y", pos.y);
                pElement->SetAttribute("z", pos.z);
                pRoot->InsertEndChild(pElement);
            }
        }

        { // Mesh
            auto property = object->Get<Mesh>();
            if (property) {
                auto pElement = doc->NewElement("mesh");
                auto scale = property->GetScale();
                auto pScale = pElement->InsertNewChildElement("scale");
                pScale->SetAttribute("x", scale.x);
                pScale->SetAttribute("y", scale.y);
                pScale->SetAttribute("z", scale.z);
                auto vertices = property->vertices;
                auto pVertices = pElement->InsertNewChildElement("vertices");
                for (auto vertex : vertices) {
                    auto pVertex = pVertices->InsertNewChildElement("vertex");
                    pVertex->SetAttribute("x", vertex.x);
                    pVertex->SetAttribute("y", vertex.y);
                    pVertex->SetAttribute("z", vertex.z);
                    pVertex->SetAttribute("r", vertex.r);
                    pVertex->SetAttribute("g", vertex.g);
                    pVertex->SetAttribute("b", vertex.b);
                    pVertex->SetAttribute("a", vertex.a);
                    pVertex->SetAttribute("u", vertex.u);
                    pVertex->SetAttribute("v", vertex.v);
                }
                pRoot->InsertEndChild(pElement);
            }
        }

        if (object->children.size() > 0) {
            XMLElement* children = doc->NewElement("children");
            for (auto child : object->children) {
                if (!child) continue;
                auto child_xml = Marshal(child);
                if (!child_xml) continue;
                auto cloned = child_xml->RootElement()->DeepClone(doc.get());
                children->InsertEndChild(cloned);
            }
            pRoot->InsertEndChild(children);
        }

        return doc;
    }

    std::shared_ptr<PObject> Unmarshal(std::unique_ptr<XMLDocument>) {
        auto object = std::make_unique<PObject>();



        return object;
    }
}
