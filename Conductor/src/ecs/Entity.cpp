#include <ecs/Entity.h>

constexpr size_t k_sizeOfEntity = sizeof(ECS::Entity);
static_assert(k_sizeOfEntity == 32, "Entity should be half the size of a cache line.");
