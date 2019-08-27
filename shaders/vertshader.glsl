attribute vec4 vertex; /* Entire Vector position value*/
attribute vec4 texture_loc0; /* textures' locations*/
attribute vec4 texture_loc1;
attribute vec4 texture_loc2;
attribute vec4 texture_loc3;

uniform mat4 tex0Matrix;
uniform mat4 tex1Matrix;
uniform mat4 tex2Matrix;
uniform mat4 tex3Matrix;

uniform vec4 startLocation;
uniform vec4 trans;

uniform float leftMarginTexture0;
uniform float leftMarginTexture1;
uniform float leftMarginTexture2;
uniform float leftMarginTexture3;

varying vec2 tcoord0;
varying vec2 tcoord1;
varying vec2 tcoord2;
varying vec2 tcoord3;

varying float vertexPoint;

precision mediump float;

void main(void)
{
    vec4 left0 = vec4(leftMarginTexture0, 0,0,0);
    vec4 left1 = vec4(leftMarginTexture1, 0,0,0);
    vec4 left2 = vec4(leftMarginTexture2, 0,0,0);
    vec4 left3 = vec4(leftMarginTexture3, 0,0,0);

	tcoord0.xy = vec2(texture_loc0.xy) ;
	tcoord1.xy = vec2(texture_loc1.xy)  ;
	tcoord2.xy = vec2(texture_loc2.xy) ;
	tcoord3.xy = vec2(texture_loc3.xy) ;

    vertexPoint = vertex.x;

    //first image
    if (vertexPoint >= 0.0 && vertexPoint < 1.0)
    {
        gl_Position = vertex * tex0Matrix + left0;
    }
    //second image
    else if (vertexPoint >= 1.0 && vertexPoint < 2.0)
    {
        gl_Position = (vertex - vec4(1,0,0,0)) * tex1Matrix + left1;
    }
    //third image
    else if (vertexPoint >= 2.0 && vertexPoint < 3.0)
    {
        gl_Position = (vertex - vec4(2,0,0,0)) * tex2Matrix + left2;
    }
    //forth image
    else if (vertexPoint >= 3.0 && vertexPoint < 4.0)
    {
        gl_Position = (vertex - vec4(3,0,0,0)) * tex3Matrix + left3;
    }

    gl_Position = gl_Position - vec4(1,0,0,0) + startLocation + trans;
}
