#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/drm.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

static lv_color_t * buf_1;
static lv_color_t * buf_2;

int main(void)
{
    /*LittlevGL init*/
    lv_init();

    LV_LOG_INFO("disp_init....");
    drm_init();

    lv_coord_t hor_res = 0, ver_res = 0;
    uint32_t dpi = 0;
    drm_get_sizes(&hor_res, &ver_res, &dpi);
    LV_LOG_INFO("drm_get_sizes %dx %d , dpi %d \n", hor_res, ver_res, dpi);

    static lv_disp_draw_buf_t draw_buf;

    buf_1 = (lv_color_t *)calloc(hor_res * ver_res, sizeof(lv_color_t));
    buf_2 = (lv_color_t *)calloc(hor_res * ver_res, sizeof(lv_color_t));

    // static lv_color_t buf_1[hor_res * ver_res];            /*A screen sized buffer*/
    // static lv_color_t buf_2[hor_res * ver_res];            /*Another screen sized buffer*/
    lv_disp_draw_buf_init(&draw_buf, buf_1, buf_2, hor_res * ver_res); /*Initialize the display buffer*/

    static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);   /*Basic initialization*/

    disp_drv.hor_res      = hor_res;
    disp_drv.ver_res      = ver_res;
    disp_drv.dpi          = dpi;
    disp_drv.flush_cb     = drm_flush;
    disp_drv.draw_buf     = &draw_buf;
    disp_drv.full_refresh = 1;

    /*Finally register the driver*/
    lv_disp_t * disp = lv_disp_drv_register(&disp_drv);
    lv_disp_set_default(disp);

    evdev_init();
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_1.read_cb = evdev_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);


    /*Set a cursor for the mouse*/
    LV_IMG_DECLARE(mouse_cursor_icon)
    lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
    lv_indev_set_cursor(mouse_indev, cursor_obj);             /*Connect the image  object to the driver*/


    /*Create a Demo*/
    lv_demo_music();

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        usleep(lv_timer_handler() * 1000);
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
