/****************************************************************
* Kara Jensen (KaraPeaceJensen@gmail.com) 
* Generic world shader
*****************************************************************/

float4x4 World                 : World;
float4x4 WorldViewProjection   : WorldViewProjection;
float4x4 WorldInvTrans         : WorldInverseTranspose;

float3 LightPos;
float AmbientIntensity;     
float4 AmbientColor;
float DiffuseIntensity;     
float4 DiffuseColor;

struct VS_OUTPUT
{
    float4 Pos               : POSITION;
    float3 Normal            : TEXCOORD0;
    float3 LightVector       : TEXCOORD1;
};

// Vertex Shader
VS_OUTPUT VShader(float4 inPos : POSITION, 
                  float3 inNormal : NORMAL)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Pos = mul(inPos, WorldViewProjection); 
    float3 PosWorld = mul(inPos, World); 
   
    output.Normal = mul(inNormal,WorldInvTrans);
    output.LightVector = LightPos - PosWorld;

    return output;
}

//Pixel Shader
float4 PShader(VS_OUTPUT input) : COLOR0
{   
    input.LightVector = normalize(input.LightVector);
    input.Normal = normalize(input.Normal);
    
    float4 Diffuse = (dot(input.LightVector, input.Normal)+1)*0.5; //Brings into range 0-1
    Diffuse = Diffuse * DiffuseIntensity * DiffuseColor;
    
    float4 Ambient = AmbientIntensity * AmbientColor;
    
    return Diffuse + Ambient;
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