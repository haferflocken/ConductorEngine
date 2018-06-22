#include <collection/ProgramParameters.h>

#include <asset/RecordSchema.h>
#include <codegen/InfoAssetCodeGen.h>
#include <file/JSONReader.h>
#include <json/JSONTypes.h>
#include <mem/UniquePtr.h>

#include <iostream>

int main(const int argc, const char* argv[])
{
	const Collection::ProgramParameters params{ argc, argv };
	
	std::string inputPath;
	if (!params.TryGet("-inputFile", inputPath))
	{
		std::cerr << "Missing required parameter: -inputFile <file>" << std::endl;
		return -1;
	}

	const Mem::UniquePtr<JSON::JSONValue> jsonInput = File::ReadJSONFile(File::MakePath(inputPath.c_str()));
	if (jsonInput->GetType() != JSON::ValueType::Object)
	{
		std::cerr << "Failed to parse the input as a JSON object." << std::endl;
		return -2;
	}
	const JSON::JSONObject& jsonObject = *dynamic_cast<const JSON::JSONObject*>(jsonInput.Get());

	const Asset::RecordSchema schema = Asset::RecordSchema::MakeFromJSON(jsonObject);

	const std::string structString = CodeGen::GenerateInfoInstanceStruct(schema);
	
	std::cout << structString << std::endl;

	return 0;
}
