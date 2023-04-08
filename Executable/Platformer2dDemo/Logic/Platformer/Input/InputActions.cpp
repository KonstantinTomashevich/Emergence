#include <Platformer/Input/InputActions.hpp>

using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString PlatformerInputGroups::MOVEMENT_INPUT {"Platformer::Movement"};

const Emergence::Celerity::InputAction PlatformerInputActions::DIRECTED_MOVEMENT {PlatformerInputGroups::MOVEMENT_INPUT,
                                                                                  "Directed"_us};

const Emergence::Celerity::InputAction PlatformerInputActions::MOVEMENT_ROLL {PlatformerInputGroups::MOVEMENT_INPUT,
                                                                              "Roll"_us};
