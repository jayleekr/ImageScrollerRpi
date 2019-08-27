uniform sampler2D umaps[4];

varying vec2 tcoord0;
varying vec2 tcoord1;
varying vec2 tcoord2;
varying vec2 tcoord3;

varying float vertexPoint;

void main(void)
{
	//first image
	if (vertexPoint >= 0.0 && vertexPoint < 1.0)
	{
		gl_FragColor = vec4( texture2D(umaps[0],tcoord0).bgra);
	}
	//second image
	else if (vertexPoint >= 1.0 && vertexPoint < 2.0)
	{
		gl_FragColor = vec4( texture2D(umaps[1],tcoord1).bgra);
	}
	//third image
	else if (vertexPoint >= 2.0 && vertexPoint < 3.0)
	{
		gl_FragColor = vec4( texture2D(umaps[2],tcoord2).bgra);
	}
	//forth image
	else if (vertexPoint >= 3.0 && vertexPoint < 4.0)
	{
		gl_FragColor = vec4( texture2D(umaps[3],tcoord3).bgra);
	}
}
