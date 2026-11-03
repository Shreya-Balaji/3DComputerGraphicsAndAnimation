#version 330 core
layout(std140) uniform Material {
    vec3 kd;
    vec3 ks;
    float shininess;
    float transparency;
};
void main()
{
    if (transparency < 0.0)
        discard; 
}