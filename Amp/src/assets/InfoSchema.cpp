#include <asset/InfoSchema.h>

namespace Asset
{
InfoSchema::InfoSchema()
	: m_rootGroup(InfoSchemaField::MakeGroupField(0))
{}
}
