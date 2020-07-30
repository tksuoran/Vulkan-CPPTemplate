#ifndef configuration_hpp_vipu_graphics
#define configuration_hpp_vipu_graphics

#include "graphics/surface.hpp"

namespace vipu
{

class Configuration
{
public:
    Surface::Type surface_type{Surface::Type::eNone};
};

} // namespace vipu

#endif // configuration_hpp_vipu_graphics
