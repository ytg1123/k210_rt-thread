#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "lvgl.h"

#include "board_config.h"
#include "fpioa.h"
#include "image.h"
#include "lcd.h"
#include "nt35310.h"
#include "sysctl.h"
#include "unistd.h"

#include "../lv_misc/lv_color.h"


static void io_set_power(void) {
  sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
  sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

static void io_mux_init(void) {
  fpioa_set_function(38, FUNC_GPIOHS0 + DCX_GPIONUM);
  fpioa_set_function(36, FUNC_SPI0_SS3);
  fpioa_set_function(39, FUNC_SPI0_SCLK);
  fpioa_set_function(37, FUNC_GPIOHS0 + RST_GPIONUM);
  sysctl_set_spi0_dvp_data(1);
}

void lcd_draw_picture_by_half(uint16_t x1, uint16_t y1, uint16_t width,
                              uint16_t height, uint16_t* ptr) {
  lcd_set_area(x1, y1, x1 + width - 1, y1 + height - 1);
  tft_write_half(ptr, width * height);
}

static uint32_t lcd_gram[LCD_X_MAX * LCD_Y_MAX / 2] __attribute__((aligned(128)));
static uint8_t img[LCD_X_MAX * LCD_Y_MAX * 2];

static void disp_flush(lv_disp_drv_t* disp_drv, const lv_area_t* area,
                       lv_color_t* color_p) {
  /*The most simple case (but also the slowest) to put all pixels to the screen
   * one-by-one*/
  uint16_t x1, x2, y1, y2;
  y1 = area->y1;
  y2 = area->y2;
  x1 = area->x1;
  x2 = area->x2;

  lcd_draw_picture_by_half((uint16_t)x1, (uint16_t)y1, (uint16_t)(x2 - x1 + 1),
                           (uint16_t)(y2 - y1 + 1), (uint16_t*)color_p);
  /* IMPORTANT!!!
   * Inform the graphics library that you are ready with the flushing*/
  lv_disp_flush_ready(disp_drv);
}

//static void disp_init() {
void disp_init() {
  lv_disp_drv_t disp_drv;  
  lv_disp_drv_init(&disp_drv);

  static lv_disp_buf_t disp_buf_1;  
  static lv_color_t buf1_1[LV_HOR_RES_MAX * 50]; /*A buffer for 10 rows*/
  lv_disp_buf_init(&disp_buf_1, buf1_1, NULL,
                   LV_HOR_RES_MAX * 50); 
  //lv_conf.h 
  /*Set the resolution of the display*/
  disp_drv.hor_res = LV_HOR_RES_MAX;
  disp_drv.ver_res = LV_VER_RES_MAX;
  // 
  /*Set a display buffer*/
  disp_drv.buffer = &disp_buf_1;
  // 
  /*Used to copy the buffer's content to the display*/
  disp_drv.flush_cb = disp_flush;
  // 
  /*Finally register the driver*/
  lv_disp_drv_register(&disp_drv);
}

void lv_test2()
{
  lv_obj_t* label2 = lv_label_create(lv_scr_act(), NULL);
  //lv_obj_set_style_local_text_font(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_simsun_16_cjk);
  lv_obj_set_style_local_text_font(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
  lv_obj_set_pos(label2, 100, 100);
  //  ?ꎵ  simsun_16_cjk font
  //lv_label_set_text(label2, "\xE4\xB8\x80\xE4\xB8\x83\xE4\xB8\x80\xE4\xB8\x83\xE4\xB8\x80\xE4\xB8\x83\xE4\xB8\x80\xE4\xB8\x83");
  // ?Y?C?L
  lv_label_set_text(label2, "\xE3\x82\xBA\xE3\x82\xA4\xE3\x82\xAD");
}

void lv_tutorial_hello_world(void) {
  static lv_style_t style_txt;                  /*Declare a new style. Should be `static`*/

  //lv_style_init(&style_txt);

  /* Create the main label */
  lv_obj_t* label = lv_label_create(lv_scr_act(), NULL);
  //lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_simsun_16_cjk);
  //lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_ipa_gothic_16);
  //lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_ipa_pgothic_16);
  //lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_ipa_mincho_16);
  //lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_ipa_pmincho_16);

  //lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_SMALL);
  //lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_NORMAL);
  //lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_SUBTITLE);
  lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);

  lv_obj_set_pos(label, 0, 0);

  // UTF-8 kabusikikaisya zuiki 株式会社瑞起
  //lv_label_set_text(label, "\xE6\xA0\xAA\xE5\xBC\x8F\xE4\xBC\x9A\xE7\xA4\xBE\xE7\x91\x9E\xE8\xB5\xB7");

  // UTF-8 chinese like zuiki kabusikikaisya 瑞起股份公司
  //lv_label_set_text(label,"\xE7\x91\x9E\xE8\xB5\xB7\xE8\x82\xA1\xE4\xBB\xBD\xE5\x85\xAC\xE5\x8F\xB8");
  lv_label_set_text(label,"瑞起股份公司");

  // UTF-8 kabusikikaisya zuiki
  //lv_label_set_text(label,"株式会社瑞起");

  lv_obj_t* scr = lv_disp_get_scr_act(NULL);
  lv_obj_t* label3 = lv_label_create(scr, NULL);
  lv_label_set_text(label3, "QingYeMuRong");
  lv_obj_align(label3, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_pos(label3, 20, 20);

  /* button controll create for button1 */
  lv_obj_t* btn1 = lv_btn_create(scr, NULL);
  lv_obj_t* label1 = lv_label_create(btn1, NULL);
  // kanji zuiki "瑞起"
  //lv_label_set_text(label1, "\xE7\x91\x9E\xE8\xB5\xB7");
  lv_label_set_text(label1, "瑞起");
  // katakana zuiki ズイキ
  //lv_label_set_text(label1, "\xE3\x82\xBA\xE3\x82\xA4\xE3\x82\xAD");

  lv_obj_set_pos(btn1, 20, 170);

  /* button controll create for button2 */
  lv_obj_t* btn2 = lv_btn_create(scr, NULL);      /* btn */
  lv_obj_t* label2 = lv_label_create(btn2, NULL); /* label */
  // katakana front フロント
  //lv_label_set_text(label2, "\xE3\x83\x95\xE3\x83\xAD\xE3\x83\xB3\xE3\x83\x88");
  lv_label_set_text(label2, "フロント");
  lv_obj_set_pos(btn2, 160, 170);                 /* set button*/

}

int lvgl_main(void) {
  printf("lcd test\n");
  io_mux_init();
  io_set_power();
  lcd_init();
  lv_init();
  disp_init();
  lcd_draw_picture(0, 0, 320, 240, rgb_image);
  lcd_set_direction(DIR_YX_RLDU); /* left up 0,0 */
  // usleep(1000000);
  printf("show pic\n");
  lv_tutorial_hello_world();
  while (1) {
    lv_task_handler();
    lv_tick_inc(1);  // lvgl 时基函数
  }
}
