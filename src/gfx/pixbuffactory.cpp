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

#include <glib/gi18n.h>
#include <stdexcept>
#include <memory>

#include "gfx/pixbuffactory.hpp"
#include "misc/autogfreeptr.hpp"
#include "gfx/pixbuf.hpp"
#include "gfx/pixbufmanip.hpp"
#include "settings.hpp"

/* scale2x is not translated: the license says that we should call it in its original name. */
// TRANSLATORS: you can translate "nearest neighbor" to "nearest" if the resulting
// string would be too long otherwise.
const char *gd_scaling_names[] = {N_("Nearest neighbor"), "Scale2x", "HQX", NULL};
static_assert(G_N_ELEMENTS(gd_scaling_names) == GD_SCALING_MAX + 1); /* +1 is the terminating NULL */



/* scales a pixbuf with the appropriate scaling type. */
std::unique_ptr<Pixbuf> PixbufFactory::create_scaled(const Pixbuf &src, double scaling_factor, GdScalingType scaling_type, bool pal_emulation) const {

    // title screen: use user-definde scaling factor
    if (gd_full_cave_view && src.get_width() >= 100 && src.get_height() >= 100) {
        scaling_factor = gd_cell_scale_factor_game;
    }

    std::unique_ptr<Pixbuf> scaled = this->create(src.get_width() * scaling_factor, src.get_height() * scaling_factor);

    // NOTE: integer scaling factors 1-4 have special algorithms
    if (scaling_factor == 1.0) {
        src.copy(*scaled, 0, 0);
    } else if (scaling_factor == 2.0) {
        switch (scaling_type) {
            case GD_SCALING_NEAREST:
                scale2xnearest(src, *scaled);
                break;
            case GD_SCALING_SCALE2X:
                scale2x(src, *scaled);
                break;
            case GD_SCALING_HQX:
                hq2x(src, *scaled);
                break;
            case GD_SCALING_MAX:
                g_assert_not_reached();
                break;
        }
    } else if (scaling_factor == 3.0) {
        switch (scaling_type) {
            case GD_SCALING_NEAREST:
                scale3xnearest(src, *scaled);
                break;
            case GD_SCALING_SCALE2X:
                scale3x(src, *scaled);
                break;
            case GD_SCALING_HQX:
                hq3x(src, *scaled);
                break;
            case GD_SCALING_MAX:
                g_assert_not_reached();
                break;
        }
    } else if (scaling_factor == 4.0) {
        switch (scaling_type) {
            case GD_SCALING_NEAREST:
                /* 2x nearest applied twice. */
                {
                    std::unique_ptr<Pixbuf> scale2x(this->create(src.get_width() * 2, src.get_height() * 2));
                    scale2xnearest(src, *scale2x);
                    scale2xnearest(*scale2x, *scaled);
                }
                break;
            case GD_SCALING_SCALE2X:
                /* scale2x applied twice. */
                {
                    std::unique_ptr<Pixbuf> scale2xpb(this->create(src.get_width() * 2, src.get_height() * 2));
                    scale2x(src, *scale2xpb);
                    scale2x(*scale2xpb, *scaled);
                }
                break;
            case GD_SCALING_HQX:
                hq4x(src, *scaled);
                break;
            case GD_SCALING_MAX:
                g_assert_not_reached();
                break;
        }
    } else {
        src.scale(*scaled, scaling_factor, scaling_type);
    }

    if (pal_emulation)
        pal_emulate(*scaled);

    return scaled;
}


std::unique_ptr<Pixbuf> PixbufFactory::create_from_base64(const char *base64) const {
    gsize len;
    AutoGFreePtr<unsigned char> decoded(g_base64_decode(base64, &len));
    return create_from_inline(len, decoded);
}
