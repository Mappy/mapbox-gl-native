#ifndef MBGL_ANNOTATION_POINT_ANNOTATION
#define MBGL_ANNOTATION_POINT_ANNOTATION

#include <mbgl/util/geo.hpp>

#include <string>

namespace mbgl {

class PointAnnotation {
public:
    inline PointAnnotation(const LatLng& position_, const std::string& icon_ = "", const uint32_t zOrder_ = 0)
        : position(position_), icon(icon_), zOrder(zOrder_) {
    }

    const LatLng position;
    const std::string icon;
    const uint32_t zOrder;
};

} // namespace mbgl

#endif
