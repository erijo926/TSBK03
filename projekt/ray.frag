#version 150

out vec4 outColor;

uniform float time;
// uniform vec3 sentCam;

#define M_PI 3.14159265358979323846

//Distance function by Inigo Quilez
//http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float cubeDist(vec3 point, vec3 cube)
{
    vec3 d = abs(point)-cube;
    return min(max(d.x, max(d.y, d.z)), 0.0)+length(max(d, 0.0));
}

float sphereDist(vec3 point, vec3 c, float r)
{
    // vec3 c = vec3(0.0,0.0,0.0); //Allows for moving the sphere later
    return length(point-c)-r;
}

float planeDist(vec3 p, float h)
{
  // n must be normalized
  float d = p.y-h;
  return d;
  // return dot(p,n.xyz) + n.w;
}

float intersect(float A, float B)
{
    return max(A, B);
}

float funion(float A, float B)
{
    return min(A,B);
}

float smooth_union ( float d1, float d2, float k )
{
    float h = clamp(0.5 + 0.5*(d2-d1)/k, 0.0, 1.0);
    return mix(d2, d1, h) - k*h*(1.0-h);
}

float map(vec3 p)
{
    float displacement = sin(time+p.x) * sin(time+p.y) * sin(time+p.z);

    float floater = sphereDist(p,vec3(0.0,2.0,0.0),0.8);
    // float floater = cubeDist(p-vec3(0.0,1.5,0.0),vec3(0.5,0.5,0.5));
    float plane = planeDist(p, 0.0);
    float bbox = cubeDist(p, vec3(3.0,1.0,3.0));

    return min(max(plane,bbox),floater);
}

vec3 calc_norm(vec3 point, float delta)
{
    return normalize(vec3(
        map(vec3(point.x+delta, point.y, point.z)) - map(vec3(point.x-delta, point.y, point.z)),
        map(vec3(point.x, point.y+delta, point.z)) - map(vec3(point.x, point.y-delta, point.z)),
        map(vec3(point.x, point.y, point.z+delta)) - map(vec3(point.x, point.y, point.z-delta))
    ));
}

//This is the gluLookAt function
mat4 lookAT(vec3 eye, vec3 center, vec3 up)
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
vec3 rayDirection(float fieldOfView, vec2 size, vec2 fragCoord)
{
    vec2 xy = fragCoord-size/2.0;
    float z = size.y/tan(radians(fieldOfView)/2.0); //Distance to origin
    return normalize(vec3(xy,-z));
}

float shadow(vec3 origin, vec3 dir, float start, float end)
{
    float k = 6;
    float soft = 1.0;
    for (float d=start; d<end;)
    {
        float dist = map(origin+dir*d);
        if (dist<0.001) return 0.0;
        soft = min(soft, k*dist/d);
        d += dist;
    }
    return soft;
}

float cast_ray(vec3 origin, vec3 dir)
{
    float dmin = 0.001;
    float dmax = 20.0;
    float d = dmin;
    for( int i=0; i<256; i++ )
    {
        // float precis = 0.0005*d;
        float dist = map(origin+dir*d);
        if( dist<0.0001 || d>dmax ) break;
        d += dist;
    }

    if(d>dmax) d=-1.0;
    return d;
}
//Perform the ray marching:
vec3 rayMarch(vec3 camera, vec3 dir, float start, float end, float delta)
{
    float depth = start;
    vec3 light_pos = vec3(10*sin(time),-15.0,-10*cos(time));
    // vec3 light_pos = vec3(0.0,-20.0,0.0);
    vec3 color = vec3(1.0);
    float d = cast_ray(camera, dir);
    if (d>-1.0)
    {
        vec3 pos = camera+dir*d;
        vec3 normal = calc_norm(pos,delta);
        vec3 light_dir = normalize(pos-light_pos);
        float diff = dot(normal,light_dir);

        float shade = diff*shadow(pos, light_dir, 0.1, length(pos-light_pos));
        return color*shade;
    }
    vec3 sky = vec3(0.2,0.5,0.85);
    return sky;
}

void main()
{
    const float minDist = 0;
    const float maxDist = 100;
    const float delta = 0.001; //Checks distance from current pos to object

    vec3 sentCam = vec3(12,5,10);

    vec2 resolution = vec2(600,600); //Same as the window res
    vec3 viewDir = rayDirection(60, resolution, gl_FragCoord.xy);
    //gl_FragCoord contains the window-relative coordinates of the current fragment
    mat4 viewToWorld = lookAT(sentCam, vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
    vec3 worldDir = (viewToWorld * vec4(viewDir, 0.0)).xyz;
    outColor = vec4(rayMarch(sentCam,worldDir,minDist,maxDist,delta),1.0);
}
