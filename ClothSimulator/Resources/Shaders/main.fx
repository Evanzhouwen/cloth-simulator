////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com
////////////////////////////////////////////////////////////////////////////////////////

float4x4 World                 : World;
float4x4 WorldViewProjection   : WorldViewProjection;
float4x4 WorldInvTrans         : WorldInverseTranspose;

float3 LightPosition;
float AmbientIntensity;     
float3 AmbientColor;
float DiffuseIntensity;     
float3 DiffuseColor;
float3 VertexColor;

Texture DiffuseTexture;
sampler ColorSampler = sampler_state 
{ 
    texture = <DiffuseTexture>; 
    magfilter = ANISOTROPIC; 
    minfilter = ANISOTROPIC; 
    mipfilter = ANISOTROPIC; 
    AddressU = WRAP; 
    AddressV = WRAP; 
};

struct VS_OUTPUT
{
    float4 Position     : POSITION;
    float3 Normal       : TEXCOORD0;
    float3 LightVector  : TEXCOORD1;
	float2 UV           : TEXCOORD2;
};                 
                        
// Vertex Shader
VS_OUTPUT VShader(float4 inPos    : POSITION, 
                  float3 inNormal : NORMAL,
				  float2 inUV     : TEXCOORD0)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Position = mul(inPos, WorldViewProjection); 
    output.Normal = mul(inNormal,WorldInvTrans);
    output.LightVector = LightPosition - mul(inPos, World);
	output.UV = inUV;

    return output;
}

//Pixel Shader
float4 PShader(VS_OUTPUT input) : COLOR0
{   
    input.LightVector = normalize(input.LightVector);
    input.Normal = normalize(input.Normal);
    
    // bring diffuse into range of 0.4->1.0
    float inner = 0.4;
    float3 diffuse = ((dot(input.LightVector, input.Normal)
        +1.0)*((1.0-inner)/(2.0)))+inner;

    diffuse *= DiffuseIntensity * DiffuseColor;
    float3 ambient = AmbientIntensity * AmbientColor;
	float3 color = saturate(tex2D(ColorSampler, input.UV).xyz + VertexColor);

    return float4(color * (diffuse + ambient), 1.0);
}

//Techniques
technique Main
{
    pass Pass0
    {
        LIGHTING = TRUE;
        ZENABLE = TRUE;
        ZWRITEENABLE = TRUE;
        CULLMODE = CCW;

        VertexShader = compile vs_2_0 VShader();
        PixelShader = compile ps_2_0 PShader();
    }
}