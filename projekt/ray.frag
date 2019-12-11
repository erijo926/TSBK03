#version 150
#define M_PI 3.1415926535897932384626433832795

out vec4 outColor;

uniform float time;
uniform vec3 cam_pos;
uniform float ball_height;
uniform float drop_time;
uniform bool gravity;

float WAVE_HEIGHT;

float cube_dist(vec3 point, vec3 cube)
{
    vec3 d = abs(point)-cube;
    return min(max(d.x, max(d.y, d.z)), 0.0)+length(max(d, 0.0));
}

float map_sphere(vec3 point, vec3 c, float r)
{
    return length(point-c)-r;
}

// Noise generation functions (by Inigo Quilez):
// https://www.shadertoy.com/view/4sS3zG
// For more basics on noise:
// https://thebookofshaders.com/10/
float random(vec2 p)
{
    return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453123);
}

// VALUE noise implemented by Inigo Quilez
// https://en.wikipedia.org/wiki/Value_noise
float noise(in vec2 p)
{
    vec2 int_coord = floor(p);
    vec2 fract_coord = fract(p);
    vec2 u = fract_coord*fract_coord*(3.0-2.0*fract_coord); // Vad gör den här exakt? Någon sorts interpolator
    return -1.0+2.0*mix( mix( random( int_coord + vec2(0.0,0.0) ),
                     random( int_coord + vec2(1.0,0.0) ), u.x),
                mix( random( int_coord + vec2(0.0,1.0) ),
                     random( int_coord + vec2(1.0,1.0) ), u.x), u.y);
}


float wave_peak(vec2 p, float peak)
{
    // peak should denote the "sharpness" of the wave crest
    p += noise(p);
    vec2 wave = 1.0-sin(p);
    float no_p = pow(wave.x*wave.y,0.5);
    float d = pow(1.0-no_p,peak);
    return d;
}

float wave_dist(vec3 p, int level)
{
    vec2 point = p.xz;
    float a = .6; // amplitude
    float w = .2; // freq=2pi/wavelength. Has to be low -> large wavelength
    float v = .8; // wave speed
    float peak = 4.0;

    float h;
    float d = 0.0; // d = tot. dist, h = wave height
    const mat2 R = mat2(1.5,-1,1,1.5); //*(sqrt(2)/2);
    for(int i = 0; i < level; i++)
    {
        h = wave_peak((point+time*v)*w, peak);
        h += wave_peak((point-(time-2)*v)*w, peak);
        d += h*a;

        point *= R;
        w *= 2.0;
        a *= 0.3;
        peak *= 0.8;
    }
    return p.y-d;
}

float plane_dist(vec3 p, float h)
{
    return p.y-h;
}

float map(vec3 p)
{
    float water = wave_dist(p, 4);
    float cube = cube_dist(p, vec3(3.0, 3.0, 3.0));
    return max(water,cube);
}

vec3 sphere_norm(vec3 p, float d, vec3 c, float r)
{
    return normalize(vec3(
        map_sphere(vec3(p.x+d,p.y,p.z),c,r)-map_sphere(vec3(p.x-d,p.y,p.z),c,r),
        map_sphere(vec3(p.x,p.y+d,p.z),c,r)-map_sphere(vec3(p.x,p.y-d,p.z),c,r),
        map_sphere(vec3(p.x,p.y,p.z+d),c,r)-map_sphere(vec3(p.x,p.y,p.z-d),c,r)
    ));
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

float shadow(vec3 origin, vec3 dir, float start, float end, vec3 c, float r)
{
    float k = 6;
    float soft = 1.0;
    for (float d=start; d<end;)
    {
        float dist = map_sphere(origin+dir*d,c,r);
        if (dist<0.001) return 0.0;
        soft = min(soft, k*dist/d);
        d += dist;
    }
    return soft;
}

float cast_ray(vec3 origin, vec3 dir, vec3 c, float r)
{
    float dmin = 0.001;
    float dmax = 10.0;
    float d = dmin;
    for( int i=0; i<256; i++ )
    {
        float dist = map_sphere(origin+dir*d,c,r);
        if( dist<0.0001 || d>dmax ) break;
        d += dist;
    }

    if(d>dmax) d=-1.0;
    return d;
}

float trace_ball(vec3 orig, vec3 dir, float start, float end, vec3 c, float r)
{
    for (float d=start; d<end;)
    {
        float dist = map_sphere(orig+dir*d,c,r);
        if (dist<0.001) return d;
        d += dist;
    }
    return -1.0;
}

float trace_floor(vec3 orig, vec3 dir, float start, float end)
{
    for (float d=start; d<end;)
    {
        float dist = plane_dist(orig+dir*d,-1.0);
        if (dist<0.001) return d;
        d += dist;
    }
    return -1.0;
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
    float cosi = dot(i_dir,n);
    return i_dir+2.0*cosi*n;
}

vec3 shade_ball(vec3 pos, vec3 lpos, vec3 n, vec3 c, float r, vec3 w_dir)
{
    float diff, specular, shade;
    // vec3 clr = pos;
    vec3 clr = vec3(1.0,0,0);
    vec3 ldir = normalize(pos-lpos);
    vec3 r_spec = reflect(ldir, n);
    vec3 v = normalize(w_dir); // View direction
    specular = dot(r_spec, v);
    if (specular > 0.0) specular = 1.5*pow(specular, 10.0);
    specular = max(specular,0.0);
    diff = dot(n,ldir);
    shade = 0.7*diff+0.3*specular;

    return clr*shade;
}

vec3 shade_water(vec3 pos, vec3 cam, vec3 lpos, vec3 n, vec3 c, float r, vec3 w_dir)
{
    float specular, shade;
    vec3 total = vec3(0.0);

    vec3 refr = vec3(0.0);
    vec3 refl = vec3(0.0);
    float refractive_i = 1.33/1.00029;
    vec3 clr = normalize(vec3(63.0,127.0,191.0));
    vec3 inc_dir = normalize(pos-cam);
    vec3 refl_dir = normalize(test_refl(inc_dir,n));
    vec3 refr_dir = normalize(test_refr(inc_dir,n,refractive_i));

    // float d = cast_ray(pos,refl_dir,c,r);
    // float t = cast_ray(pos,refr_dir,c,r);
    float d = trace_ball(pos,refl_dir,0.0,length(pos-cam),c,r);
    float t = trace_ball(pos,refr_dir,0.0,length(pos-cam),c,r);

    if (d!=-1.0) refl += shade_ball(pos+refl_dir*d,lpos,n,c,r,w_dir);
    if (t!=-1.0) refr += shade_ball(pos+refr_dir*t,lpos,n,c,r,w_dir);

    // Diffuse
    vec3 ldir = normalize(pos-lpos);
    float diff = dot(n,ldir);

    // Specular
    // vec3 v = normalize(inc_dir); // View direction
    vec3 r_spec = reflect(ldir, n);
    specular = dot(r_spec, inc_dir);
    if (specular > 0.0)
        specular = 1.5 * pow(specular, 10.0);
    specular = max(specular, 0.0);

    shade = 0.7*diff+0.3*specular;
    float val = .0;
    // total += normalize(cam)*shade+refl; //*shade+refl;
    total += clr*shade+(val*refl)+((1.0-val)*refr);
    return total;
}

float drop_ball(float h)
{
    float acc = 0.982;
    float curr_time = time-drop_time;
    float v = acc*curr_time;
    return h-v;
}

//Perform the ray marching:
vec3 ray_march(vec3 camera, vec3 dir, float start, float dmax, float delta)
{
    // vec3 light_pos = vec3(-10*sin(time),-15.0,-10*cos(time));
    vec3 light_pos = vec3(-15.0,-20.0,-0.0);
    vec3 color = vec3(0.0);
    vec3 pos = vec3(0.0);
    vec3 c = vec3(0.0);
    if (gravity == false) c.y = ball_height;
    if (gravity == true) c.y = drop_ball(ball_height);

    float r = 1.0;
    float d = start;
    for(int i=0; i<256; i++)
    {
        bool sphere = false;
        pos = camera+dir*d;
        float dist = map(pos);
        float dist_s = map_sphere(pos,c,r);
        if (dist > dist_s) {
            sphere=true;
            d += dist_s;
        } else {d += dist;}

        if((dist<0.0001 || dist_s<0.0001) && d<dmax)
        {
            if (sphere) {
                vec3 normal = sphere_norm(pos,delta,c,r);
                return shade_ball(pos,light_pos,normal,c,r,dir);
            }
            else {
                vec3 normal = water_norm(pos,delta);
                return shade_water(pos,camera,light_pos,normal,c,r,dir);
            }
        }
    }
    return vec3(0.2,0.5,0.6);
}

void main()
{
    const float min_dist = 0.001;
    const float max_dist = 30.0;
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
