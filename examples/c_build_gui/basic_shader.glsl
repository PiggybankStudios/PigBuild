@ctype mat4 mat4
@ctype vec2 v2
@ctype vec3 v3
@ctype vec4 v4

@vs vs
// +--------------------------------------------------------------+
// |                            Vertex                            |
// +--------------------------------------------------------------+
layout(binding=0) uniform VertParams
{
	uniform mat4 world;
	uniform mat4 view;
	uniform mat4 projection;
	uniform vec4 sourceRec;
	uniform vec2 textureSize;
};

in vec2 position;
in vec2 texCoord;
in vec4 color;

out vec4 fragColor;
out vec2 fragSampleCoord;

void main()
{
	gl_Position = projection * (view * (world * vec4(position, 0.0f, 1.0f)));
	fragColor = color;
	fragSampleCoord = vec2(
		((texCoord.x * sourceRec.z) + sourceRec.x) / textureSize.x,
		((texCoord.y * sourceRec.w) + sourceRec.y) / textureSize.y
	);
}
@end



@fs fs
// +--------------------------------------------------------------+
// |                           Fragment                           |
// +--------------------------------------------------------------+
layout(binding=1) uniform FragParams
{
	uniform vec4 tint;
};
layout(binding=0) uniform texture2D tex;
layout(binding=0) uniform sampler texSampler;

in vec4 fragColor;
in vec2 fragSampleCoord;

out vec4 outColor;

void main()
{
	vec4 sampleColor = texture(sampler2D(tex, texSampler), fragSampleCoord);
	outColor = fragColor * sampleColor * tint;
}
@end


// +--------------------------------------------------------------+
// |                           Program                            |
// +--------------------------------------------------------------+
@program basic_shader vs fs