#ifndef ISX_PLAYER_H
#define ISX_PLAYER_H

#include <QBoxLayout>
#include <memory>
#include "Fwd.h"

namespace isx
{

///
/// A class implementing a player.
///
class Player
{
public:
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

private:
    class MyOpenGLWidget;
    std::shared_ptr<MyOpenGLWidget> window_;
    tMovie_SP movie_;

    bool isValid_ = false;
    bool isPlaying_ = false;

};

} // namespace isx

#endif // ISX_PLAYER_H
