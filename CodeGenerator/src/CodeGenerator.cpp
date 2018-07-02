#include <collection/ProgramParameters.h>

#include <asset/RecordSchema.h>
#include <codegen/InfoAssetSaveFunctionCodeGen.h>
#include <codegen/RecordSchemaStructCodeGen.h>
#include <file/JSONReader.h>
#include <json/JSONTypes.h>
#include <mem/UniquePtr.h>

#include <cwchar>
#include <fstream>
#include <iostream>
#include <string>

int main(const int argc, const char* argv[])
{
	// Collect the required command line arguments.
	const Collection::ProgramParameters params{ argc, argv };
	
	std::string inputPath;
	if (!params.TryGet("-inputDir", inputPath))
	{
		std::cerr << "Missing required parameter: -inputDir <path>" << std::endl;
		return -1;
	}
	const File::Path inputDir = File::MakePath(inputPath.c_str());
	if (!File::IsDirectory(inputDir))
	{
		std::cerr << "-inputDir must specify a directory." << std::endl;
		return -1;
	}
	const size_t inputPathLength = wcslen(inputDir.c_str());

	std::string outputPath;
	if (!params.TryGet("-outputDir", outputPath))
	{
		std::cerr << "Missing required parameter: -outputDir <path>" << std::endl;
		return -1;
	}
	const File::Path outputDir = File::MakePath(outputPath.c_str());
	if (!File::IsDirectory(outputDir))
	{
		std::cerr << "-outputDir must specify a directory." << std::endl;
		return -1;
	}

	// Generate the output from the input.
	const bool result = File::ForEachFileInDirectoryRecursive(inputDir, [&](const File::Path& file, const size_t depth)
	{
		if ((!file.has_extension()) || wcscmp(file.extension().c_str(), L".json") != 0)
		{
			return true;
		}
		std::cout << file.filename() << std::endl;

		// Extract the namespace information of the generated code.
		constexpr size_t k_stackCapacity = 8;
		std::string parentStack[k_stackCapacity];
		size_t parentStackSize = 0;

		File::Path current = file;
		while (parentStackSize < depth)
		{
			if (parentStackSize == k_stackCapacity)
			{
				std::cerr << "!> File too nested from the input directory to extract namespace." << std::endl;
				return false;
			}
			current = current.parent_path();
			parentStack[parentStackSize++] = current.filename().string();
		}

		// Determine the type of code generation to do based on the top of the stack.
		if (parentStackSize == 0)
		{
			std::cerr << "!> Cannot determine code generation type at the top level directory." << std::endl;
			return false;
		}
		
		enum class GenerationType
		{
			InfoAsset,
			Component
		};
		GenerationType generationType;
		if (strcmp(parentStack[parentStackSize - 1].c_str(), "InfoAsset") == 0)
		{
			generationType = GenerationType::InfoAsset;
		}
		else if (strcmp(parentStack[parentStackSize - 1].c_str(), "Component") == 0)
		{
			generationType = GenerationType::Component;
		}
		else
		{
			std::cerr << "!> Unrecognized code generation type [";
			std::cerr << parentStack[parentStackSize - 1].c_str() << ']' << std::endl;
			return false;
		}

		// Determine the output filepath.
		File::Path outputFile = outputDir;
		for (int64_t i = parentStackSize - 1; i >= 0; --i)
		{
			outputFile /= parentStack[i];
		}
		outputFile /= file.filename();
		outputFile.replace_extension(".h");

		if (File::Exists(outputFile) && !File::IsRegularFile(outputFile))
		{
			std::cerr << "!> Desired output path is not an ordinary file [" << outputFile << ']' << std::endl;
			return false;
		}

		// Load the file as a RecordSchema.
		const Mem::UniquePtr<JSON::JSONValue> jsonInput = File::ReadJSONFile(file);
		if (jsonInput->GetType() != JSON::ValueType::Object)
		{
			std::cerr << "!> Failed to parse as a JSON object." << std::endl;
			return false;
		}
		const JSON::JSONObject& jsonObject = *dynamic_cast<const JSON::JSONObject*>(jsonInput.Get());
		const Asset::RecordSchema schema = Asset::RecordSchema::MakeFromJSON(jsonObject);
		if (!schema.CheckIsErrorFree())
		{
			return false;
		}

		// Generate and output code.
		Collection::ArrayView<std::string> namespaceNames{ parentStack, parentStackSize };
		CodeGen::StructGenParams structGenParams{ namespaceNames, "", schema };

		switch (generationType)
		{
		case GenerationType::InfoAsset:
		{
			std::fstream outputStream{ outputFile, std::ios_base::out | std::ios_base::trunc };
			CodeGen::GenerateStructFromRecordSchema(structGenParams, outputStream);
			CodeGen::GenerateInfoInstanceSaveFunction(namespaceNames, schema, outputStream);
			// CodeGen::GenerateInfoInstanceLoadFunction(namespaceNames, schema, outputStream);
			break;
		}
		case GenerationType::Component:
		{
			// TODO
			break;
		}
		}
		return true;
	});
	return (result ? 0 : -2);
}
