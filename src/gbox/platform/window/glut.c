/*!The Graphic Box Library
 * 
 * GBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * GBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with GBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2014 - 2015, ruki All rights reserved.
 *
 * @author      ruki
 * @file        glut.c
 * @ingroup     platform
 *
 */

/* //////////////////////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_MODULE_NAME            "window_glut"
#define TB_TRACE_MODULE_DEBUG           (1)

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "../../core/canvas.h"
#ifdef TB_CONFIG_OS_MAC
# 	include <GLUT/glut.h>
#else
# 	include <GL/glut.h>
#endif

/* //////////////////////////////////////////////////////////////////////////////////////
 * types
 */

// the glut window impl type
typedef struct __gb_window_glut_impl_t
{
    // the base
    gb_window_impl_t        base;

    // the window id
    tb_int_t                id;

    // is stoped?
    tb_atomic_t             stop;

    // the canvas
    gb_canvas_ref_t         canvas;

}gb_window_glut_impl_t;

/* //////////////////////////////////////////////////////////////////////////////////////
 * globals
 */

// the windows
static gb_window_glut_impl_t*   g_windows[16] = {tb_null};

/* //////////////////////////////////////////////////////////////////////////////////////
 * private implementation
 */
static __tb_inline__ gb_window_glut_impl_t* gb_window_glut_get()
{
    tb_int_t id = glutGetWindow();
    return (id < tb_arrayn(g_windows))? g_windows[id] : tb_null;
}
static tb_void_t gb_window_glut_display()
{
    // check
    gb_window_glut_impl_t* impl = gb_window_glut_get();
    tb_assert_and_check_return(impl && impl->canvas);

    // trace
//    tb_trace_d("draw");

    // done draw
    gb_window_impl_draw((gb_window_ref_t)impl, impl->canvas);

	// flush
	glutSwapBuffers();
}
static tb_void_t gb_window_glut_reshape(tb_int_t width, tb_int_t height)
{
    // check
    gb_window_glut_impl_t* impl = gb_window_glut_get();
    tb_assert_and_check_return(impl && width <= GB_WIDTH_MAXN && height <= GB_HEIGHT_MAXN);

    // minimum?
    tb_check_return(width && height);

    // trace
    tb_trace_d("reshape: %dx%d", width, height);

    // the device
    gb_device_ref_t device = gb_canvas_device(impl->canvas);
    tb_assert_and_check_return(device);

    // update the window width and height
    impl->base.width   = width;
    impl->base.height  = height;

    // resize the device
    gb_device_resize(device, width, height);

    // done resize
    if (impl->base.info.resize) impl->base.info.resize((gb_window_ref_t)impl, impl->canvas, impl->base.info.priv);
}
static tb_void_t gb_window_glut_keyboard(tb_byte_t key, tb_int_t x, tb_int_t y)
{ 
    // check
    gb_window_glut_impl_t* impl = gb_window_glut_get();
    tb_assert_and_check_return(impl);

    // trace
    tb_trace_d("keyboard: %d at: %d, %d", key, x, y);
}
static tb_void_t gb_window_glut_special(tb_int_t key, tb_int_t x, tb_int_t y)
{ 
    // check
    gb_window_glut_impl_t* impl = gb_window_glut_get();
    tb_assert_and_check_return(impl);

    // trace
    tb_trace_d("special: %d at: %d, %d", key, x, y);
}
static tb_void_t gb_window_glut_mouse(tb_int_t button, tb_int_t state, tb_int_t x, tb_int_t y)
{ 
    // check
    gb_window_glut_impl_t* impl = gb_window_glut_get();
    tb_assert_and_check_return(impl);

    // trace
    tb_trace_d("passive_mouse: button: %d, state: %d at: %d, %d", button, state, x, y);
}
static tb_void_t gb_window_glut_passive_motion(tb_int_t x, tb_int_t y)
{ 
    // check
    gb_window_glut_impl_t* impl = gb_window_glut_get();
    tb_assert_and_check_return(impl);

    // trace
//    tb_trace_d("passive_motion: %d, %d", x, y);
}
static tb_void_t gb_window_glut_motion(tb_int_t x, tb_int_t y)
{ 
    // check
    gb_window_glut_impl_t* impl = gb_window_glut_get();
    tb_assert_and_check_return(impl);

    // trace
    tb_trace_d("motion: %d, %d", x, y);
}
static tb_void_t gb_window_glut_timer(tb_int_t value)
{
    // check
    gb_window_glut_impl_t* impl = g_windows[value];
    tb_assert_and_check_return(impl);

    // trace
//    tb_trace_d("timer: %d", value);

    // post to draw it
    glutPostRedisplay();

    // next timer
    glutTimerFunc(1000 / impl->base.info.framerate, gb_window_glut_timer, value);
}
#ifndef TB_CONFIG_OS_WINDOWS
static tb_void_t gb_window_glut_close()
{ 
    // check
    gb_window_glut_impl_t* impl = gb_window_glut_get();
    tb_assert_and_check_return(impl && impl->canvas);

    // trace
    tb_trace_d("clos");

    // done clos
    if (impl->base.info.clos) impl->base.info.clos((gb_window_ref_t)impl, impl->base.info.priv);

    // stop it
    tb_atomic_set(&impl->stop, 1);
}
#endif
static tb_void_t gb_window_glut_exit(gb_window_ref_t window)
{
    // check
    gb_window_glut_impl_t* impl = (gb_window_glut_impl_t*)window;
    tb_assert_and_check_return(impl && impl->id < tb_arrayn(g_windows));

    // stop it
    tb_atomic_set(&impl->stop, 1);

    // exit canvas
    if (impl->canvas) gb_canvas_exit(impl->canvas);
    impl->canvas = tb_null;

    // exit window
    if (impl->id) 
    {
        glutDestroyWindow(impl->id);
        g_windows[impl->id] = tb_null;
        impl->id = 0;
    }

    // exit it
    tb_free(window);
}
static tb_void_t gb_window_glut_loop(gb_window_ref_t window)
{
    // check
    gb_window_glut_impl_t* impl = (gb_window_glut_impl_t*)window;
    tb_assert_and_check_return(impl);

    // fullscreen
    if (impl->base.flag & GB_WINDOW_FLAG_FULLSCREEN) 
    {
        // remove fullscreen first
        impl->base.flag &= ~GB_WINDOW_FLAG_FULLSCREEN;

        // enter fullscreen 
        gb_window_fullscreen((gb_window_ref_t)impl, tb_true);
    }
    else
    {
        // append fullscreen first
        impl->base.flag |= GB_WINDOW_FLAG_FULLSCREEN;

        // leave fullscreen 
        gb_window_fullscreen((gb_window_ref_t)impl, tb_false);
    }

    // hide cursor?
    if (impl->base.flag & GB_WINDOW_FLAG_HIHE_CURSOR) 
        glutSetCursor(GLUT_CURSOR_NONE);

    // init canvas
    if (!impl->canvas) impl->canvas = gb_canvas_init_from_window(window);
    tb_assert_abort(impl->canvas);

    // loop
#ifdef TB_CONFIG_OS_WINDOWS
    glutMainLoop();
#else
    while (!tb_atomic_get(&impl->stop))
    {
        glutCheckLoop();
    }
#endif
}
static tb_void_t gb_window_glut_fullscreen(gb_window_ref_t window, tb_bool_t fullscreen)
{
    // check
    gb_window_glut_impl_t* impl = (gb_window_glut_impl_t*)window;
    tb_assert_and_check_return(impl);

    // fullscreen?
    if (fullscreen && !(impl->base.flag & GB_WINDOW_FLAG_FULLSCREEN)) 
    {
        // init mode string
        tb_char_t mode[64] = {0};
        tb_snprintf(mode, sizeof(mode) - 1, "%lux%lu", tb_screen_width(), tb_screen_height());

        // trace
        tb_trace_d("mode: %s", mode);

        // init mode
//      glutGameModeString("640x480:32@60");
        glutGameModeString(mode);

        // can fullscreen?
        if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE))
        {
            // exit the previous window first
            if (impl->id) glutDestroyWindow(impl->id);
            impl->id = 0;

            // enter fullscreen
            tb_int_t id = glutEnterGameMode();
            tb_assert_abort(id < tb_arrayn(g_windows));

            // trace
            tb_trace_d("fullscreen: enter: %d", id);

            // save window
            g_windows[id] = impl;

            // init window func
            glutDisplayFunc(gb_window_glut_display);
            glutReshapeFunc(gb_window_glut_reshape);
            glutKeyboardFunc(gb_window_glut_keyboard);
            glutSpecialFunc(gb_window_glut_special);
            glutMouseFunc(gb_window_glut_mouse);
            glutPassiveMotionFunc(gb_window_glut_passive_motion);
            glutMotionFunc(gb_window_glut_motion);
            glutTimerFunc(1000 / impl->base.info.framerate, gb_window_glut_timer, id);

            // update flag
            impl->base.flag |= GB_WINDOW_FLAG_FULLSCREEN;
        }
        else
        {
            // trace
            tb_trace_e("cannot enter fullscreen: %s", mode);
        }
    }
    else if (impl->base.flag & GB_WINDOW_FLAG_FULLSCREEN)
    {
        // fullscreen now?
        if (glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) 
        {
            // leave fullscreen
            glutLeaveGameMode();

            // trace
            tb_trace_d("fullscreen: leave: %d", impl->id);
        }

        // exit the previous window first
        if (impl->id) glutDestroyWindow(impl->id);
        impl->id = 0;

        // make window
        impl->id = glutCreateWindow(impl->base.info.title? impl->base.info.title : "");
        tb_assert_abort(impl->id && impl->id < tb_arrayn(g_windows));

        // save window
        g_windows[impl->id] = impl;

        // init window func
        glutDisplayFunc(gb_window_glut_display);
        glutReshapeFunc(gb_window_glut_reshape);
        glutKeyboardFunc(gb_window_glut_keyboard);
        glutSpecialFunc(gb_window_glut_special);
        glutMouseFunc(gb_window_glut_mouse);
        glutPassiveMotionFunc(gb_window_glut_passive_motion);
        glutMotionFunc(gb_window_glut_motion);
        glutTimerFunc(1000 / impl->base.info.framerate, gb_window_glut_timer, impl->id);
#ifndef TB_CONFIG_OS_WINDOWS
        glutWMCloseFunc(gb_window_glut_close);
#endif

        // update flag
        impl->base.flag &= ~GB_WINDOW_FLAG_FULLSCREEN;
    }
}
/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
gb_window_ref_t gb_window_init_glut(gb_window_info_t const* info, tb_size_t width, tb_size_t height, tb_size_t flag)
{
    // done
    tb_bool_t               ok = tb_false;
    gb_window_glut_impl_t*  impl = tb_null;
    do
    {
        // check
        tb_assert_and_check_break(info && info->framerate);
        tb_assert_and_check_break(width && width <= GB_WIDTH_MAXN && height && height <= GB_HEIGHT_MAXN);

        // make window
        impl = tb_malloc0_type(gb_window_glut_impl_t);
        tb_assert_and_check_break(impl);

        // init base
        impl->base.type         = GB_WINDOW_TYPE_GLUT;
        impl->base.mode         = GB_WINDOW_MODE_GL;
        impl->base.flag         = flag;
        impl->base.width        = (tb_uint16_t)width;
        impl->base.height       = (tb_uint16_t)height;
        impl->base.loop         = gb_window_glut_loop;
        impl->base.exit         = gb_window_glut_exit;
        impl->base.fullscreen   = gb_window_glut_fullscreen;
        impl->base.info         = *info;

        /* init pixfmt
         * 
         * supports: 
         * - rgba8888_be
         * - rgb565_le
         * - rgb888_be
         * - rgba4444_be
         * - rgba5551_le
         */
        impl->base.pixfmt   = gb_quality() < GB_QUALITY_TOP? GB_PIXFMT_RGB565 : (GB_PIXFMT_RGBA8888 | GB_PIXFMT_BENDIAN);

        // init loop
        impl->stop = 0;

        // init glut
        tb_int_t    argc = 1;
        tb_char_t*  argv[] = {"", tb_null};
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_STENCIL | GLUT_MULTISAMPLE);
        glutInitWindowPosition(0, 0);
        glutInitWindowSize(impl->base.width, impl->base.height);

        // check: not implementation
        tb_assert(!(flag & GB_WINDOW_FLAG_MAXIMUM));
        tb_assert(!(flag & GB_WINDOW_FLAG_MINIMUM));
        tb_assert(!(flag & GB_WINDOW_FLAG_HIHE));
        tb_assert(!(flag & GB_WINDOW_FLAG_HIHE_TITLE));
        tb_assert(!(flag & GB_WINDOW_FLAG_NOT_REISZE));

        // ok
        ok = tb_true;

    } while (0);

    // failed?
    if (!ok)
    {
        // exit it
        if (impl) gb_window_exit((gb_window_ref_t)impl);
        impl = tb_null;
    }

    // ok?
    return (gb_window_ref_t)impl;
}