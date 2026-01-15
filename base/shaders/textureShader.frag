#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D tex;

void main() {
    vec4 texCol = texture(tex, TexCoords);
    gl_FragDepth = texCol.a;
    FragColor = vec4(texCol.rgb, 1.0);

    // Render depth
    // texCol.a = ( 2.0 * 0.1 ) / ( 10000.0 + 0.1 - texCol.a * ( 10000.0 - 0.1 ) );
    // texCol.a *= 10.0;
    // FragColor = vec4(texCol.aaa, 1.0);
}

