#ifndef shaders_h
#define shaders_h


const char fragmentShader[] = \
"#version 120\n"\
\
"varying vec2 UV;\n"\
\
"uniform sampler2D myTextureSampler;\n"\
\
"void main(){\n"\
\
"    gl_FragColor = texture2D( myTextureSampler, UV );\n"\
"    //gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"\
"}\n"\
;

const char vertexShader[] = \
"#version 120\n"\
\
"attribute vec3 vertexPosition_modelspace;\n"\
"attribute vec2 vertexUV;\n"\
\
"varying vec2 UV;\n"\
\
"uniform mat4 MVP;\n"\
\
"void main(){\n"\
\
"    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);\n"\
\
"    UV = vertexUV;\n"\
"}\n"\
;

#endif /* shaders_h */
