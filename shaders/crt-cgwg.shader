<?xml version="1.0" encoding="UTF-8"?>

<shader language="GLSL">

<vertex><![CDATA[
        uniform vec2 rubyTextureSize;

        void main()
        {
                gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                gl_TexCoord[0] = gl_MultiTexCoord0;
        }
]]></vertex>

/*
    CRT (Geom, cgwg) shader

    Copyright (C) 2010-2012 cgwg, Themaister and DOLLS

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.

    (cgwg gave their consent to have the original version of this shader
    distributed under the GPL in this message:

        http://board.byuu.org/viewtopic.php?p=26075#p26075

        "Feel free to distribute my shaders under the GPL. After all, the
        barrel distortion code was taken from the Curvature shader, which is
        under the GPL."
*/
	
<fragment outscale = "1.0"  filter="nearest"><![CDATA[
uniform sampler2D rubyTexture;
uniform vec2 rubyInputSize;
uniform vec2 rubyOutputSize;
uniform vec2 rubyTextureSize;

// Tweakable options
		
#define CRTgamma 2.2
#define monitorgamma 2.4
#define cornersize 0.005
#define cornersmooth 400.0
#define DOTMASK 0.3
#define brightboost 1.0
#define saturation  1.1
#define sharpness   1.0   // 1.0 to 4.0
#define scanstr     0.3   // 0.3 dark, 0.4 bright
#define warpX 0.00
#define warpY 0.00
#define ringing 0.1


	
// Use the older, purely gaussian beam profile
//#define USEGAUSSIAN

        // Macros.
        #define FIX(c) max(abs(c), 1e-5);
        #define PI 3.141592653589

	//#define TEX2D(c) texture2D(rubyTexture, c)
	  #define TEX2D(c) pow(texture2D(rubyTexture, (c)), vec4(CRTgamma, CRTgamma, CRTgamma, CRTgamma))
  


// overscan (e.g. 102.0 for 2% overscan)
const  vec2 overscan = vec2( 100.0, 100.0 );
		

// Distortion of scanlines, and end of screen alpha.
vec2 Warp(vec2 pos){
  pos=pos*2.0-1.0;    
  pos*=vec2(1.0+(pos.y*pos.y)*warpX,1.0+(pos.x*pos.x)*warpY);
  return pos*0.5+0.5;} 
  
  
float corner(vec2 coord)
{
	coord *= rubyTextureSize / rubyInputSize;
	coord = (coord - vec2(0.5)) * vec2(overscan.x / 100.0, overscan.y / 100.0) + vec2(0.5);
	coord = min(coord, vec2(1.0)-coord) * vec2(1.0, rubyInputSize.y/rubyInputSize.x);
	vec2 cdist = vec2(cornersize);
	coord = (cdist - min(coord,cdist));
	float dist = sqrt(dot(coord,coord));
	return clamp((cdist.x-dist)*cornersmooth,0.0, 1.0);
} 

        // Calculate the influence of a scanline on the current pixel.
        //
        // 'distance' is the distance in texture coordinates from the current
        // pixel to the scanline in question.
        // 'color' is the colour of the scanline at the horizontal location of
        // the current pixel.
        vec4 scanlineWeights(float distance, vec4 color)
        {
                // "wid" controls the width of the scanline beam, for each RGB
                // channel The "weights" lines basically specify the formula
                // that gives you the profile of the beam, i.e. the intensity as
                // a function of distance from the vertical center of the
                // scanline. In this case, it is gaussian if width=2, and
                // becomes nongaussian for larger widths. Ideally this should
                // be normalized so that the integral across the beam is
                // independent of its width. That is, for a narrower beam
                // "weights" should have a higher peak at the center of the
                // scanline than for a wider beam.
        #ifdef USEGAUSSIAN
                vec4 wid = 0.3 + 0.1 * pow(color, vec4(3.0,3.0,3.0,3.0));
                vec4 weights = vec4(distance,distance,distance,distance) / wid;
                return 0.4 * exp(-weights * weights) / wid;
        #else
                vec4 wid = 2.0 + 2.0 * pow(color, vec4(4.0,4.0,4.0,4.0));
                vec4 weights = vec4(distance,distance,distance,distance) / scanstr;
                return 1.4 * exp(-pow(weights * pow(0.5 * wid, vec4(-0.5)), wid)) / (0.6 + 0.2 * wid);
        #endif
        }

void main()
{
				vec2 TextureSize = rubyTextureSize;
	
				// Calculating texel coordinates

				vec2 size     = TextureSize;
				vec2 one = 1.0/TextureSize;
	
                // Here's a helpful diagram to keep in mind while trying to
                // understand the code:
                //
                //  |      |      |      |      |
                // -------------------------------
                //  |      |      |      |      |
                //  |  01  |  11  |  21  |  31  | <-- current scanline
                //  |      | @    |      |      |
                // -------------------------------
                //  |      |      |      |      |
                //  |  02  |  12  |  22  |  32  | <-- next scanline
                //  |      |      |      |      |
                // -------------------------------
                //  |      |      |      |      |
                //
                // Each character-cell represents a pixel on the output
                // surface, "@" represents the current pixel (always somewhere
                // in the bottom half of the current scan-line, or the top-half
                // of the next scanline). The grid of lines represents the
                // edges of the texels of the underlying texture.

                // Texture coordinates of the texel containing the active pixel.
                vec2 xy = gl_TexCoord[0].xy;
				xy = Warp(xy*(rubyTextureSize/rubyInputSize))*(rubyInputSize/rubyTextureSize); 

                float cval = corner(xy);

                // Of all the pixels that are mapped onto the texel we are
                // currently rendering, which pixel are we currently rendering?
                vec2 ratio_scale = xy * size - vec2(0.5,0.5);

                vec2 uv_ratio = fract(ratio_scale);

                // Snap to the center of the underlying texel.
                xy = (floor(ratio_scale) + vec2(0.5,0.5)) / size;

                // Calculate Lanczos scaling coefficients describing the effect
                // of various neighbour texels in a scanline on the current
                // pixel.
                vec4 coeffs = PI * vec4(1.0 + uv_ratio.x, uv_ratio.x, 1.0 - uv_ratio.x, 2.0 - uv_ratio.x);

                // Prevent division by zero.
                coeffs = FIX(coeffs);

                // Lanczos2 kernel.
                coeffs = 2.0 * sin(coeffs) * sin(coeffs / 2.0) / (coeffs * coeffs);
				
				// Apply sharpness hack
				coeffs = sign(coeffs)*pow(abs(coeffs), vec4(sharpness,sharpness,sharpness,sharpness));

                // Normalize.
                coeffs /= dot(coeffs, vec4(1.0,1.0,1.0,1.0));

                // Calculate the effective colour of the current and next
                // scanlines at the horizontal location of the current pixel,
                // using the Lanczos coefficients above.

		vec4 a1 = TEX2D(xy + vec2(-one.x, 0.0));
		vec4 a2 = TEX2D(xy);
		vec4 a3 = TEX2D(xy + vec2(one.x, 0.0));
		vec4 a4 = TEX2D(xy + vec2(2.0 * one.x, 0.0));

		vec4 b1 = TEX2D(xy + vec2(-one.x, one.y));
		vec4 b2 = TEX2D(xy + vec2(0.0, one.y));
		vec4 b3 = TEX2D(xy + vec2(one.x, one.y));
		vec4 b4 = TEX2D(xy + vec2(2.0 * one.x, one.y));

		//vec4 na = min(min(a1,a2),min(a3,a4));
		//vec4 xa = max(max(a1,a2),max(a3,a4));

		//vec4 nb = min(min(b1,b2),min(b3,b4));
		//vec4 xb = max(max(b1,b2),max(b3,b4));
	
		vec4 col  = clamp(mat4(a1,a2,a3,a4) * coeffs, (1.0-ringing)*min(a2,a3), (1.0+ringing)*max(a2,a3));
		vec4 col2 = clamp(mat4(b1,b2,b3,b4) * coeffs, (1.0-ringing)*min(b2,b3), (1.0+ringing)*max(b2,b3));


                //col  = pow(col , vec4(CRTgamma,CRTgamma,CRTgamma,CRTgamma));
                //col2 = pow(col2, vec4(CRTgamma,CRTgamma,CRTgamma,CRTgamma));

                // Calculate the influence of the current and next scanlines on
                // the current pixel.
                vec4 weights  = scanlineWeights(uv_ratio.y, col);
                vec4 weights2 = scanlineWeights(1.0 - uv_ratio.y, col2);

                vec3 mul_res  = (col * weights + col2 * weights2).rgb * vec3(cval,cval,cval);

				float mod_factor = gl_FragCoord.x;
				
				vec3 dotMaskWeights = mix(vec3(1.0, 1.0 - DOTMASK, 1.0), vec3(1.0 - DOTMASK, 1.0, 1.0 - DOTMASK), floor(mod(mod_factor, 2.0)));

				mul_res *= dotMaskWeights; 				
				
                // Convert the image gamma for display on our output device.
                mul_res = pow(mul_res, vec3(1.0 / monitorgamma,1.0 / monitorgamma,1.0 / monitorgamma));

				// Saturation
				float l = length(mul_res);
				mul_res = normalize(pow(mul_res, vec3(saturation, saturation, saturation)))*l;		
				
                // Color the texel.
				gl_FragColor = vec4(mul_res*brightboost, 1.0);
}
]]></fragment>
	

</shader>
