#version 150

out vec4 outColor;

uniform float time;
uniform vec3 sentCam;

#define M_PI 3.14159265358979323846

//Distance function by Inigo Quilez
//http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float cubeDist(vec3 point, vec3 cube)
{
    vec3 d = abs(point)-cube;
    return min(max(d.x, max(d.y, d.z)), 0.0)+length(max(d, 0.0));
}

float map_sphere(vec3 point, vec3 c, float r)
{
    return length(point-c)-r;
}

float planeDist(vec3 p, float h)
{
    return p.y-h;
}

float map(vec3 p)
{
    float displacement = 0.3 * (sin(time+p.x)*sin(time+p.y)*sin(time+p.z));
    // float floater = cubeDist(p-vec3(0.0,1.5,0.0),vec3(0.5,0.5,0.5));
    // float floater = map_sphere(p,vec3(0.0,0.6,0.0),0.6);
    float plane = planeDist(p, 0.0);
    float bbox = cubeDist(p, vec3(3.0,1.0,3.0));

    return plane+displacement; //max(plane,bbox);
}

vec3 sphere_norm(vec3 p, float d, vec3 c, float r)
{
    // vec3 c = vec3(0.0,1.0,0.0);
    // float r = 0.5;
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

vec3 trace_reflect(vec3 origin, vec3 dir, float start, float end, vec3 c, float r)
{
    for (float d=start; d<end;)
    {
        float dist = map_sphere(origin+dir*d,c,r);
        if (dist<0.001) return vec3(0,0,1.0);
        d += dist;
    }
    return vec3(1.0);
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
//Perform the ray marching:
vec3 ray_march(vec3 camera, vec3 dir, float start, float end, float delta)
{
    float depth = start;
    // vec3 light_pos = vec3(10*sin(time),-15.0,-10*cos(time));
    vec3 light_pos = vec3(-10.0,-15.0,-10.0);
    vec3 color = vec3(1.0);
    vec3 c = vec3(0,0.0,0);
    float r = 0.5;
    vec3 sky = vec3(0.2,0.5,0.85);
    vec3 reflection = vec3(1.0);
    // float d = cast_ray(camera, dir,c,r);

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
            vec3 normal = water_norm(pos,delta);
            if (dist == dist_s) normal = sphere_norm(pos,delta,c,r);
            else {
                vec3 ref_dir = normalize(reflect(normal,camera));
                reflection = trace_reflect(pos, ref_dir, 0.1, length(pos-camera),c,r);
            }
            vec3 light_dir = normalize(pos-light_pos);
            float diff = dot(normal,light_dir);
            float shade = diff*shadow(pos, light_dir, 0.1, length(pos-light_pos),c,r);
            return color*reflection*diff;
        }
    }
    return sky;
}

void main()
{
    const float minDist = 0;
    const float maxDist = 100;
    const float delta = 0.001; //Checks distance from current pos to object

    // vec3 sentCam = vec3(12,5,10);

    vec2 resolution = vec2(600,600); //Same as the window res
    vec3 viewDir = ray_dir(60, resolution, gl_FragCoord.xy);
    //gl_FragCoord contains the window-relative coordinates of the current fragment
    mat4 viewToWorld = look_at(sentCam, vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
    vec3 worldDir = (viewToWorld * vec4(viewDir, 0.0)).xyz;
    outColor = vec4(ray_march(sentCam,worldDir,minDist,maxDist,delta),1.0);
}
