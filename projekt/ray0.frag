#version 150
#define M_PI 3.1415926535897932384626433832795

out vec4 outColor;

uniform float time;
uniform vec3 cam_pos;

const float SEA_HEIGHT = 0.6;
const float SEA_CHOPPY = 4.0;
const float SEA_FREQ = 0.16;

// Noise generation functions (by iq)
float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float hash( vec2 p ) {
    return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453123);
}

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	// vec2 rg = textureLod( iChannel1, (uv+0.5)/256.0, 0.0).yx;
	return mix( uv.x, uv.y, f.z );
}

float noise( in vec2 p ) {
    vec2 i = floor( p );
    vec2 f = fract( p );
	f = f*f*(3.0-2.0*f);
    return -1.0+2.0*mix( mix( hash( i + vec2(0.0,0.0) ),
                     hash( i + vec2(1.0,0.0) ), f.x),
                mix( hash( i + vec2(0.0,1.0) ),
                     hash( i + vec2(1.0,1.0) ), f.x), f.y);
}

float sea_octave(vec2 uv, float choppy) {
    uv += noise(uv);
    vec2 wv = 1.0-abs(sin(uv));
    vec2 swv = abs(cos(uv));
    wv = mix(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

float mapWater(vec3 p, int steps) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;

    float d, h = 0.0;
    const float SEA_SPEED = 0.8;
    const mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);
    float seaTime = time;
    d = sea_octave((uv+seaTime)*freq,choppy);
    for(int i = 0; i < steps; i++)
    {
    	d += sea_octave((uv-seaTime)*freq,choppy);
        h += d * amp;
    	uv *= octave_m;
        freq *= 1.9;
        amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

float wave_dist(vec3 p)
{
    vec2 point = vec2(p.x,p.z);
    int wc = 2;
    float d = 0.0;
    float v = 0.4; //+random(point,1.0); // hastighet
    vec2 c = vec2(0.0,0.0);
    vec2 r = vec2(0.1,-0.5);

    // vec2 r = (-1)*(point-c)/(length(point-c)); // cirkelvåg
    vec2 r2 = vec2(-0.7,0.3);
    vec2 r3 = vec2(-0.3,0.8);

    float a = 0.2; // +random(point,2.0); // amplitud
    float L = 1.3; // +random(point,5.0);
    float w = 2*M_PI/L; // vinkelfrek=2pi/våglängd
    d += 2*a*pow(sin(dot(r,point)*w+time*(v*w))/2,2);
    d += 2*0.3*pow(sin(dot(r2,point)*w+time*(v*4*w/2.0))/2,2.5);
    d += 2*0.1*pow(sin(dot(r3,point)*w+time*(v*10*w))/2,2.5);
    return d;
}

float plane_dist(vec3 p, float h)
{
    return p.y-h;
}

float cube_dist(vec3 point, vec3 cube)
{
    vec3 d = abs(point)-cube;
    return min(max(d.x, max(d.y, d.z)), 0.0)+length(max(d, 0.0));
}

float map(vec3 p)
{
    // float wave = wave_dist(p);
    // float plane = (p.y-wave);
    float water = mapWater(p, 4);
    float cube = cube_dist(p, vec3(3.0, 2.0, 3.0));
    return max(water,cube);
}

vec3 water_norm(vec3 p, float d)
{
    // return vec3(0,1,0);
    return normalize(vec3(
        map(vec3(p.x, p.y+d, p.z)) - map(vec3(p.x, p.y-d, p.z)),
        map(vec3(p.x+d, p.y, p.z)) - map(vec3(p.x-d, p.y, p.z)),
        map(vec3(p.x, p.y, p.z+d)) - map(vec3(p.x, p.y, p.z-d))
    ));
}
//This is a copy of the gluLookAt function
mat4 look_at(vec3 eye, vec3 center, vec3 up)
{
    vec3 f = normalize(center - eye);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);
    return mat4(
        vec4(s, 0.0),
        vec4(u, 0.0),
        vec4(-f, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
}

//From: http://prideout.net/blog/?p=64
vec3 ray_dir(float fieldOfView, vec2 size, vec2 fragCoord)
{
    vec2 xy = fragCoord-size/2.0;
    float z = size.y/tan(radians(fieldOfView)/2.0); //Distance to origin
    return normalize(vec3(xy,-z));
}

float trace_floor(vec3 orig, vec3 dir, float start, float end)
{
    for (float d=start; d<end;)
    {
        float dist = plane_dist(orig+dir*d,-1.0);
        if (dist<0.001) return d;
        d += dist;
    }
    return 0.0;
}

vec3 test_refr(vec3 inc_dir, vec3 n, float index)
{
    float ind = 1.00029/1.33;
    float cosi = -dot(inc_dir,n);
    float sin2t = pow(ind,2)*(1-pow(cosi,2));
    vec3 R = ind*inc_dir + (ind*cosi - (1-sqrt(1-sin2t)));
    return R;
}

vec3 test_refl(vec3 i_dir, vec3 n)
{
    float cosi = -dot(i_dir,n);
    return i_dir+2.0*cosi*n;
}

vec3 shade_water(vec3 pos, vec3 cam, vec3 lpos, vec3 n, vec3 c, float r, vec3 w_dir)
{
    float specular, shade;
    vec3 total = vec3(0.0);

    // vec3 refl, refr;
    vec3 refr = vec3(0.0);
    vec3 refl = vec3(0.0);
    float refractive_i = 1.33/1.00029;
    vec3 clr = normalize(vec3(63.0,127.0,191.0));
    vec3 inc_dir = normalize(pos-cam);

    // Diffuse
    vec3 ldir = normalize(pos-lpos);
    float diff = dot(n,ldir);

    // Specular
    // vec3 v = normalize(inc_dir); // View direction
    vec3 r_spec = test_refl(ldir, n);
    specular = dot(r_spec, inc_dir);
    if (specular > 0.0)
        specular = 1.5 * pow(specular, 10.0);
    specular = max(specular, 0.0);

    shade = 0.7*diff+1.0*specular;
    // total += normalize(cam)*shade+refl; //*shade+refl;
    total += clr*shade;
    return total;
}

//Perform the ray marching:
vec3 ray_march(vec3 camera, vec3 dir, float start, float end, float delta)
{
    // vec3 light_pos = vec3(-10*sin(time),-15.0,-10*cos(time));
    vec3 light_pos = vec3(-15.0,-20.0,-0.0);
    vec3 color = vec3(0.0);
    vec3 c = vec3(0,0.0,0);
    float r = 0.5;
    vec3 pos = vec3(0.0);
    float dmin = start;
    float dmax = 30.0;
    float d = dmin;
    for(int i=0; i<256; i++)
    {
        pos = camera+dir*d;
        float dist = map(pos);
        d += dist;
        if(dist<0.0001 && d<dmax)
        {
            vec3 normal = water_norm(pos,delta);
            return shade_water(pos,camera,light_pos,normal,c,r,dir);
        }
    }
    return vec3(0.2,0.5,0.6);
}

void main()
{
    const float min_dist = 0.001;
    const float max_dist = 100;
    const float delta = 0.01; //Checks distance from current pos to object

    // vec3 cam_pos = vec3(5*sin(time),3,5*cos(time));
    // vec3 cam_pos = vec3(15,5,0);
    vec2 resolution = vec2(600,600); //Same as the window res
    vec3 view_dir = ray_dir(60, resolution, gl_FragCoord.xy);
    //gl_FragCoord contains the window-relative coordinates of current fragment
    mat4 view_to_world = look_at(cam_pos, vec3(0.0,0.0,0.0), vec3(0.0,1.0,0.0));
    vec3 world_dir = (view_to_world * vec4(view_dir, 0.0)).xyz;
    outColor = vec4(ray_march(cam_pos,world_dir,min_dist,max_dist,delta),1.0);
}
