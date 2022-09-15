#version 460 core
out vec4 FragColor;
  
in vec2 tex_coord;

layout(binding = 0) uniform sampler2D tex0;
layout(binding = 1) uniform sampler2D tex1;

uniform float mix_value;

void main()
{
    // texture(tex1, TexCoord).a * mix_value deals with transparency nicely, but is a hard coded solution
    // look into this later: 
    // glEnable(GL_BLEND);// you enable blending function
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    FragColor = mix(texture(tex0, tex_coord), texture(tex1, tex_coord), texture(tex1, tex_coord).a * mix_value);
}
