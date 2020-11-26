// Source code of fragment shader
#version 130

// Attributes passed from the vertex shader
in vec4 color_interp;
in vec2 uv_interp;

out vec4 FragColor;

uniform sampler2D onetex;
uniform float drawLevelSelectHighlight;
uniform vec2 offset;

void main() {
	vec4 color = texture2D(onetex, uv_interp);
    if((color.r + color.b) / 2.0 > 0.98) {
		discard;
	}

	FragColor = vec4(0.7,0.7,0.7,1.0);
}
