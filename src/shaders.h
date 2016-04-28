#ifndef shaders_h
#define shaders_h


const char fragmentShader[] = \
"#version 120\n"\
\
"// Interpolated values from the vertex shaders\n"\
"varying vec2 UV;\n"\
\
"// Values that stay constant for the whole mesh.\n"\
"uniform sampler2D myTextureSampler;\n"\
\
"void main(){\n"\
\
"    // Output color = color of the texture at the specified UV\n"\
"    gl_FragColor = texture2D( myTextureSampler, UV );\n"\
"    //gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"\
"}\n"\
;

const char vertexShader[] = \
"#version 120\n"\
\
"// Input vertex data, different for all executions of this shader.\n"\
"attribute vec3 vertexPosition_modelspace;\n"\
"attribute vec2 vertexUV;\n"\
\
"// Output data ; will be interpolated for each fragment.\n"\
"varying vec2 UV;\n"\
\
"// Values that stay constant for the whole mesh.\n"\
"uniform mat4 MVP;\n"\
\
"void main(){\n"\
\
"    // Output position of the vertex, in clip space : MVP * position\n"\
"    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);\n"\
\
"    // UV of the vertex. No special space for this one.\n"\
"    UV = vertexUV;\n"\
"}\n"\
;

#endif /* shaders_h */
