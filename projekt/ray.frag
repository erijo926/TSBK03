#version 150
#define M_PI 3.1415926535897932384626433832795

out vec4 outColor;

uniform float time;
uniform vec3 cam_pos;

float cube_dist(vec3 point, vec3 cube)
{
    vec3 d = abs(point)-cube;
    return min(max(d.x, max(d.y, d.z)), 0.0)+length(max(d, 0.0));
}

float map_sphere(vec3 point, vec3 c, float r)
{
    return length(point-c)-r;
}

float PHI = 1.61803398874989484820459 * 00000.1; // Golden Ratio
float PI  = 3.14159265358979323846264 * 00000.1; // PI
float SQ2 = 1.41421356237309504880169 * 10000.0; // Square Root of Two

float random(vec2 coordinate,float seed){
    float max_val = 0.5;
    float min_val = -1.0;
    float temp = fract(tan(distance(coordinate*(seed+PHI), vec2(PHI, PI)))*SQ2);
    return floor(temp*(max_val-min_val+1)+min_val);
}

float wave_dist(vec3 p)
{
    vec2 point = vec2(p.x,p.z);
    int wc = 2;
    float d = 0.0;
    float v = 0.1; //+random(point,1.0); // hastighet
    vec2 c = vec2(0.0,0.0);
    // vec2 r = (-1)*(point-c)/(length(point-c)); // cirkelvåg
    vec2 r = vec2(0.5,0.1);
    vec2 r2 = vec2(0.7,-0.3);
    vec2 r3 = vec2(-0.3,0.8);
    float a = 0.2; // +random(point,2.0); // amplitud
    float L = 1.0; // +random(point,5.0);
    float w = 2*M_PI/L; // vinkelfrek=2pi/våglängd
    for (int i=0; i < wc; i++)
    {
    }
    d += 2*a*pow(sin(dot(r,point)*w+time*(v*w))/2,2.5);
    d += 2*0.3*pow(sin(dot(r2,point)*w+time*(v*4*w/2.0))/2,2.5);
    d += 2*0.1*pow(sin(dot(r3,point)*w+time*(v*10*w))/2,2.5);
    return d;
}

float ave_dist(vec3 p)
{
    p *= .2*vec3(1.0);
    const int octaves = 4;
    float f = 0.0;
    p += time*vec3(0,.1,.1);
    for ( int i=0; i < octaves; i++ )
    {
        p = (p.yzx + p.zyx*vec3(1,-1,1))/sqrt(2.0);
        f  = f*1.0+abs(random(vec2(p.x,p.y),1.0)-.5)*2.0;
        p *= 2.0;
    }
    f /= exp2(float(octaves));

    return (.5-f)*1.0;
}

float plane_dist(vec3 p, float h)
{
    return p.y-h;
}

float map(vec3 p)
{
    float wave = wave_dist(p);
    float plane = (p.y-wave);
    float bbox = cube_dist(p, vec3(3.0,0.5,3.0));
    return plane;
}

vec3 sphere_norm(vec3 p, float d, vec3 c, float r)
{
    return normalize(vec3(
        map_sphere(vec3(p.x+d, p.y, p.z),c,r) - map_sphere(vec3(p.x-d, p.y, p.z),c,r),
        map_sphere(vec3(p.x, p.y+d, p.z),c,r) - map_sphere(vec3(p.x, p.y-d, p.z),c,r),
        map_sphere(vec3(p.x, p.y, p.z+d),c,r) - map_sphere(vec3(p.x, p.y, p.z-d),c,r)
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
    float dmax = 20.0;
    float d = dmin;
    for( int i=0; i<256; i++ )
    {
        // float precis = 0.0005*d;
        float dist = map(origin+dir*d);
        float dist_s = map_sphere(origin+dir*d,c,r);
        if (dist < dist_s);
        else dist = dist_s;
        if( dist<0.0001 || d>dmax ) break;
        d += dist;
    }

    if(d>dmax) d=-1.0;
    return d;
}

vec3 shade_ball(vec3 pos, vec3 lpos, vec3 n, vec3 c, float r)
{
    vec3 total = vec3(pos.x,pos.y,pos.z);
    vec3 l_dir = normalize(pos-lpos);
    float diff = dot(n,l_dir);
    return total*diff;
}

float trace_ball(vec3 orig, vec3 dir, float start, float end, vec3 c, float r)
{
    for (float d=start; d<end;)
    {
        float dist = map_sphere(orig+dir*d,c,r);
        if (dist<0.001) return d;
        d += dist;
    }
    return 0.0;
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

vec3 shade_water(vec3 pos, vec3 cam, vec3 lpos, vec3 n, vec3 c, float r)
{
    vec3 refl = vec3(0.0);
    vec3 refr = vec3(0.0);
    float refractive_i = 1.00029/1.33;
    vec3 clr = vec3(1.0);
    vec3 total = vec3(0.1);
    vec3 refl_dir = normalize(reflect(n,cam));
    vec3 refr_dir = normalize(refract(cam,n,refractive_i));

    vec3 l_dir = normalize(pos-lpos);
    float diff = dot(n,l_dir);
    float d = trace_ball(pos,refl_dir,0.0,length(pos-cam),c,r);
    if(d!=0.0) refl = shade_ball(pos+refl_dir*d,lpos,n,c,r);

    d = trace_ball(pos,refr_dir,0.0,length(pos-cam),c,r);
    if(d!=0.0) refr = shade_ball(pos+refr_dir*d,lpos,n,c,r);

    float val = 0.8;
    total += clr*diff*0.6+(val*refl)+((1.0-val)*refr);
    return total;
}

//Perform the ray marching:
vec3 ray_march(vec3 camera, vec3 dir, float start, float end, float delta)
{
    // vec3 light_pos = vec3(-10*sin(time),-15.0,-10*cos(time));
    vec3 light_pos = vec3(-10.0,-15.0,-10.0);
    vec3 color = vec3(0.0);
    vec3 c = vec3(0,0.3,0);
    float r = 0.5;

    float dmin = 0.001;
    float dmax = 30.0;
    float d = dmin;
    for(int i=0; i<256; i++)
    {
        vec3 pos = camera+dir*d;
        float dist = map(pos);
        float dist_s = map_sphere(pos,c,r);
        if (dist > dist_s) dist = dist_s;
        d += dist;
        if(dist<0.0001 && d<dmax)
        {
            if (dist == dist_s) {
                vec3 normal = sphere_norm(pos,delta,c,r);
                return shade_ball(pos,light_pos,normal,c,r);
            }
            else {
                vec3 normal = water_norm(pos,delta);
                return shade_water(pos,camera,light_pos,normal,c,r);
            }
            // return color;
        }
    }
    return vec3(0.2,0.5,0.85);
}

void main()
{
    const float min_dist = 0;
    const float max_dist = 100;
    const float delta = 0.001; //Checks distance from current pos to object

    // vec3 cam_pos = vec3(12,5,10);

    vec2 resolution = vec2(600,600); //Same as the window res
    vec3 view_dir = ray_dir(60, resolution, gl_FragCoord.xy);
    //gl_FragCoord contains the window-relative coordinates of current fragment
    mat4 view_to_world = look_at(cam_pos, vec3(0.0,0.0,0.0), vec3(0.0,1.0,0.0));
    vec3 world_dir = (view_to_world * vec4(view_dir, 0.0)).xyz;
    outColor = vec4(ray_march(cam_pos,world_dir,min_dist,max_dist,delta),1.0);
}
