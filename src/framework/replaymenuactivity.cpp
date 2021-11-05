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

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <vector>
#include <memory>

#include "framework/replaymenuactivity.hpp"
#include "framework/commands.hpp"
#include "cave/colors.hpp"
#include "framework/app.hpp"
#include "cave/cavestored.hpp"
#include "misc/util.hpp"
#include "misc/printf.hpp"
#include "framework/gameactivity.hpp"
#include "framework/replaysaveractivity.hpp"
#include "gfx/screen.hpp"
#include "gfx/fontmanager.hpp"
#include "cave/gamecontrol.hpp"
#include "cave/caveset.hpp"
#include "misc/helptext.hpp"

ReplayMenuActivity::ReplayMenuActivity(App *app)
    : Activity(app) {
    lines_per_page = (app->screen->get_height() / app->font_manager->get_line_height()) - 4;

    /* for all caves */
    for (unsigned n = 0; n < app->caveset->caves.size(); ++n) {
        CaveStored &cave = app->caveset->caves[n];

        /* if cave has replays... */
        if (!cave.replays.empty()) {
            /* add an empty item as separator, if not on the first line */
            if (!(items.size() % lines_per_page == 0)) {
                ReplayItem i = { NULL, NULL };
                items.push_back(i);
            }
            /* if now on the last line, add another empty line */
            if (items.size() % lines_per_page == lines_per_page - 1) {
                ReplayItem i = { NULL, NULL };
                items.push_back(i);
            }
            /* add cave data */
            ReplayItem i = { &cave, NULL };
            items.push_back(i);
            /* add replays, too */
            for (std::list<CaveReplay>::iterator rit = cave.replays.begin(); rit != cave.replays.end(); ++rit) {
                // .cave unchanged, .replay is the address of the replay
                i.replay = &*rit;
                items.push_back(i);
            }
        }
    }
    current = 1;
}


void ReplayMenuActivity::pushed_event() {
    if (items.empty()) {
        app->show_message(_("No replays for caveset."), "", std::make_unique<PopActivityCommand>(app));
    }
}

void ReplayMenuActivity::redraw_event(bool full) const {
    app->clear_screen();

    int page = current / lines_per_page;
    // TRANSLATORS: 40 chars max
    app->title_line(_("Replays"));
    // TRANSLATORS: 40 chars max
    app->status_line(_("Space: play   H: help   Esc: exit"));

    for (unsigned n = 0; n < lines_per_page && page * lines_per_page + n < items.size(); n++) {
        unsigned pos = page * lines_per_page + n;

        int y = (n + 2) * app->font_manager->get_line_height();

        if (items[pos].cave && !items[pos].replay) {
            /* no replay pointer: this is a cave, so write its name. */
            app->set_color(GD_GDASH_WHITE);
            app->font_manager->blittext_n(0, y, " %s", items[pos].cave->name);
        }
        if (items[pos].replay) {
            /* level, successful/not, saved/not, player name  */
            app->font_manager->blittext_n(0, y, "  %c%c %c%c %cL%d, %s",
                                                GD_COLOR_INDEX_LIGHTBLUE, (items[pos].replay->saved ? GD_CHECKED_BOX_CHAR : GD_UNCHECKED_BOX_CHAR),
                                                (items[pos].replay->success ? GD_COLOR_INDEX_GREEN : GD_COLOR_INDEX_RED), GD_BALL_CHAR,
                                                (current == pos ? GD_COLOR_INDEX_YELLOW : GD_COLOR_INDEX_GRAY3),
                                                items[pos].replay->level, items[pos].replay->player_name);
        }
    }
    
    if (items.size() > lines_per_page)
        app->draw_scrollbar(0, current, items.size()-1);

    app->screen->drawing_finished();
}


/* the replay saver thing only works in the sdl version */
#ifdef HAVE_SDL

class SaveReplayCommand: public Command1Param<std::string> {
public:
    SaveReplayCommand(App *app, CaveStored *cave, CaveReplay *replay)
        :
        Command1Param<std::string>(app),
        filename_prefix(p1),
        cave(cave),
        replay(replay) {
    }
private:
    std::string &filename_prefix;
    CaveStored *cave;
    CaveReplay *replay;
    void execute() {
        app->enqueue_command(std::make_unique<PushActivityCommand>(app, std::make_unique<ReplaySaverActivity>(app, cave, replay, filename_prefix)));
    }
};

#endif /* IFDEF HAVE_SDL */


void ReplayMenuActivity::keypress_event(KeyCode keycode, int gfxlib_keycode) {
    /* these keys are active also when there are replays and when there aren't any. */
    switch (keycode) {
        case 'h':
        case 'H':
            app->show_help(replayhelp);
            break;
        case App::Escape:
            app->enqueue_command(std::make_unique<PopActivityCommand>(app));
            break;
    }

    /* if no replays, the keys below should be inactive. so return. */
    if (items.empty())
        return;


    switch (keycode) {
        case App::Up:
            do {
                current = gd_clamp(current - 1, 1, items.size() - 1);
            } while (items[current].replay == NULL && current >= 1);
            queue_redraw();
            break;
        case App::Down:
            do {
                current = gd_clamp(current + 1, 1, items.size() - 1);
            } while (items[current].replay == NULL && current < items.size());
            queue_redraw();
            break;
        case App::PageUp:
            current = gd_clamp(current - lines_per_page, 0, items.size() - 1);
            queue_redraw();
            break;
        case App::PageDown:
            current = gd_clamp(current + lines_per_page, 0, items.size() - 1);
            queue_redraw();
            break;
        case 'i':
        case 'I': {
            std::string s;
            CaveReplay *r = items[current].replay;
            s += Printf("%c%s: %c%s\n", GD_COLOR_INDEX_WHITE, _("Cave"), GD_COLOR_INDEX_YELLOW, items[current].cave->name);
            // TRANSLATORS: Cave level (1-5)
            s += Printf("%c%s: %c%d\n", GD_COLOR_INDEX_WHITE, _("Level"), GD_COLOR_INDEX_YELLOW, r->level);
            // TRANSLATORS: "player" here means the name of the human player who played the replay
            s += Printf("%c%s: %c%s\n", GD_COLOR_INDEX_WHITE, _("Player"), GD_COLOR_INDEX_YELLOW, r->player_name);
            s += Printf("%c%s: %c%s\n", GD_COLOR_INDEX_WHITE, _("Successful"), GD_COLOR_INDEX_LIGHTBLUE, r->success ? _("yes") : _("no"));
            s += Printf("%c%s: %c%d %s\n", GD_COLOR_INDEX_WHITE, _("Score"), GD_COLOR_INDEX_LIGHTBLUE, r->score, _("points"));
            s += "\n";
            if (r->comment != "")
                s += Printf("%c%s: %c%s\n", GD_COLOR_INDEX_WHITE, _("Comment"), GD_COLOR_INDEX_LIGHTBLUE, r->comment);
            if (r->duration > 0) {
                // TRANSLATORS: duration of replay in seconds
                s += Printf("%c%s: %c%ds\n", GD_COLOR_INDEX_WHITE, _("Duration"), GD_COLOR_INDEX_LIGHTBLUE, r->duration);
            }
            if (r->date != "")
                s += Printf("%c%s: %c%s\n", GD_COLOR_INDEX_WHITE, _("Date"), GD_COLOR_INDEX_LIGHTBLUE, r->date);
            if (r->recorded_with != "")
                // TRANSLATORS: ie. software by which the replay was recorded.
                s += Printf("%c%s: %c%s\n", GD_COLOR_INDEX_WHITE, _("Recorded with"), GD_COLOR_INDEX_LIGHTBLUE, r->recorded_with);

            // TRANSLATORS: title of the window showing data about a particular replay
            app->show_message(_("Replay Info"), s);
        }
        break;
        case 's':
        case 'S':
            if (items[current].replay) {
                items[current].replay->saved = !items[current].replay->saved;
                app->caveset->edited = true;
                queue_redraw();
            }
            break;
            /* the replay saver thing only works in the sdl version */
#ifdef HAVE_SDL
        case 'w':
        case 'W': {
            std::string prefix = Printf("%s%sout", gd_last_folder, G_DIR_SEPARATOR);
            // TRANSLATE: the prefix of the name of the output files when saving the replay.
            app->input_text_and_do_command(_("Output filename prefix"), prefix.c_str(), std::make_unique<SaveReplayCommand>(app, items[current].cave, items[current].replay));
        }
        break;
#endif /* IFDEF HAVE_SDL */
        case ' ':
        case App::Enter:
            if (items[current].replay) {
                auto game = GameControl::new_replay(app->caveset, items[current].cave, items[current].replay);
                app->enqueue_command(std::make_unique<PushActivityCommand>(app, std::make_unique<GameActivity>(app, std::move(game))));
            }
            break;
    }
}
