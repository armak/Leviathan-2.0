#version 130
uniform int m;
out vec4 o;
float PI = 3.1416;
float t = m/float(44100);
float rep = 2.0;
float dist = 18.0;
float scene = 0;
float brg = 1.48;
float ap1 = 23.0;
float ap2 = 35.0;
float hash(float c){return fract(sin(dot(c, 12.9898)) * 43758.5453);}
mat3 rx(float a){return mat3(1.0,0.0,0.0,0.0,cos(a),-sin(a),0.0,sin(a),cos(a));}
mat3 ry(float a){return mat3(cos(a),0.0,sin(a),0.0,1.0,0.0,-sin(a),0.0,cos(a));}
mat3 rz(float a){return mat3(cos(a),-sin(a),0.0,sin(a),cos(a),0.0,0.0,0.0,1.0);}
float box(vec3 p, vec3 b)
{
	return max(max(abs(p.x)-b.x,abs(p.y)-b.y),abs(p.z)-b.z);
}
float modp(inout vec2 p, float rep) {
	float angle = 2*PI/rep;
	float a = atan(p.y, p.x) + angle/2;
	float c = floor(a/angle);
	a = mod(a,angle) - angle/2.;
	p = vec2(cos(a), sin(a))*length(p);
	if (abs(c) >= (rep/2)) c = abs(c);
	return c;
}
vec2 mod2(inout vec2 p, vec2 size) {
	vec2 c = floor((p + size*0.5)/size);
	p = mod(p + size*0.5,size) - size*0.5;
	return c;
}
float sp(vec3 p, float r)
{
	return length(p)-r;
}
float caps(vec3 p, float r, float c){
	return mix(length(p.xz) - r, length(vec3(p.x, abs(p.y) - c, p.z)) - r, step(c, abs(p.y)));
}
float map(vec3 p)
{
	if( scene < 1 ){
		float d = (p.y);
		vec2 r = mod2(p.zx, vec2(dist));
		modp(p.zx, rep);
		d = min(d, box((p-vec3(0.,1,9)), vec3(9,9,0.2)) );
		d = min(d, box((p-vec3(-6,1,0)), vec3(0.2,9.,9)) );
		d = min(d, box((p-vec3(6,1,-9)), vec3(0.2,9.,9)) );
		d = max(d, -box((p-vec3(0,4,0)), vec3(1.5,4.,10)));
		return d;
	}
	else
	{
		p += vec3(-70,0,0);
		vec3 q = p;
		modp(p.xz, 4.0);
		vec3 o = p;
		mod2(o.yx, vec2(6,9));
		vec3 r = p;
		float d = caps(p-vec3(rep,-3,0), 44.0, dist);
		mod2(r.xz, vec2(12.7));
		float d2 = box((o-vec3(0.,1,0)), vec3(9,1.5,ap1));
		d2 = max(d2, p.y-28.0);
		d = max(d, -d2);
		d = min(d, box((r-vec3(0.,8,0)), vec3(.6,9,.6)));
		d = max(d, -caps(p-vec3(rep,-3,0), 24.0, 15.0));
		float d6 = sp(p-vec3(rep,40,0), 30.0 );
		d = max(d, -d6);
		float d3 = caps(q-vec3(0,-3,0), 55.0, 0.0);
		d3 = max(d3, -caps(q-vec3(0,-3,0), ap2, 15.0));
		d3 = max(d3, -sp(q-vec3(0,40,0), 30.0 ));
		d = max(d, d3);
		return min(d, p.y);
	}
}
vec3 rhs(vec3 dir, float i)
{
	vec2 rnd = vec2(hash(i+1.), hash(i+2.));
	float s = rnd.x*PI*2.;
	float t = rnd.y*2.-1.;
	vec3 v = vec3(sin(s), cos(s), t) / sqrt(1.0 + t * t);
	return v * sign(dot(v, dir));
}
float ao( vec3 p, vec3 n, float maxDist, float falloff)
{
	float ao = 0.0;
	for( int i=0; i<10; i++ )
	{
		float l = hash(float(i))*maxDist;
		vec3 rd = normalize(n+rhs(n, l )*0.95)*l;
		ao += (l - map( p + rd )) / pow(1.+l, falloff);
	}
	return clamp(1.-ao*0.1,0.0,999.0);
}
vec3 shade( vec3 p, vec3 n, vec3 org, vec3 dir, vec2 v )
{
	return vec3(0.8)*sqrt(mix(ao(p,n, 8., 0.97), ao(p,n, 2., 0.9), 0.4));
}
vec3 normal( vec3 p )
{
	vec3 eps = vec3(0.001, 0.0, 0.0);
	return normalize( vec3(
		map(p+eps.xyy)-map(p-eps.xyy),
		map(p+eps.yxy)-map(p-eps.yxy),
		map(p+eps.yyx)-map(p-eps.yyx)
	));
}
vec3 mr( vec3 ro, vec3 rd, vec2 nfplane, out float f)
{
	vec3 p = ro+rd*nfplane.x;
	float t = 0.;
	for(int i=0; i<40; i++)
	{
		float d = map(p);
		t += d;
		p += rd*d;
		if( d < 0.01 || t > nfplane.y )
			break;
	}
	f = 0.04*sqrt(nfplane.y/max(9.0, distance(ro, p)));
	return p;
}
vec3 rm( vec3 ro, vec3 rd, vec2 nfplane )
{
	vec3 p = ro+rd*nfplane.x;
	float t = 0.;
	for(int i=0; i<80; i++)
	{
		float d = map(p);
		t += d;
		p += rd*d;
		if( d < 0.01 || t > nfplane.y )
			break;   
	}
	return p;
}
vec3 cc(vec3 c)
{
	return 0.08+0.94*pow(-0.5+1.7*pow(c,vec3(1.6)), vec3(0.6));
}
void main()
{
	vec2 res = vec2(1280,720);
	vec2 q = gl_FragCoord.xy/res.xy;
	vec2 v = -1.0+2.0*q;
	v.x *= res.x/res.y;
	vec3 ro = vec3(0);
	vec3 rd = vec3(0);
	if(t < 4){
		o = vec4(0.035*hash(length(q)*t));
		return;
	} else if(t < 9.0) {
		ro = vec3( 90.0+t,90.0,-130.0+t );
		rd = normalize( vec3(v.x, v.y, 8.0))*rx(1.)*ry(.5);
		rep = 6.0;
		dist = 13.9;
	} else if(t < 16){
		ro = vec3( 90.0,90.0,-130.0+t );
		rd = normalize( vec3(v.x, v.y, 8.0))*rx(1.)*ry(.5);
		rep = 8.0;
		dist = 16.0;
	} else if(t < 26){
		ro = vec3( 90.0+t,90.0,-130.0+t*0.1 );
		rd = normalize( vec3(v.x, v.y, 8.0))*rx(1.)*ry(.5);
	}
	else if(t < 32){
		ro = vec3( 90.0+t,90.0,-130.0 );
		rd = normalize( vec3(v.x, v.y, 5.0))*rx(PI/2.);
	}
	else if(t < 35){
		ro = vec3( 90.0+t,90.0,-130.0+t );
		rd = normalize( vec3(v.x, v.y, 8.0))*rx(1.)*ry(.5);
		rep = 6.0;
		dist = 11.9;
	}
	else if(t < 44){
		ro = vec3( 90.0-t*0.1,90.0,-130.0+t );
		rd = normalize( vec3(v.x, v.y, 8.0))*rx(1.)*ry(.5);
		rep = 6.0;
		brg = 2.48-smoothstep(33,41,t);
	}
	else if(t < 49.5){
		ro = vec3( 90.0+t*.5,40.0,-130.0+t*2 );
		rd = normalize( vec3(v.x, v.y, 4.0))*rx(.5)*ry(-.2);
		rep = 6.0;
		dist = 21.0;
		brg = 1.6;
	}
	else if(t < 58){
		ro = vec3( 90.0+t,90.0,-130.0+t*0.1 );
		rd = normalize( vec3(v.x, v.y, 8.0))*rx(1.)*ry(.5);
		rep = 8.0;
	}
	else if(t < 69){
		ro = vec3(t*.5,90.0,-t*.5 );
		rd = normalize( vec3(v.x, v.y, 5.0))*rx(PI/2.)*ry(t*.01);
		rep = 6.0;
		dist = 16.0 - 3.0*smoothstep(21,30,t*.4);
	}
	else if(t < 75){
		float tt = t-69;
		ro = vec3( 0,290+tt*2,-tt*3);
		rd = normalize( vec3(v.x, v.y, 8.0) )*ry(1.2)*rx(1.5-0.01*tt)*rz(-1);
		rep = 0.0;
		dist = -26+43*smoothstep(70,82,t);
		scene = 1;
		brg = 1.4;
	}
	else if(t < 81){
		float tt = t-75;
		ro = vec3( 90.0-tt*3,210-tt,160-tt);
		rd = normalize( vec3(v.x, v.y, 5.0) )*ry(3.1-(t-73)*0.01)*rx(-.9);
		rep = 0.0;
		dist = -26+43*smoothstep(70,82,t);
		scene = 1;
	}
	else if(t < 98){
		float tt = t-80;
		ro = vec3( t,290-t,240-t);
		rd = normalize( vec3(v.x, v.y, 5.0) )*ry(3.1+tt*0.003)*rx(-.9);
		rep = 60.0;
		dist = 17.0;
		ap2 = 58-19*smoothstep(75,95,t)-12*smoothstep(85,96,t);
		ap1 = 9+19*smoothstep(80,95,t);
		scene = 1;
	}
	else if(t < 105){
		ro = vec3( 90.0+t,90.0,-130.0+t );
		rd = normalize( vec3(v.x, v.y, 8.0))*rx(1.)*ry(.5);
		rep = 6.0;
		dist = 11.9;
	}
	else if(t < 190){
		ro = vec3( 90.0+t,90.0,-130.0+t*0.1 );
		rd = normalize( vec3(v.x, v.y, 8.0))*rx(1.)*ry(.5);
		brg = 1.48+1.5*smoothstep(111,140,t);
	}
	vec3 p = rm(ro, rd, vec2(1., 545.) );
	vec3 n = normal(p.xyz);
	vec3 col = shade(p, n, ro, rd, q);
	vec3 rp = vec3(0);
	float tile = 1.0;//ceil(sin(p.x*0.9)*cos(p.z*0.05));
	if(p.y < 26.0){
		// uhh, stuff...
		//normalize(n+(1.0-tile)*0.1*vec3(hash(dot(p,p)),hash(length(p)),hash(dot(p,-p)) ))
		float f;
		rp = mr(p, reflect(rd, n), vec2(0.1, 300.), f );
		vec3 rn = normal(rp.xyz);
		col += shade(rp, rn, ro, rd, q)*f; 
		col *= 0.8;
	} else {
		col = mix(col,0.5+0.5*vec3(col*dot(n, normalize(p-vec3(99,-99,0)))), 0.3);
	}
	col = pow(col*brg/sqrt(2.+dot(v*0.3,v*0.3)), vec3(1./2.2));
	o = vec4(cc(col-0.035*hash(length(q)*t)), p.y<0.1?tile*distance(p, rp)/10.0:0.0 );
}