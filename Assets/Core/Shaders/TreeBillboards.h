uniform float heightScale;
uniform vec2 heightMapUvScale;
uniform vec2 heightMapUvOffset;
uniform vec2 attributeMapUvScale;
uniform vec2 attributeMapUvOffset;
uniform vec2 tileWorldSize;

uniform sampler2D heightSampler;
uniform sampler2D attributeSampler;

int getTerrainAttributeId(vec2 vertexPos)
{
	vec2 attributeTexCoord = vertexPos.yx * attributeMapUvScale + attributeMapUvOffset;
	return int(texelFetch(attributeSampler, ivec2(attributeTexCoord * textureSize(attributeSampler, 0)), 0).r * 255);
}

vec3 getWorldBillboardPositionFromVertexPosition(vec2 vertexPos)
{
	vec3 pos;
	pos.xy = (vertexPos.xy - vec2(0.5)) * tileWorldSize;
	pos.z = -(texture(heightSampler, vertexPos.yx * heightMapUvScale + heightMapUvOffset).r - 0.5f) * heightScale;
	return pos;
}
