#include <ecs/Entity.h>

constexpr size_t k_sizeOfEntity = sizeof(ECS::Entity);
static_assert(k_sizeOfEntity == 64, "Entity should be the size of a cache line.");

const Util::StringHash ECS::Entity::k_typeHash = Util::CalcHash(ECS::Entity::k_typeName);
