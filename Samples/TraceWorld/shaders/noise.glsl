#ifndef _NOISE_
#define _NOISE_

float hash(float n) { return fract(sin(n) * 1e4); }
float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float noise(float x) {
	float i = floor(x);
	float f = fract(x);
	float u = f * f * (3.0 - 2.0 * f);
	return mix(hash(i), hash(i + 1.0), u);
}

float noise(vec2 x) {
	vec2 i = floor(x);
	vec2 f = fract(x);

	// Four corners in 2D of a tile
	float a = hash(i);
	float b = hash(i + vec2(1.0, 0.0));
	float c = hash(i + vec2(0.0, 1.0));
	float d = hash(i + vec2(1.0, 1.0));

	// Simple 2D lerp using smoothstep envelope between the values.
	// return vec3(mix(mix(a, b, smoothstep(0.0, 1.0, f.x)),
	//			mix(c, d, smoothstep(0.0, 1.0, f.x)),
	//			smoothstep(0.0, 1.0, f.y)));

	// Same code, with the clamps in smoothstep and common subexpressions
	// optimized away.
	vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float octNoise(vec2 x, uint octaves) {
  float result = 0;
  for (uint i = 0; i < octaves; i++) {
    float factor = pow(2, float(i));
    result += noise(x * factor) / factor;
  }
  return result - 1.0;
}



///


vec3 random3(vec3 c) {
	float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
	vec3 r;
	r.z = fract(512.0*j);
	j *= .125;
	r.x = fract(512.0*j);
	j *= .125;
	r.y = fract(512.0*j);
	return r-0.5;
}


float snoise(vec3 p) {
  const float F3 =  0.3333333;
  const float G3 =  0.1666667;
	vec3 s = floor(p + dot(p, vec3(F3)));
	vec3 x = p - s + dot(s, vec3(G3));
	 
	vec3 e = step(vec3(0.0), x - x.yzx);
	vec3 i1 = e*(1.0 - e.zxy);
	vec3 i2 = 1.0 - e.zxy*(1.0 - e);
	 	
	vec3 x1 = x - i1 + G3;
	vec3 x2 = x - i2 + 2.0*G3;
	vec3 x3 = x - 1.0 + 3.0*G3;
	 
	vec4 w, d;
	 
	w.x = dot(x, x);
	w.y = dot(x1, x1);
	w.z = dot(x2, x2);
	w.w = dot(x3, x3);
	 
	w = max(0.6 - w, 0.0);
	 
	d.x = dot(random3(s), x);
	d.y = dot(random3(s + i1), x1);
	d.z = dot(random3(s + i2), x2);
	d.w = dot(random3(s + 1.0), x3);
	 
	w *= w;
	w *= w;
	d *= w;
	 
	return dot(d, vec4(52.0)) * 0.5 + 0.5;
}

float snoiseFractal(vec3 m) {
	return   0.5333333* snoise(m)
				+0.2666667* snoise(2.0*m)
				+0.1333333* snoise(4.0*m)
				+0.0666667* snoise(8.0*m);
}

float snoiseOctave(vec3 m, int steps) {
  
  float period = 0.5;
  float scale = 1.0;
  float sum = 0.0;
  
  for (int i = 0; i < steps; i++) {
    
    scale *= 0.5;
    period *= 2.0;
    sum += snoise(m * period) * scale;
    
  }
  
  return sum;
}

vec3 normalNoise(vec2 uv, float time) {
  
  vec3 me = vec3(0.0, snoiseFractal(vec3(uv, time)), 0.0);
  
  float step = 0.01;
 
  vec3 dx = vec3(step, snoiseFractal(vec3(uv + vec2(step, 0.0), time)), 0.0);
  
  vec3 dy = vec3(0.0, snoiseFractal(vec3(uv + vec2(0.0, step), time)), step);
  
  
  return normalize(cross(normalize(dy - me), normalize(dx - me)));
  
}


vec3 noiseVec(vec2 uv) {
  
  const vec2 skip = vec2(100.0, 100.0);
  
  return normalize(vec3(
      noise(uv),
      noise(uv + skip),
      noise(uv + skip + skip)
    ));

}

#endif