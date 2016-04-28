#include "isxPlayer.h"

#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <memory>



namespace isx
{
class Player::MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    MyOpenGLWidget(const std::shared_ptr<QBoxLayout> & inParent)
    : context_(new QOpenGLContext())
    {

    }

private:
    std::unique_ptr<QOpenGLContext> context_;    

};

Player::Player(const std::shared_ptr<QBoxLayout> & inParent)
: window_(new MyOpenGLWidget(inParent))
{

}

Player::~Player()
{}

void
Player::start()
{
    if (isPlaying_)
    {
        return;
    }




    isPlaying_ = true;
}

void
Player::stop()
{
    if (!isPlaying_)
    {
        return;
    }

    isPlaying_ = false;
}

bool
Player::isPlaying()
{
    return isPlaying_;
}

} // namespace isx
