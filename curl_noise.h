#pragma once
#include "geometric.hpp"
#include "vec3.hpp"

glm::vec3 potential_occluder(
	glm::vec3 p,        // center of occluder
	glm::vec3 axis,     // axis to rotate x-axis to
	glm::vec3 radius,   // radii of ellipsoid occluder
	glm::vec3 phi,      // potential so far
	glm::vec3 x)        // point to evaluate
{
	x -= p;
	//vec4 q = quat_init_rot(axis, vec3(1, 0, 0));
	//x = quat_transform(x, q);
	x /= radius;
	glm::vec3 n = glm::normalize(x);
	float dist = glm::length(x);
	float alpha = glm::smoothstep(1.0f, 2.0f, dist);
	//phi = quat_transform(phi, q);
	glm::vec3 nphi = (1.0f - alpha) * n * glm::dot(n, phi) + phi * alpha;
	if (dist < 1.0f)
	{
		nphi = glm::vec3(0.0f);
	}
	//q = quat_inv(q);
	return nphi;//quat_transform(nphi, q);
}

glm::vec3 potential_vortex(
	float R,      // radius of influence
	glm::vec3 x_c,     // center point of vortex
	glm::vec3 omega_c, // angular velocity of vortex
	glm::vec3 x)
{
	// f(|x - x_c|/R) (R*R - ||x-x_c||^2)/2 omega_c
	// where f: smoothing kernel
	return glm::smoothstep(1.0f, 0.0f, glm::length(x - x_c) / R) * (R * R - glm::dot(x - x_c, x - x_c)) / 2.0f * omega_c;
}

glm::vec3 potential_vortex_ring(
	float R,       // radius of vortex around ring
	float r,       // radius of ring itself
	glm::vec3 c,        // center of ring
	glm::vec3 n,        // normal of ring
	glm::vec3 x)        // point to evaluate
{
	// f(|x - x_c|/R) (R*R - ||x-x_c||^2)/2 omega_c
	// where
	// f: smoothing kernel
	// omega_c: angular velocity (tangent to curve)
	glm::vec3 dist = x - c;
	dist -= n * glm::dot(dist, n);

	// closest point on circular curve
	glm::vec3 x_c = r * glm::normalize(dist);
	glm::vec3 omega_c = glm::cross(n, glm::normalize(x_c));
	return potential_vortex(R, c + x_c, omega_c, x);
}

glm::vec3 potential_field(glm::vec3 x)
{
	// rotation of downwash:
	glm::vec3 phi = potential_vortex(
		5.95f,           // radius
		glm::vec3(0,0,0),     // center
		glm::vec3(0, -0.5f, 0), // angular velocity
		x);
		// vortex ring of main rotor
	phi += potential_vortex_ring(
		7.0f,        // radius of vortex around ring
		5.95f,         // radius of ring itself
		glm::vec3(0, 0, 0), // center of ring
		glm::vec3(0, 1, 0), // normal of ring
		x);
	// second vortex ring
	/*phi += potential_vortex_ring(
		2.5f,        // max: 11.9/4 radius of vortex around ring
		2.5f,        // max: 11.9/4 radius of ring itself
		glm::vec3(0, 0, 0), // center of ring
		glm::vec3(0, -1, 0), // normal of ring
		x);*/
		// main rotor
		// vec3 phi = potential_propeller(vec3(0, 0, 0), vec3(0, 1, 0), 10.0, 7.0, 4, x);
		// tail rotor
		// phi += potential_propeller(vec3(0, 0, -6.5), normalize(vec3(-1, 0.2, 0)), 0.5, 3.0, 3, x);
		// fuselage
	phi = potential_occluder(glm::vec3(0, -1.6f, 0), glm::vec3(1, 0, 0), glm::vec3(1, 1.6f, 4.5f), phi, x);
	return phi;
#if 0
	// float dist = exp(-dot(x.xz,x.xz)/25.0);
	// return  - x.x * vec3(0, 0, 1) +  2*dist * x.y;

	float dist = exp(-dot(x.xz, x.xz) / 25.0); // spiral profile: most velocity in ring around center
	return  -2 * x.x * vec3(0, 0, 1)  // go down (y direction)
		- vec3(0, 1, 0) * 10 * dist;       // spiral around y direction (ccw)
#endif
#if 0
  // XXX TODO: develop occluder and propeller
  // strength modulates in x and z directions:
	float sz = 1;//0.1 + 0.4*(1+cos(x.z / 6.5 * 3.1));
	float sx = 1.0 * (0.1 + 0.4 * (1 - cos(x.x / 6.5 * 6.14)));
	vec3 phi = sz * sx - x.x * vec3(0, 0, 1);
	vec3 n = normalize(x);
	float dist = length(x.x);
	float alpha = smoothstep(0.0f, 3.0f, dist);
	return mix(-n * dot(n, phi), phi, alpha);
	// go down:
	vec3 vert = 0.3 * vec3(0, -0.1 - (x.y + 4), 0);
	// radially move to the outside
	vec3 side = 0.005 * vec3(x.x, 0.0, x.z);
	// interpolate alongside downwash: t=0 is top, t>0 goes towards ground
	float t = 3 - x.y;
	return sx * sz * vert + t * t * side;
#endif
}

void potential_deriv(
	glm::vec3 x,
	glm::vec3* dpdx,
	glm::vec3* dpdy,
	glm::vec3* dpdz)
{
	glm::vec3 c = potential_field(x);
	*dpdx = (c - potential_field(x + glm::vec3(0.0001f, 0, 0))) / 0.0001f;
	*dpdy = (c - potential_field(x + glm::vec3(0, 0.0001f, 0))) / 0.0001f;
	*dpdz = (c - potential_field(x + glm::vec3(0, 0, 0.0001f))) / 0.0001f;
}

// compute divergence free noise by using curl (grad x)
glm::vec3 velocity_field(glm::vec3 x)
{
	glm::vec3 dpdx, dpdy, dpdz;
	potential_deriv(x, &dpdx, &dpdy, &dpdz);
	glm::vec3 result = glm::vec3(
		dpdy.z - dpdz.y,
		dpdz.x - dpdx.z,
		dpdx.y - dpdy.x);
	return result;
}
namespace util
{
	// these work for float, double, int ;)
#define crosss(v1, v2, res) \
  (res)[0] = (v1)[1]*(v2)[2] - (v2)[1]*(v1)[2];\
  (res)[1] = (v1)[2]*(v2)[0] - (v2)[2]*(v1)[0];\
  (res)[2] = (v1)[0]*(v2)[1] - (v2)[0]*(v1)[1]

#define dot(u, v) ((u)[0]*(v)[0] + (u)[1]*(v)[1] + (u)[2]*(v)[2])

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(a, m, M) MIN(MAX(a, m), M)

	static inline void copy(float a[], float b[])
	{
		b[1] = a[1];
		b[2] = a[2];
		b[3] = a[3];
	}

	static inline void subtract(float a[], float b[], float c[])
	{
		c[1] = a[1] - b[1];
		c[2] = a[2] - b[2];
		c[3] = a[3] - b[3];
	}

	static inline void add(float a[], float b[], float c[])
	{
		c[1] = a[1] + b[1];
		c[2] = a[2] + b[2];
		c[3] = a[3] + b[3];
	}

	static inline void divide(float a[], float b[], float c[])
	{
		c[1] = a[1] / b[1];
		c[2] = a[2] / b[2];
		c[3] = a[3] / b[3];
	}

	static inline void multiply(float a[], float b[], float c[])
	{
		c[1] = a[1] * b[1];
		c[2] = a[2] * b[2];
		c[3] = a[3] * b[3];
	}

	static inline void multiplyNumber(float a[], float b, float c[])
	{
		c[1] = a[1] * b;
		c[2] = a[2] * b;
		c[3] = a[3] * b;
	}

	static inline float length(float a[])
	{
		return sqrt(pow(a[0], 2) + pow(a[1], 2) + pow(a[2], 2));
	}

	static inline float clamp(float x, float min, float max)
	{
		if (x > min)
		{
			if (x < max)
			{
				return x;
			}
			else
			{
				return max;
			}
		}
		else
		{
			{
				return min;
			}
		}
	}

	static inline float smoothstep(float edge0, float edge1, float x)
	{
		float t = CLAMP((x - edge0) / (edge1 - edge0), 0.0, 1.0);
		return t * t * (3.0 - 2.0 * t);
	}

	static inline void mix(float x[], float y[], float a, float resulting[])
	{
		resulting[0] = (1 - a) * x[0] + a * y[0];
		resulting[1] = (1 - a) * x[1] + a * y[1];
		resulting[2] = (1 - a) * x[2] + a * y[2];
	}

	static inline void normalise(float* f)
	{
		const float len = 1.0f / sqrtf(dot(f, f));
		for (int k = 0; k < 3; k++) f[k] *= len;
	}
}

using namespace util;

inline void potential_occluder(
	float p[],        // center of occluder
	float radius[],   // radii of ellipsoid occluder
	float phi[],      // potential so far
	float x[],
	float nphi[])
{
	float local_x[3] = { x[0], x[1], x[2] };
	for (int k = 0; k < 3; k++) local_x[k] -= p[k];
	for (int k = 0; k < 3; k++) local_x[k] /= radius[k];
	float n[3] = { x[0], x[1], x[2] };
	normalise(n);
	float dist = length(local_x);
	float alpha = smoothstep(1.0f, 1.5f, dist);
	float dot = dot(n, phi);
	for (int k = 0; k < 3; k++) nphi[k] = (1.0f - alpha) * n[k] * dot + phi[k] * alpha;
	if (dist < 1.0f)
	{
		for (int k = 0; k < 3; k++) nphi[k] = 0.0f;
	}
}

inline void potential_vortex(
	float R,      // radius of influence
	float x_c[],     // center point of vortex
	float omega_c[], // angular velocity of vortex
	float x[],
	float vec[])
{
	// f(|x - x_c|/R) (R*R - ||x-x_c||^2)/2 omega_c
	// where f: smoothing kernel
	float dist[3] = { 0.0f, 0.0f, 0.0f };
	for (int k = 0; k < 3; k++) dist[k] = x[k] - x_c[k];
	float s = CLAMP((1.0f - length(dist) / R), 0.0f, 1.0f) * (R * R - dot(dist, dist)) / 2.0;
	for (int k = 0; k < 3; k++) vec[k] = omega_c[k] * s;
}

inline void potential_vortex_ring(
	float R,       // radius of vortex around ring
	float r,       // radius of ring itself
	float c[],        // center of ring
	float n[],        // normal of ring
	float x[],      // point to evaluate
	float vec[])
{
	// f(|x - x_c|/R) (R*R - ||x-x_c||^2)/2 omega_c
	// where
	// f: smoothing kernel
	// omega_c: angular velocity (tangent to curve)
	
	float dist[3] = { 0.0f, 0.0f, 0.0f };
	for (int k = 0; k < 3; k++) dist[k] = x[k] - c[k];
	float dot = dot(dist, n);
	float n_dot[3] = { n[0], n[1], n[2] };
	//printf("DOT: %f\n", dot);
	for (int k = 0; k < 3; k++) n_dot[k] *= dot;
	for (int k = 0; k < 3; k++) dist[k] -= n_dot[k];
	// closest point on circular curve
	float x_c[3] = { 0.0f, 0.0f, 0.0f };
	normalise(dist);
	for (int k = 0; k < 3; k++) x_c[k] = dist[k] * r;
	float omega_c[3] = { 0.0f, 0.0f, 0.0f };
	crosss(n, dist, omega_c);
	for (int k = 0; k < 3; k++) x_c[k] += c[k];
	for (int k = 0; k < 3; k++) omega_c[k] *= 2.0f;
	potential_vortex(R, x_c, omega_c, x, vec);
}

inline void potential_field(float x[], float potential[], float center[], float radius)
{
	float radius_function = -abs(radius - 5.95f) + 5.95f;
	float av[] = { 0.0f, -0.5f, 0.0f }; // angular velocity
	// rotation of downwash:
	float phi[3] = { 0.0f, 0.0f, 0.0f };
	potential_vortex(
		5.95f,           // radius
		center,            // center
		av, // angular velocity
		x,
		phi);
	//for (int k = 0; k < 3; k++) phi[k] = 0.0f; // removing downwash rotation for debugging
	float axis[3] = { 0.0f, 1.0f, 0.0f };
	// vortex ring of main rotor
	float vec[3] = { 0.0f, 0.0f, 0.0f };
	potential_vortex_ring(
		radius_function,        // radius of vortex around ring
		5.95f,         // radius of ring itself
		center, // center of ring
		axis, // normal of ring
		x,
		vec);
	for (int k = 0; k < 3; k++) phi[k] += vec[k];
	// second vortex ring, starts after first vortex ring covers the whole main rotor
	if (radius > 5.95f)
	{
		axis[1] = -1.0f;
		potential_vortex_ring(
			5.95f - radius_function,        // max: 11.9/4 radius of vortex around ring
			5.95f - radius_function,        // max: 11.9/4 radius of ring itself
			center, // center of ring
			axis, // normal of ring
			x,
			vec);
		for (int k = 0; k < 3; k++) phi[k] += vec[k];
	}

	// main rotor
	// vec3 phi = potential_propeller(vec3(0, 0, 0), vec3(0, 1, 0), 10.0, 7.0, 4, x);
	// tail rotor
	// phi += potential_propeller(vec3(0, 0, -6.5), normalize(vec3(-1, 0.2, 0)), 0.5, 3.0, 3, x);
	// fuselage
	float com[3] = { 0.0f, -1.6f, 0.0f }; // center of mass
	float occluder_radius[3] = { 1.0f, 1.6f, 4.5f };
	potential_occluder(com, occluder_radius, phi, x, vec);
	for (int k = 0; k < 3; k++) phi[k] = vec[k];
	for (int k = 0; k < 3; k++) potential[k] = phi[k];
}

inline void potential_deriv(
	float center[],
	float radius,
	float x[],
	float dpdx[],
	float dpdy[],
	float dpdz[])
{
	float eps = 1e-4f;
	float c[3] = { 0.0f, 0.0f, 0.0f };
	float vec[3] = { 0.0f, 0.0f, 0.0f };
	float delta[3] = { eps, 0.0f, 0.0f };
	float potential[3] = { 0.0f, 0.0f, 0.0f };
	potential_field(x, c, center, radius);
	for (int k = 0; k < 3; k++) vec[k] = x[k] + delta[k];
	potential_field(vec, potential, center, radius);
	for (int k = 0; k < 3; k++) dpdx[k] = c[k] - potential[k];
	for (int k = 0; k < 3; k++) dpdx[k] /= eps;
	delta[1] = eps;
	delta[0] = 0.0f;
	for (int k = 0; k < 3; k++) vec[k] = x[k] + delta[k];
	potential_field(vec, potential, center, radius);
	for (int k = 0; k < 3; k++) dpdy[k] = c[k] - potential[k];
	for (int k = 0; k < 3; k++) dpdy[k] /= eps;
	delta[2] = eps;
	delta[1] = 0.0f;
	for (int k = 0; k < 3; k++) vec[k] = x[k] + delta[k];
	potential_field(vec, potential, center, radius);
	for (int k = 0; k < 3; k++) dpdz[k] = c[k] - potential[k];
	for (int k = 0; k < 3; k++) dpdz[k] /= eps;
}

// compute divergence free noise by using curl (grad x)
inline void velocity_field(float x[], float vec[], float center[], float radius)
{
	float dpdx[3] = { 0.0f, 0.0f, 0.0f };
	float dpdy[3] = { 0.0f, 0.0f, 0.0f };
	float dpdz[3] = { 0.0f, 0.0f, 0.0f };
	potential_deriv(center, radius, x, dpdx, dpdy, dpdz);
	vec[0] = dpdy[2] - dpdz[1];
	vec[1] = dpdz[0] - dpdx[2];
	vec[2] = dpdx[1] - dpdy[0];
}