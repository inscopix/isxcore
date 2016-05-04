#ifndef ISX_PLAYER_H
#define ISX_PLAYER_H

#include <QBoxLayout>
#include <memory>
#include <functional>
#include "Fwd.h"

namespace isx
{

///
/// A class implementing a player.
///
class Player
{
public:
    /// Type of callback reporting current frame.
    ///
    typedef std::function<void(double, uint32_t)> tCurrentFrameCB;
    
    /// Constructor.
    /// \param inParent Parent window
    /// \param inLayout BoxLayout to which this player will add its window into which it draws its frames
    ///
    Player(QWidget * inParent, QBoxLayout * inLayout);

    /// Destructor.
    ///
    ~Player();

    /// Set movie for player
    /// \param inMovie Movie to play.
    void
    setMovie(const tMovie_SP & inMovie);
    
    /// Start playback.
    ///
    void
    start();

    /// Stop playback.
    ///
    void
    stop();
    
    /// Set current playback time.
    /// \param inTime 0.0 and less represent the beginning of the movie, 1.0 and more represent the end of the movie
    ///
    void
    setTime(float inTime);

    /// Query playback state
    /// \return true means is this player is in playing state
    ///
    bool
    isPlaying();

    /// Query player valid state
    /// \return true means player is valid and can render (eg. failed OpenGL init would yield an invalid player)
    ///
    bool
    isValid();
    
    /// Set callback function that is invoked after every frame draw
    /// \param inCurrentFrameCB callback function takes two parameters: double seconds, uint32_t frameIndex
    ///
    void
    setCurrentFrameCB(tCurrentFrameCB inCurrentFrameCB);

private:
    class MyOpenGLWidget;
    std::shared_ptr<MyOpenGLWidget> window_;
    tMovie_SP movie_;


    bool isValid_ = false;
    bool isPlaying_ = false;

};

} // namespace isx

#endif // ISX_PLAYER_H
