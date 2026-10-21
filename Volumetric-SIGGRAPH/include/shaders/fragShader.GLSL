#version 330 core
out vec4 FragColor;

uniform vec2 u_resolution;
uniform vec3 u_sphere_center;
uniform float u_radius;
uniform float u_sigma_a;
uniform vec3 u_scatter;

bool intersectSphere(vec3 ro, vec3 rd, vec3 sphere_center, float radius, out float t0, out float t1) {
    vec3 oc = ro - sphere_center; 

    float a = dot(rd, rd);
    float b = 2.0 * dot(rd, oc); 
    float c = dot(oc, oc) - radius * radius;

    float disc = b * b - 4.0 * a * c; 
    if (disc < 0.0) return false; 
    float sqrtDisc = sqrt(disc); 
    
    t0 = (-b - sqrtDisc) / (2.0 * a);
    t1 = (-b + sqrtDisc) / (2.0 * a);
    
    if (t0 < 0.0) t0 = t1; 
    return t0 >= 0.0; 
}

vec3 traceScene(vec3 ro, vec3 rd) {    
    vec3 background_color = vec3(0.5, 0.8, 0.9);

    float t0, t1;
    if (intersectSphere(ro, rd, u_sphere_center, u_radius, t0, t1)) 
    { 
        vec3 p1 = ro + rd * t0; 
        vec3 p2 = ro + rd * t1; 
        float dist = length(p2 - p1); 
        float transmission = exp(-dist * u_sigma_a); 
        return background_color * transmission + u_scatter * (1.0 - transmission); 
    }
    else 
    { 
        return background_color; 
    }
}

void main() {
    //uv
    vec2 fragCoord = gl_FragCoord.xy;
    float x = (2.0 * (fragCoord.x + 0.5) / u_resolution.x - 1.0) * (u_resolution.x / u_resolution.y);
    float y = (1.0 - 2.0 * (fragCoord.y + 0.5) / u_resolution.y);

    vec3 ro = vec3(0.0, 0.0, 0.0);
    vec3 rd = normalize(vec3(x, y, -1.0));

    vec3 color = traceScene(ro, rd);
    FragColor = vec4(color, 1.0);
}
