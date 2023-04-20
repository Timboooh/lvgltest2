#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/drm.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#define DISP_BUF_SIZE (128 * 1024)

    #define MY_DISP_HOR_RES    1024
    #define MY_DISP_VER_RES    800

static bool disp_flush_enabled = true;
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
	    if(disp_flush_enabled) {
	        drm_flush(disp_drv, area, color_p);
   			LV_LOG_INFO("....disp_flush....");
	    }
	
	    /*IMPORTANT!!!
	     *Inform the graphics library that you are ready with the flushing*/
	    lv_disp_flush_ready(disp_drv);
}

int main(void)
{
    /*LittlevGL init*/
    lv_init();
    
	LV_LOG_INFO("disp_init....");
    drm_init();

    /* Example for 3) also set disp_drv.full_refresh = 1 below*/
    static lv_disp_draw_buf_t draw_buf_dsc_3;
    static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*A screen sized buffer*/
    static lv_color_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*Another screen sized buffer*/
    lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2,
                          MY_DISP_HOR_RES * MY_DISP_VER_RES);   /*Initialize the display buffer*/


    
    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/
    lv_coord_t w=0,h=0;
    uint32_t d=0;
    drm_get_sizes(&w,&h,&d);
    LV_LOG_INFO("drm_get_sizes %dx %d , dpi %d \n", w, h, d );
    /*Set the resolution of the display*/
    disp_drv.hor_res = w;
    disp_drv.ver_res = h;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_3;

    /*Required for Example 3)*/
    disp_drv.full_refresh = 1;

    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.*/
    //disp_drv.gpu_fill_cb = gpu_fill;

    /*Finally register the driver*/
    lv_disp_t *disp=lv_disp_drv_register(&disp_drv);
    lv_disp_set_default(disp);

    // /*Linux frame buffer device init*/

    // /*A small buffer for LittlevGL to draw the screen's content*/
    // static lv_color_t buf[DISP_BUF_SIZE];

    // /*Initialize a descriptor for the buffer*/
    // static lv_disp_draw_buf_t disp_buf;
    // lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    // /*Initialize and register a display driver*/
    // uint32_t dpi = 0;
    // static lv_disp_drv_t disp_drv;
    // lv_disp_drv_init(&disp_drv);

    // drm_get_sizes(&disp_drv.hor_res, &disp_drv.ver_res, &dpi);

    // disp_drv.draw_buf   = &disp_buf;
    // disp_drv.flush_cb = drm_flush;
    // disp_drv.wait_cb  = drm_wait_vsync;
    // lv_disp_drv_register(&disp_drv);

    /*Create a Demo*/
    //lv_demo_widgets();

    lv_demo_music();

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        usleep(lv_timer_handler()* 1000);
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
