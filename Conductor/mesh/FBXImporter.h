#pragma once

#include <file/Path.h>

namespace Mesh
{
class TriangleMesh;

/**
 * Import an FBX file as a TriangleMesh.
 */
bool TryImportFBX(const File::Path& filePath, TriangleMesh* destination);
}
