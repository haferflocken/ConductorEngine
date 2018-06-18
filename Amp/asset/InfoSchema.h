#pragma once

#include <asset/InfoSchemaField.h>

namespace Asset
{
/**
 * Defines the data layout of a class of text data files.
 */
class InfoSchema
{
	InfoSchemaField m_rootGroup;

public:
	InfoSchema();
};
}
