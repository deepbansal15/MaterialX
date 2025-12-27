// Example Usage of MaterialX HLSL Shader Generator
// This file demonstrates how to use the new HLSL shader generator

#include <MaterialXCore/Document.h>
#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

using namespace MaterialX;

// Example function to generate HLSL shader from a MaterialX document
void generateHLSLShader(DocumentPtr materialXDoc, const string& outputFilename)
{
    // Create HLSL shader generator
    HlslShaderGeneratorPtr hlslGenerator = HlslShaderGenerator::create();
    
    // Create generation context
    GenContext context(hlslGenerator);
    
    // Find a surface material in the document
    MaterialPtr material = materialXDoc->getMaterial("MyMaterial");
    if (!material)
    {
        throw Exception("Material not found");
    }
    
    // Generate shader
    ShaderPtr shader = hlslGenerator->generate("MyHLSLShader", material, context);
    
    // Get generated HLSL code
    const string& vertexShader = shader->getStage(Stage::VERTEX)->getSourceCode();
    const string& pixelShader = shader->getStage(Stage::PIXEL)->getSourceCode();
    
    // Output the generated shaders
    // In a real application, you would write these to files
    // or compile them directly using FXC or DXC
    
    // Example: Compile using FXC (Windows)
    // CompileVertexShader(vertexShader, "VS_Main", "vs_5_0", "vertex.hlsl");
    // CompilePixelShader(pixelShader, "PS_Main", "ps_5_0", "pixel.hlsl");
    
    return;
}

// Example of a simple MaterialX material that would generate HLSL
const char* exampleMaterialX = R"(
<?xml version="1.0"?>
<materialx version="1.38" colorscheme="default" namespace="test">
  <material name="SimpleMaterial" />
  
  <nodedef name="SimpleSurface" node="surfacematerial" />
  
  <nodegraph name="SimpleGraph" nodedef="SimpleSurface">
    <!-- Base color -->
    <constant name="baseColorValue" type="color3">
      <parameter name="value" type="color3" value="0.8, 0.2, 0.1" />
    </constant>
    
    <!-- Roughness -->
    <constant name="roughnessValue" type="float">
      <parameter name="value" type="float" value="0.5" />
    </constant>
    
    <!-- Metallic -->
    <constant name="metallicValue" type="float">
      <parameter name="value" type="float" value="0.0" />
    </constant>
    
    <!-- Output connection -->
    <output name="surfaceOutput" type="surfaceshader" nodename="standardSurface" />
    
    <standardSurface name="standardSurface" type="surfaceshader">
      <input name="baseColor" type="color3" nodename="baseColorValue" />
      <input name="roughness" type="float" nodename="roughnessValue" />
      <input name="metallic" type="float" nodename="metallicValue" />
    </standardSurface>
  </nodegraph>
  
  <materialassign name="materialAssign" material="SimpleMaterial" nodegraph="SimpleGraph" />
</materialx>
)";

int main()
{
    // Create MaterialX document
    DocumentPtr doc = createDocument();
    
    // In real usage, you would load from file:
    // DocumentPtr doc = createDocument();
    // readFromXmlFile(doc, "material.mtlx");
    
    // Generate HLSL shader
    try
    {
        generateHLSLShader(doc, "MyShader");
    }
    catch (const Exception& e)
    {
        // Handle shader generation errors
        return 1;
    }
    
    return 0;
}
