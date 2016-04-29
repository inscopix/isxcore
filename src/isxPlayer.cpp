#include "isxPlayer.h"
#include "isxMovie.h"

#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QtGlobal>
#include <QtDebug>
#include <QTimer>
#include <memory>
#include <vector>
#include <cmath>

#include "shaders.h"

namespace isx
{
class Player::MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    typedef std::function<void(uint32_t, void *, size_t)> tGetFrameCB;

    MyOpenGLWidget(QWidget * inParent, QBoxLayout * inLayout)
    : QOpenGLWidget(inParent)
    , timer_(new QTimer())
    {
        inLayout->addWidget(this);
        QSurfaceFormat fmt = format();
        fmt.setVersion(2, 1);
//        setFormat(fmt);
        
        mvp_matrix_.resize(16);
    }
    
    void setMovieInfo(
        int32_t inFrameWidth,
        int32_t inFrameHeight,
        size_t inFrameSizeInBytes,
        int32_t inFirstFrameIndex,
        int32_t inLastFrameIndex,
        tGetFrameCB inGetFrameCB)
    {
        makeCurrent();
        
        frameWidth_ = inFrameWidth;
        frameHeight_ = inFrameHeight;
        frameSizeInBytes_ = inFrameSizeInBytes;
        firstFrameIndex_ = inFirstFrameIndex;
        lastFrameIndex_ = inLastFrameIndex;
        
        currentFrameIndex_ = firstFrameIndex_;
        
        getFrameCB_  = inGetFrameCB;
        
        glGenTextures(1, &texObj_);
        glBindTexture(GL_TEXTURE_2D, texObj_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
#if 1
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frameWidth_, frameHeight_, 0, GL_LUMINANCE, GL_UNSIGNED_SHORT, 0);
#else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frameWidth_, frameHeight_, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#endif
        updateTexture(texObj_, frameWidth_, frameHeight_);
        
        // Create and compile our GLSL program from the shaders
        programObj_ = buildShaders(vertexShader, fragmentShader);   // defined in shaders.h
        
        // Get a handle for our "MVP" uniform
        matrixID_ = glGetUniformLocation(programObj_, "MVP");
        textureID_ = glGetUniformLocation(programObj_, "myTextureSampler");
        
        // Get a handle for our buffers
        vertexPosition_modelspaceID_ = glGetAttribLocation(programObj_, "vertexPosition_modelspace");
        vertexUVID_ = glGetAttribLocation(programObj_, "vertexUV");
        
#if 1
        static float s_vertex_buffer_data[] =
        {
            0.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            1.f, 1.f, 0.f,
            1.f, 0.f, 0.f
        };
        
        static float s_uv_buffer_data[] =
        {
            0.f, 0.f,
            0.f, 1.f,
            1.f, 1.f,
            1.f, 0.f
        };
#else
        static float s_vertex_buffer_data[] =
        {
            200.f, 125.f, 0.f,
            100.f, 375.f, 0.f,
            300.f, 375.f, 0.f
        };
        
        static float s_uv_buffer_data[] =
        {
            0.5f, 0.f,
            0.f, 1.f,
            1.f, 1.f
        };
#endif
        
        glGenBuffers(1, &vertexbufferObj_);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbufferObj_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertex_buffer_data), s_vertex_buffer_data, GL_STATIC_DRAW);
        
        glGenBuffers(1, &uvbufferObj_);
        glBindBuffer(GL_ARRAY_BUFFER, uvbufferObj_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_uv_buffer_data), s_uv_buffer_data, GL_STATIC_DRAW);
        
        doneCurrent();
    }


    void start()
    {
        QObject::connect(timer_.get(), &QTimer::timeout, this, &MyOpenGLWidget::nextFrame);
        timer_->start(1000/20); // 20 Hz
    }
    
    void stop()
    {
    }
    
    void
    setCurrentFrameCB(tCurrentFrameCB inCurrentFrameCB)
    {
        currentFrameCB_ = inCurrentFrameCB;
    }


protected:
    void initializeGL()
    {
        bool valid = isValid();
        if (valid)
        {
            initializeOpenGLFunctions();
            
            glClearColor(0.f, 0.f, 0.f, 1.f);
        }
        else
        {
            qDebug() << "QOpenGLWidget::isValid: " << valid << "\n";
        }
        
    }

    void resizeGL(int w, int h)
    {
        float r_x = 2.f / float(1.f);
        float r_y = 2.f / float(1.f);
        float r_z = -2.f / (-1.f - 1.f);
        float t_x = -1.f;
        float t_y = -1.f;
        float t_z = 0.f;
        
        mvp_matrix_ = {
            r_x, 0.f, 0.f, 0.f,
            0.f, r_y, 0.f, 0.f,
            0.f, 0.f, r_z, 0.f,
            t_x, t_y, t_z, 1.f
        };

    }

    void paintGL()
    {
        if (vertexbufferObj_ && uvbufferObj_)
        {
            
            glClear(GL_COLOR_BUFFER_BIT);		/* clear the display */
            glUseProgram(programObj_);
            
            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(matrixID_, 1, GL_FALSE, &mvp_matrix_[0]);
            
            // Bind our texture in Texture Unit 0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texObj_);
            // Set our "myTextureSampler" sampler to use Texture Unit 0
            glUniform1i(textureID_, 0);
            
            glEnableVertexAttribArray(vertexPosition_modelspaceID_);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbufferObj_);
            glVertexAttribPointer(
                vertexPosition_modelspaceID_, // The attribute we want to configure
                3,                            // size
                GL_FLOAT,                     // type
                GL_FALSE,                     // normalized?
                0,                            // stride
                (void*)0                      // array buffer offset
            );
            
            // 2nd attribute buffer : UVs
            glEnableVertexAttribArray(vertexUVID_);
            glBindBuffer(GL_ARRAY_BUFFER, uvbufferObj_);
            glVertexAttribPointer(
                vertexUVID_,                  // The attribute we want to configure
                2,                            // size : U+V => 2
                GL_FLOAT,                     // type
                GL_FALSE,                     // normalized?
                0,                            // stride
                (void*)0                      // array buffer offset
            );
            
            // Draw the triangles !
            glDrawArrays(GL_QUADS, 0, 16*3); // 12*3 indices starting at 0 -> 12 triangles
            
            glDisableVertexAttribArray(vertexPosition_modelspaceID_);
            glDisableVertexAttribArray(vertexUVID_);

            currentFrameCB_(0.0, currentFrameIndex_);
        }
    }
    
private:
    GLuint buildShaders(const char * vertex_shader, const char * fragment_shader)
    {
        // Create the shaders
        GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        
        GLint Result = GL_FALSE;
        int InfoLogLength;
        
        // Compile Vertex Shader
        char const * VertexSourcePointer = vertex_shader;
        glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
        glCompileShader(VertexShaderID);
        
        // Check Vertex Shader
        glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            printf("%s\n", &VertexShaderErrorMessage[0]);
        }
        
        // Compile Fragment Shader
        char const * FragmentSourcePointer = fragment_shader;
        glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
        glCompileShader(FragmentShaderID);
        
        // Check Fragment Shader
        glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            printf("%s\n", &FragmentShaderErrorMessage[0]);
        }
        
        // Link the program
        GLuint ProgramID = glCreateProgram();
        glAttachShader(ProgramID, VertexShaderID);
        glAttachShader(ProgramID, FragmentShaderID);
        glLinkProgram(ProgramID);
        
        // Check the program
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            std::vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            printf("%s\n", &ProgramErrorMessage[0]);
        }
        
        
        glDetachShader(ProgramID, VertexShaderID);
        glDetachShader(ProgramID, FragmentShaderID);
        
        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);
        
        return ProgramID;
    }
    
    void updateTexture(unsigned int tex, int w, int h)
    {
        static std::vector<unsigned char> buf(frameSizeInBytes_);
        static std::vector<unsigned char> texbuf(w * h);
        
        getFrameCB_(currentFrameIndex_, &buf[0], frameSizeInBytes_);
        
        unsigned short int * p = (unsigned short int *)&buf[0];
        for (int i = 0; i < w * h; ++i)
        {
            float f = float(p[i]);
            f /= 4095.f;
            f *= 255.f;
            f += 0.5f;
            f = std::floor(f);
            
            texbuf[i] = (unsigned char) f; //(unsigned char)(floorf((float(b) / 4095.f) + 0.5f) * 255.f);
            
        }
        
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_LUMINANCE, GL_UNSIGNED_BYTE, &texbuf[0]);
    }
    
    void nextFrame()
    {
        ++currentFrameIndex_;
        if (currentFrameIndex_ > lastFrameIndex_)
        {
            currentFrameIndex_ = firstFrameIndex_;
        }
        updateTexture(texObj_, frameWidth_, frameHeight_);
        update();
    }
    
    int32_t currentFrameIndex_ = 0;
    int32_t firstFrameIndex_ = 0;
    int32_t lastFrameIndex_ = 0;

    int32_t frameWidth_ = 0;
    int32_t frameHeight_ = 0;
    size_t frameSizeInBytes_;
    tGetFrameCB getFrameCB_;
    tCurrentFrameCB currentFrameCB_;
    
    std::vector<float> mvp_matrix_;

    std::unique_ptr<QTimer> timer_;
    
    // opengl objects and IDs
    uint32_t texObj_ = 0;
    uint32_t programObj_ = 0;
    uint32_t matrixID_ = 0;
    uint32_t textureID_ = 0;
    uint32_t vertexPosition_modelspaceID_ = 0;
    uint32_t vertexUVID_ = 0;

    uint32_t vertexbufferObj_ = 0;
    uint32_t uvbufferObj_ = 0;

};

Player::Player(QWidget * inParent, QBoxLayout * inLayout)
: window_(new MyOpenGLWidget(inParent, inLayout))
{

}

Player::~Player()
{}

void
Player::setMovie(const tMovie_SP & inMovie)
{
    if (isValid_ && !isPlaying_)
    {
        movie_ = inMovie;
        window_->setMovieInfo(
            movie_->getFrameWidth(),
            movie_->getFrameHeight(),
            movie_->getFrameSizeInBytes(),
            0,
            movie_->getNumFrames() - 1,
            [&](uint32_t inFrameNumber, void * inBuffer, size_t inBufferSize){
                if (inBufferSize == movie_->getFrameSizeInBytes())
                {
                    movie_->getFrame(inFrameNumber, inBuffer, inBufferSize);
                }
                
        });
    }
}

void
Player::start()
{
    if (!isValid_ || isPlaying_)
    {
        return;
    }
    window_->start();
    window_->update();



    isPlaying_ = true;
}

void
Player::stop()
{
    if (!isValid_ || !isPlaying_)
    {
        return;
    }

    isPlaying_ = false;
}

bool
Player::isPlaying()
{
    return isValid_ && isPlaying_;
}

bool
Player::isValid()
{
    isValid_ = window_->isValid();
    if (!isValid_)
    {
        window_.reset();
    }
    
    return isValid_;
}
    
void
Player::setCurrentFrameCB(tCurrentFrameCB inCurrentFrameCB)
{
    if (isValid_)
    {
        window_->setCurrentFrameCB(inCurrentFrameCB);
    }
}
    
    
} // namespace isx
