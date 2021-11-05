/*
 * Copyright (c) 2007-2018, GDash Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:

 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ACTIVITY_HPP_INCLUDED
#define ACTIVITY_HPP_INCLUDED

#include "config.h"

class App;

/**
 * @ingroup Framework
 * @brief An activity object is a mini-application controlled by the user in the
 * game - for example the title screen, a file selection dialog or the game
 * playing.
 *
 * The activity objects are managed by an App object, which organizes them in
 * a stack. Every user event (keypress etc.) is always sent to the topmost activity,
 * which is an instance of a derived class of this Activity class.
 *
 * Activities are free to do just like anything to the screen. They can measure time
 * by overloading the Activity::timer_event() function; they can receive keypresses
 * by overloading the Activity::keypress_event() function. They are notified when
 * they become the topmost one by calling their Activity::shown_event() method,
 * and when occluded by a new activity, by their Activity::hidden_event() method.
 * Their redraw_event() is called by the App object, when they must redraw the screen.
 *
 * Activities can enqueue Command objects in the queue of the App. After calling any
 * of the event methods, the App will check if there are any commands enqueued. If so,
 * they are executed, and this way the activities can control the flow of the whole
 * application.
 */
class Activity {
public:
    /** The keycode given to an activity is either one of the
     * App::KeyCodeSpecialKey codes, or a Unicode encoded character. */
    typedef unsigned KeyCode;

    /** Construct an activity.
     * @param app The App, in which the activity will run. */
    Activity(App *app) : app(app), redraw_queued(false) {}

    virtual ~Activity() = default;

private:
    /**
     * Inform the Activity object about time elapsing.
     * @param ms_elapsed The number of milliseconds elapsed. These are
     * usually not real milliseconds, but the interval of the timer used
     * by the App. */
    virtual void timer_event(int ms_elapsed) {}
    
    /**
     * This is another timer, which is seldom used. It is called only in
     * replay video saving mode, and implemented only by the
     * ReplaySaverActivity class. The timer2 events are generated there
     * by the SDL_Mixer library, to use the audio mixing routine for the
     * cave replay to be totally in sync with the framerate of the game. */
    virtual void timer2_event() {}
    
    /**
     * A key pressed, sent to the Activity.
     * @param keycode a unicode character code, or some special key (see KeyCodeSpecialKey)
     * @param gfxlib_keycode graphics library specific keycode (sdl/gtk/etc). This is
     * here so the Activity can know it and also catch it, to be able to configure the
     * GameInputHandler object.
     */
    virtual void keypress_event(KeyCode keycode, int gfxlib_keycode) {}
    
    
    /**
     * Text input, to be appended to the current input.
     * This is mainly SDL-specific.
     * @param appendtext UTF-8 text.
     */
    virtual void textinput_event(char *appendtext) {}
    
    /**
     * Text input, to overwrite the current input.
     * This is mainly SDL-specific.
     * @param overwritetext UTF-8 text.
     */
    virtual void textediting_event(char *overwritetext) {}
    
    /**
     * Called by the App to request the topmost Activity to redraw the screen.
     * The state of the Activity should not change, as it may be called any time.
     * When the Activity becomes the topmost one, this is called by the App
     * automatically. Activities should not call their own redraw_event() methods.
     */
    virtual void redraw_event(bool full) const = 0;
    
    /**
     * Called by the App when the Activity gets pushed into the stack of activities
     * of the App. The Activity can perform some post-initialization, it may even
     * pop itself. The Command
     * objects enqueued in this event will be executed when the Activity is already
     * in the stack, so new activities they may create will be above in the stack.
     * An activity will only receive one pushed_event() in its lifetime. */
    virtual void pushed_event() {}
    
    /**
     * Called by the App when the activities occluding the current activities have
     * disappeared. May perform any actions, but it is not necessary to do a redraw,
     * because the redraw_event will also be called. An activity may receive many
     * shown_event()-s in its lifetime. */
    virtual void shown_event() {}
    
    /**
     * Called by the App when the Activity is occluded by a new one. May free some
     * resources, for example, to be reacquired later, when the Acitivity becomes
     * topmost again. An activity may receive many hidden_event()-s in its lifetime. */
    virtual void hidden_event() {}

protected:
    friend class App;
    
    /** The owner app. Used for enqueueing Command objects, for example. */
    App *app;
    
    /** There is something to draw. */
    bool redraw_queued;
    
    /** Event handlers can call this function to tell the App that a redraw should be done.
     * Redraws are processed after all events and commands. */
    void queue_redraw() {
        redraw_queued = true;
    }

private:
    Activity(Activity const &) = delete;
    void operator=(Activity const &) = delete;

};

#endif // GD_ACTIVITY_H
