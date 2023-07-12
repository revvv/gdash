<?xml version="1.0" encoding="UTF-8"?>
<!--
/*
   CRT - Trinitron Shader
   
   Copyright (C) 2019 guest(r) - guest.r@gmail.com

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
-->
<shader language="GLSL">


    <vertex><![CDATA[
        uniform vec2 rubyTextureSize;

        void main()
        {
                gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                gl_TexCoord[0] = gl_MultiTexCoord0;
        }
    ]]></vertex>


	
<fragment outscale_x = "1.0"  outscale_y = "1.0" filter="nearest"><![CDATA[
	
uniform sampler2D rubyTexture;
uniform vec2 rubyTextureSize;

#define brightboost  1.40     // adjust brightness
#define saturation   1.1      // 1.0 is normal saturation
#define scanline     8.0      // scanline param, vertical sharpness
#define beam_min     1.00     // dark area beam min - narrow
#define beam_max     1.00     // bright area beam max - wide
#define h_sharp      2.25     // pixel sharpness
#define bloompix     0.50     // glow shape, more is harder
#define bloompixy    0.80     // glow shape, more is harder
#define glow         0.10     // glow ammount
#define mcut         0.15     // Mask 5&6 cutoff
#define maskDark     0.60     // Dark "Phosphor"
#define maskLight    1.40     // Light "Phosphor"


#define eps 1e-8

	
vec3 sw(float x, vec3 color)
{
	vec3 tmp = mix(vec3(2.75*beam_min),vec3(beam_max), color);
	tmp = mix(vec3(beam_max), tmp, pow(vec3(x), color+0.3));
	vec3 ex = vec3(x)*tmp;
	return exp2(-scanline*ex*ex)/(0.65 + 0.35*color);
}

vec3 Mask(vec2 pos, vec3 c)
{
	vec3 mask = vec3(maskDark, maskDark, maskDark);
	
	float mx = max(max(c.r,c.g),c.b);
	vec3 maskTmp = vec3( min( 1.33*max(mx-mcut,0.0)/(1.0-mcut) ,maskDark));
	float adj = maskLight - 0.4*(maskLight - 1.0)*mx + 0.75*(1.0-mx)*(1.0+0.4*mcut);	
	mask = maskTmp;
	pos.x = fract(pos.x/3.0);
	if      (pos.x < 0.333) mask.r = adj;
	else if (pos.x < 0.666) mask.g = adj;
	else                    mask.b = adj;
	
	return mask;
}

void main()
{
	// Calculating texel coordinates

	vec2 size     = rubyTextureSize;
	vec2 inv_size = 1.0/rubyTextureSize;
	
	vec2 OGL2Pos = gl_TexCoord[0].xy * size  - vec2(0.5,0.5);
	vec2 fp = fract(OGL2Pos);
	vec2 dx = vec2(inv_size.x,0.0);
	vec2 dy = vec2(0.0, inv_size.y);
	vec2 g1 = vec2(inv_size.x,inv_size.y);
	
	vec2 pC4 = floor(OGL2Pos) * inv_size + 0.5*inv_size;	
	
	// Reading the texels
	vec3 ul = texture2D(rubyTexture, pC4     ).xyz; ul*=ul;
	vec3 ur = texture2D(rubyTexture, pC4 + dx).xyz; ur*=ur;
	vec3 dl = texture2D(rubyTexture, pC4 + dy).xyz; dl*=dl;
	vec3 dr = texture2D(rubyTexture, pC4 + g1).xyz; dr*=dr;
	
	float lx = fp.x;        lx = pow(lx, h_sharp);
	float rx = 1.0 - fp.x;  rx = pow(rx, h_sharp);
	
	vec3 color1 = (ur*lx + ul*rx)/(lx+rx);
	vec3 color2 = (dr*lx + dl*rx)/(lx+rx);

// calculating scanlines
	
	float f = fp.y;

	vec3 w1 = sw(f,color1);
	vec3 w2 = sw(1.0-f,color2);
	
	vec3 color = color1*w1 + color2*w2;
	vec3 ctemp = color / (w1 + w2);
	
	color*=brightboost;
	color = min(color, 1.0);
	
	color = pow(color, vec3(1.2));
	color*=Mask(gl_FragCoord.xy, sqrt(ctemp));
	color = pow(color, vec3(1.0/1.2));
	
	vec2 x2 = 2.0*dx; vec2 x3 = 3.0*dx;
	vec2 y2 = 2.0*dy;

	float wl3 = 2.0 + fp.x; wl3*=wl3; wl3 = exp2(-bloompix*wl3);
	float wl2 = 1.0 + fp.x; wl2*=wl2; wl2 = exp2(-bloompix*wl2);
	float wl1 =       fp.x; wl1*=wl1; wl1 = exp2(-bloompix*wl1);
	float wr1 = 1.0 - fp.x; wr1*=wr1; wr1 = exp2(-bloompix*wr1);
	float wr2 = 2.0 - fp.x; wr2*=wr2; wr2 = exp2(-bloompix*wr2);
	float wr3 = 3.0 - fp.x; wr3*=wr3; wr3 = exp2(-bloompix*wr3);	
	
	float wt = 1.0/(wl3+wl2+wl1+wr1+wr2+wr3);
	
	vec3 l3 = texture2D(rubyTexture, pC4 -x2 ).xyz; l3*=l3;
	vec3 l2 = texture2D(rubyTexture, pC4 -dx ).xyz; l2*=l2;
	vec3 l1 = texture2D(rubyTexture, pC4     ).xyz; l1*=l1;
	vec3 r1 = texture2D(rubyTexture, pC4 +dx ).xyz; r1*=r1;
	vec3 r2 = texture2D(rubyTexture, pC4 +x2 ).xyz; r2*=r2;
	vec3 r3 = texture2D(rubyTexture, pC4 +x3 ).xyz; r3*=r3;

	vec3 t1 = (l3*wl3 + l2*wl2 + l1*wl1 + r1*wr1 + r2*wr2 + r3*wr3)*wt;
	
	l3 = texture2D(rubyTexture, pC4 -x2 -dy).xyz; l3*=l3;
	l2 = texture2D(rubyTexture, pC4 -dx -dy).xyz; l2*=l2;
	l1 = texture2D(rubyTexture, pC4     -dy).xyz; l1*=l1;
	r1 = texture2D(rubyTexture, pC4 +dx -dy).xyz; r1*=r1;
	r2 = texture2D(rubyTexture, pC4 +x2 -dy).xyz; r2*=r2;
	r3 = texture2D(rubyTexture, pC4 +x3 -dy).xyz; r3*=r3;
	
	vec3 t2 = (l3*wl3 + l2*wl2 + l1*wl1 + r1*wr1 + r2*wr2 + r3*wr3)*wt;	
	
	l3 = texture2D(rubyTexture, pC4 -x2 +dy).xyz; l3*=l3;
	l2 = texture2D(rubyTexture, pC4 -dx +dy).xyz; l2*=l2;
	l1 = texture2D(rubyTexture, pC4     +dy).xyz; l1*=l1;
	r1 = texture2D(rubyTexture, pC4 +dx +dy).xyz; r1*=r1;
	r2 = texture2D(rubyTexture, pC4 +x2 +dy).xyz; r2*=r2;
	r3 = texture2D(rubyTexture, pC4 +x3 +dy).xyz; r3*=r3;

	vec3 b1 = (l3*wl3 + l2*wl2 + l1*wl1 + r1*wr1 + r2*wr2 + r3*wr3)*wt;

	l3 = texture2D(rubyTexture, pC4 -x2 +y2).xyz; l3*=l3;
	l2 = texture2D(rubyTexture, pC4 -dx +y2).xyz; l2*=l2;
	l1 = texture2D(rubyTexture, pC4     +y2).xyz; l1*=l1;
	r1 = texture2D(rubyTexture, pC4 +dx +y2).xyz; r1*=r1;
	r2 = texture2D(rubyTexture, pC4 +x2 +y2).xyz; r2*=r2;
	r3 = texture2D(rubyTexture, pC4 +x3 +y2).xyz; r3*=r3;
	
	vec3 b2 = (l3*wl3 + l2*wl2 + l1*wl1 + r1*wr1 + r2*wr2 + r3*wr3)*wt;	
	
	wl2 = 1.0 + fp.y; wl2*=wl2; wl2 = exp2(-bloompixy*wl2);
	wl1 =       fp.y; wl1*=wl1; wl1 = exp2(-bloompixy*wl1);
	wr1 = 1.0 - fp.y; wr1*=wr1; wr1 = exp2(-bloompixy*wr1);
	wr2 = 2.0 - fp.y; wr2*=wr2; wr2 = exp2(-bloompixy*wr2);
	
	wt = 1.0/(wl2+wl1+wr1+wr2);	
	
	vec3 Bloom = (t2*wl2 + t1*wl1 + b1*wr1 + b2*wr2)*wt;

	color += Bloom*glow; 
	
	color = min(color, 1.0);

	color = pow(color, vec3(0.5));
	
	float l = length(color);
	color = normalize(pow(color + vec3(eps,eps,eps), vec3(saturation,saturation,saturation)))*l;

	gl_FragColor = vec4(color,1.0);
	}
	
]]></fragment>
	
</shader>
