// Source code of fragment shader
#version 130

// Attributes passed from the vertex shader
in vec4 color_interp;
in vec2 uv_interp;

out vec4 FragColor;

uniform sampler2D onetex;
uniform float drawLevelSelectHighlight;
uniform vec2 offset;
uniform vec3 colorMod;

void main() {
	vec4 color = texture2D(onetex, uv_interp);
	FragColor = vec4(color.r,color.g,color.b,color.a)*vec4(colorMod,1.0);
    if((color.r + color.b) / 2.0 > 0.98 && color.g <= 0.1) {
		discard;
	}

	if (drawLevelSelectHighlight != 0.0f) {
        float s = uv_interp.x-offset.x;
		float t = uv_interp.y-offset.y;

		if (sqrt(s*s + t*t/2) > drawLevelSelectHighlight/675-0.03 && sqrt(s*s + t*t/2) < drawLevelSelectHighlight/675-0.0225) {
			discard;
		}
	}
	//FragColor = color_interp;
}