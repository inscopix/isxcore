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
#include <array>
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
    {
        inLayout->addWidget(this);
        QSurfaceFormat fmt = format();
        fmt.setVersion(2, 1);
//        setFormat(fmt);
        
    }
    
    // TODO aschildan, 05/04/2016: Aggregate the parameters once we have a eg spaceGrid class
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
        
        programObj_ = buildShaders(vertexShader, fragmentShader);   // defined in shaders.h
        
        matrixID_ = glGetUniformLocation(programObj_, "MVP");
        textureID_ = glGetUniformLocation(programObj_, "myTextureSampler");
        
        vertexPosition_modelspaceID_ = glGetAttribLocation(programObj_, "vertexPosition_modelspace");
        vertexUVID_ = glGetAttribLocation(programObj_, "vertexUV");
        
        uv_buffer_ = { {
            0.f, 1.f,
            0.f, 0.f,
            1.f, 0.f,
            1.f, 1.f
        } };

        glGenBuffers(1, &vertexbufferObj_);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbufferObj_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_), &vertex_buffer_[0], GL_STATIC_DRAW);
        
        glGenBuffers(1, &uvbufferObj_);
        glBindBuffer(GL_ARRAY_BUFFER, uvbufferObj_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_), &uv_buffer_[0], GL_STATIC_DRAW);
        
        glGenTextures(1, &texObj_);
        glBindTexture(GL_TEXTURE_2D, texObj_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frameWidth_, frameHeight_, 0, GL_LUMINANCE, GL_UNSIGNED_SHORT, 0);

        updateTexture(texObj_, frameWidth_, frameHeight_);
        resizeGL(width(), height());
        doneCurrent();
        update();
    }

    void start()
    {
        timer_.reset(new QTimer());
        QObject::connect(timer_.get(), &QTimer::timeout, this, &MyOpenGLWidget::nextFrame);
        timer_->start(1000/20); // 20 Hz
    }
    
    void stop()
    {
        timer_->stop();
        timer_.reset();
    }
    
    void setTime(float inTime)
    {
        float t = std::min(1.f, std::max(0.f, inTime));
        currentFrameIndex_ = std::floor(float(firstFrameIndex_) + t * float(lastFrameIndex_ - firstFrameIndex_) + 0.5f);
        currentFrameIndex_ = std::min(lastFrameIndex_, std::max(firstFrameIndex_, currentFrameIndex_));
        updateTexture(texObj_, frameWidth_, frameHeight_);
        update();
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

    void resizeGL(int inWidth, int inHeight)
    {
        if (frameWidth_ == 0 || frameHeight_ == 0)
        {
            return;
        }

        windowWidth_ = inWidth;
        windowHeight_ = inHeight;

        float x0 = 0.f;
        float y0 = 0.f;
        float x1 = float(windowWidth_);
        float y1 = float(windowHeight_);

        float r_x = 2.f / float(windowWidth_);
        float r_y = 2.f / float(windowHeight_);
        float r_z = -2.f / (-1.f - 1.f);
        float t_x = -1.f;
        float t_y = -1.f;
        float t_z = 0.f;
        
        mvp_matrix_ = { {
            r_x, 0.f, 0.f, 0.f,
            0.f, r_y, 0.f, 0.f,
            0.f, 0.f, r_z, 0.f,
            t_x, t_y, t_z, 1.f
        } };
        
        float wr = float(windowWidth_) / float(windowHeight_);
        float fr = float(frameWidth_) / float(frameHeight_);
        
        if (fr > wr)
        {
            // letter box
            float scale = float(windowWidth_) / float(frameWidth_);
            float scaledHeight = scale * float(frameHeight_);
            y0 = (float(windowHeight_) - scaledHeight) / 2.f;
            y1 = y0 + scaledHeight;
        }
        else
        {
            // pillar box
            float scale = float(windowHeight_) / float(frameHeight_);
            float scaledWidth = scale * float(frameWidth_);
            x0 = (float(windowWidth_) - scaledWidth) / 2.f;
            x1 = x0 + scaledWidth;
        }
        
        vertex_buffer_ = { {
            x0, y0, 0.f,
            x0, y1, 0.f,
            x1, y1, 0.f,
            x1, y0, 0.f
        } };

        glBindBuffer(GL_ARRAY_BUFFER, vertexbufferObj_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_), &vertex_buffer_[0], GL_STATIC_DRAW);
    }

    void paintGL()
    {
        if (vertexbufferObj_ && uvbufferObj_)
        {
            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(programObj_);
            glUniformMatrix4fv(matrixID_, 1, GL_FALSE, &mvp_matrix_[0]);
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texObj_);
            glUniform1i(textureID_, 0);
            
            glEnableVertexAttribArray(vertexPosition_modelspaceID_);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbufferObj_);
            glVertexAttribPointer(
                vertexPosition_modelspaceID_,
                3,                            // size
                GL_FLOAT,                     // type
                GL_FALSE,                     // normalized?
                0,                            // stride
                (void*)0                      // array buffer offset
            );
            
            glEnableVertexAttribArray(vertexUVID_);
            glBindBuffer(GL_ARRAY_BUFFER, uvbufferObj_);
            glVertexAttribPointer(
                vertexUVID_,
                2,                            // size : U+V => 2
                GL_FLOAT,                     // type
                GL_FALSE,                     // normalized?
                0,                            // stride
                (void*)0                      // array buffer offset
            );
            
            glDrawArrays(GL_QUADS, 0, 16*3);
            
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

            // TODO aschildan, 05/04/2016: Don't hard-code sensor range, instead extract this from recording / movie
            f /= 4095.f;
            f *= 255.f;
            f += 0.5f;
            f = std::floor(f);
            
            texbuf[i] = (unsigned char) f; //(unsigned char)(floorf((float(b) / 4095.f) + 0.5f) * 255.f);
        }

        if (frameWidth_ % 4)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_LUMINANCE, GL_UNSIGNED_BYTE, &texbuf[0]);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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
    
    tCurrentFrameCB currentFrameCB_;
    std::unique_ptr<QTimer> timer_;
    int32_t windowWidth_ = 0;
    int32_t windowHeight_ = 0;
    
    // movie specific data
    int32_t currentFrameIndex_ = 0;
    int32_t firstFrameIndex_ = 0;
    int32_t lastFrameIndex_ = 0;

    int32_t frameWidth_ = 0;
    int32_t frameHeight_ = 0;
    size_t frameSizeInBytes_;
    tGetFrameCB getFrameCB_;
 
    // opengl objects and IDs
    uint32_t texObj_ = 0;
    uint32_t programObj_ = 0;
    uint32_t matrixID_ = 0;
    uint32_t textureID_ = 0;
    uint32_t vertexPosition_modelspaceID_ = 0;
    uint32_t vertexUVID_ = 0;

    uint32_t vertexbufferObj_ = 0;
    uint32_t uvbufferObj_ = 0;

    std::array<float, 16>   mvp_matrix_;
    std::array<float, 12>   vertex_buffer_;
    std::array<float, 8>    uv_buffer_;
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
    isPlaying_ = true;
}

void
Player::stop()
{
    if (!isValid_ || !isPlaying_)
    {
        return;
    }
    window_->stop();
    isPlaying_ = false;
}

void
Player::setTime(float inTime)
{
    if (!isValid_)
    {
        return;
    }
    if (isPlaying_)
    {
        stop();
    }
    window_->setTime(inTime);
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
