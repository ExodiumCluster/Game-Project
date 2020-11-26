// Source code of vertex shader for particle system
#version 130

// Vertex buffer
in vec2 vertex;
in vec2 dir;
in float t; //phase
in vec2 uv;

// Uniform (global) buffer
uniform mat4 transformationMatrix;
uniform mat4 viewMatrix;
uniform float time;

// Attributes forwarded to the fragment shader
out vec4 color_interp;
out vec2 uv_interp;

void main() {
	vec4 ppos;
	float acttime;
	float speed = 6;
	float gravity = 4.9;
	acttime = mod(time + t*4.0, 1.0);
	
	vec2 scaled_vertex = vec2(vertex.x/(2*acttime+1), vertex.y/(2*acttime+1));

    ppos = vec4(scaled_vertex.x+dir.x*speed/7, abs(scaled_vertex.y+dir.y*acttime*speed + 0.5*gravity*acttime*acttime), 0.0, 1.0);
    gl_Position = viewMatrix*transformationMatrix*ppos;
	
    //color_interp = vec4(0.5+0.5*cos(4*acttime),0.5*sin(4*acttime)+0.5,0.5, 1.0);
    color_interp = vec4(uv,0.5,1.0/acttime);
	uv_interp = uv;
}