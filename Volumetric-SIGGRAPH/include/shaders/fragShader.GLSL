#version 330 core
out vec4 FragColor;

uniform vec2 u_resolution;
uniform vec3 u_sphere_center;
uniform float u_radius;
uniform float u_sigma_a;
uniform vec3 u_scatter;
uniform vec3 u_background_color;
uniform vec3 u_light_color;
uniform vec3 u_light_position;

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
    float t0, t1;
    if (!intersectSphere(ro, rd, u_sphere_center, u_radius, t0, t1)) 
    { 
        return u_background_color; 
    }

    int nSteps = 24;
    float stepSize = (t1 - t0) / float(nSteps);

    float transparency = 1.0;
    vec3 result = vec3(0.0); 

    for (int i = 0; i < nSteps; ++i) {
        float t = t0 + (float(i) + 0.5) * stepSize; 
        
        vec3 samplePos = ro + rd * t;

        float sample_attenuation = exp(-stepSize * u_sigma_a);
        transparency *= sample_attenuation;

        vec3 light_dir = normalize(u_light_position - samplePos);
    
        float lt0, lt1;
        if (intersectSphere(samplePos, light_dir, u_sphere_center, u_radius, lt0, lt1)) {
            float light_attenuation = exp(-lt1 * u_sigma_a);
            result += transparency * u_light_color * light_attenuation * stepSize;
        }
    }
    return u_background_color * transparency + result;
}

void main() {
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    vec3 ro = vec3(0.0, 0.0, 0.0);
    vec3 rd = normalize(vec3(uv, -1.0));

    vec3 color = traceScene(ro, rd);
    FragColor = vec4(color, 1.0);
}
