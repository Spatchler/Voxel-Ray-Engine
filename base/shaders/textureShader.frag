#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D tex;
uniform vec2 uScreenSize;

const int blurSize = 20;

void main() {
    vec4 texCol = texture(tex, TexCoords);
    gl_FragDepth = texCol.a;
    FragColor = vec4(texCol.rgb, 1.0);

    // Blur -------------------------------------------------
    // ivec2 p = ivec2(TexCoords * uScreenSize);
    // int halfBlurSize = blurSize / 2;

    // vec4 avgValue = vec4(0.0, 0.0, 0.0, 0.0);
    
    // // Take the central sample first
    // // avgValue += texelFetch(tex, p, 0);

    // // Go through the remaining 8 vertical samples (4 on each side of the center)
    // for (int y = p.y - halfBlurSize; y < p.y + halfBlurSize; ++y) for (int x = p.x - halfBlurSize; x < p.x + halfBlurSize; ++x)
    //     avgValue += texelFetch(tex, ivec2(x, y), 0);

    // vec4 texCol = texture(tex, TexCoords);
    // gl_FragDepth = texCol.a;
    // FragColor = avgValue / (blurSize * blurSize);

    // Render depth -----------------------------------------
    // texCol.a = ( 2.0 * 0.1 ) / ( 10000.0 + 0.1 - texCol.a * ( 10000.0 - 0.1 ) );
    // texCol.a *= 10.0;
    // FragColor = vec4(texCol.aaa, 1.0);
}

