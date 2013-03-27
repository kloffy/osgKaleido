#version 400

uniform vec3 ecLightDirection;
uniform vec3 lightColor; // Ld

in vec3 gFacetNormal;
in vec3 gTriDistance;
in vec3 gPatchDistance;
in vec3 gNormal;
in float gPrimitive;

out vec4 fragColor;

float amplify(float d, float scale, float offset)
{
    d = scale * d + offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2*d*d);
    return d;
}

vec3 diffuseModel(vec3 lightDirection, vec3 normal, vec3 Ld, vec3 Kd)
{
	return Ld * Kd * max(dot(lightDirection, normal), 0.0);
}

void main()
{
//	vec3 ecNormal = normalize(gFacetNormal);
    vec3 ecNormal = gNormal;

    vec3 color = diffuseModel(ecLightDirection, ecNormal, lightColor, vec3(1.0, 0.0, 0.0));
/*
    float d1 = min(min(gTriDistance.x, gTriDistance.y), gTriDistance.z);
    float d2 = min(min(gPatchDistance.x, gPatchDistance.y), gPatchDistance.z);
    color = amplify(d1, 40, -0.5) * amplify(d2, 60, -0.5) * color;
*/
    fragColor = vec4(color, 1.0);
}