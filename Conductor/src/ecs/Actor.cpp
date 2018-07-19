#include <ecs/Actor.h>

constexpr size_t k_sizeOfActor = sizeof(ECS::Actor);
static_assert(k_sizeOfActor == 32, "Actor should be half the size of a cache line.");
