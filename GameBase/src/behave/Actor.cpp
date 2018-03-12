#include <behave/Actor.h>

using namespace Behave;

constexpr size_t k_sizeOfActor = sizeof(Actor);
// TODO(refactor) make this work
//static_assert(k_sizeOfActor == 32, "Actor should be half the size of a cache line.");
