// A simple "chromatic aberration" example
/*
#pragma data_seg(".shader")
static const char* post = \
"#version 130\n"
"uniform sampler2D o;"
"out vec4 i;"
"vec3 ca(sampler2D t, vec2 u){"
	"const int n=10;"
	"vec3 c=vec3(0);"
	"float rf=1,gf=1,bf=1;"
	"for(int i=0;i<n;++i){"
		"c.r+=texture(t,.5+.5*(u*rf)).r;"
		"c.g+=texture(t,.5+.5*(u*gf)).g;"
		"c.b+=texture(t,.5+.5*(u*bf)).b;"
		"rf*=.9988;"
		"gf*=.9982;"
        "bf*=.996;"
	"}"
	"return c/n;"
"}"
"void main(){"
	"i = vec4(ca(o,-1+2*gl_FragCoord.xy/vec2(1280,720)),1);"
"}";
*/

// Mipmap fake glossy thing!
#pragma data_seg(".shader")
static const char* post = \
"#version 130\n"
"uniform sampler2D o;"
"out vec4 i;"
"float hash(float c){return fract(sin(dot(c, 12.9898)) * 43758.5453);}\n"
"void main(){"
"i = vec4(0);"
	"for(int t = 0; t < 25; t++){"
		"vec2 uv = gl_FragCoord.xy/vec2(1280,720);"
		"float s1 = hash(float(t+dot(uv,uv)));"
		"float s2 = hash(float(1-t+dot(uv,uv)));"
		"vec2 f = 0.01*(-1.0+2.0*vec2(s1,s2));"
		"i += vec4(textureLod(o, uv, (0.3+0.7*s1)*texture(o, (1.0+1.0*f)*uv).a*8).rgb,1);"
	"}"
	"i /= vec4(25);"
"}";
