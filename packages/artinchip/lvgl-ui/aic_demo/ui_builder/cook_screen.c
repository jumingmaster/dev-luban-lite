/*
 * Copyright (C) 2025 ArtInChip Technology Co., Ltd.
 *
 * This file was generated by AiUIBuilder
 * AiUIBuilder version: v1.2.0
 */

#include "ui_objects.h"
#include "aic_ui.h"
#include "ui_util.h"

void __attribute__((weak)) cook_screen_rec1_cont_custom_clicked(void) {
}

static void cook_screen_rec1_cont_clicked (lv_event_t *e) {
    cook_screen_rec1_cont_custom_clicked();
}

void __attribute__((weak)) cook_screen_rec2_cont_custom_clicked(void) {
}

static void cook_screen_rec2_cont_clicked (lv_event_t *e) {
    cook_screen_rec2_cont_custom_clicked();
}

void __attribute__((weak)) cook_screen_rec3_cont_custom_clicked(void) {
}

static void cook_screen_rec3_cont_clicked (lv_event_t *e) {
    cook_screen_rec3_cont_custom_clicked();
}

void __attribute__((weak)) cook_screen_rec4_cont_custom_clicked(void) {
}

static void cook_screen_rec4_cont_clicked (lv_event_t *e) {
    cook_screen_rec4_cont_custom_clicked();
}

void __attribute__((weak)) cook_screen_rec5_cont_custom_clicked(void) {
}

static void cook_screen_rec5_cont_clicked (lv_event_t *e) {
    cook_screen_rec5_cont_custom_clicked();
}

void __attribute__((weak)) cook_screen_rec6_cont_custom_clicked(void) {
}

static void cook_screen_rec6_cont_clicked (lv_event_t *e) {
    cook_screen_rec6_cont_custom_clicked();
}

void __attribute__((weak)) cook_screen_cook_back_custom_clicked(void) {
}

static void cook_screen_cook_back_clicked (lv_event_t *e) {
    cook_screen_cook_back_custom_clicked();
}


void cook_screen_create(ui_manager_t *ui)
{
    cook_screen_t *scr = cook_screen_get(ui);

    if (!ui->auto_del && scr->obj)
        return;

    // Init scr->obj
    scr->obj = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(scr->obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(scr->obj, LV_OBJ_FLAG_SCROLLABLE);

    // Set style of scr->obj
    lv_obj_set_style_bg_color(scr->obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->cook_cont
    scr->cook_cont = lv_obj_create(scr->obj);
    lv_obj_set_pos(scr->cook_cont, 20, 170);
    lv_obj_set_size(scr->cook_cont, 1240, 520);
    lv_obj_set_scrollbar_mode(scr->cook_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(scr->cook_cont, LV_OBJ_FLAG_SCROLLABLE);

    // Set style of scr->cook_cont
    lv_obj_set_style_bg_color(scr->cook_cont, lv_color_hex(0xeff8f5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->cook_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->cook_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->cook_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->cook_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->cook_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->cook_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->cook_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->rec1_cont
    scr->rec1_cont = lv_obj_create(scr->cook_cont);
    lv_obj_set_pos(scr->rec1_cont, 0, 0);
    lv_obj_set_size(scr->rec1_cont, 455, 520);
    lv_obj_set_scrollbar_mode(scr->rec1_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(scr->rec1_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->rec1_cont, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Set style of scr->rec1_cont
    lv_obj_set_style_bg_color(scr->rec1_cont, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->rec1_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->rec1_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->rec1_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->rec1_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->rec1_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->rec1_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->rec1_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Set event handler of scr->rec1_cont
    lv_obj_add_event_cb(scr->rec1_cont, cook_screen_rec1_cont_clicked, LV_EVENT_CLICKED, NULL);

    // Init scr->cook1_img
    scr->cook1_img = lv_img_create(scr->rec1_cont);
    lv_img_set_src(scr->cook1_img, LVGL_IMAGE_PATH(beef_q95_455x520.jpeg));
    lv_img_set_pivot(scr->cook1_img, 50, 50);
    lv_img_set_angle(scr->cook1_img, 0);
    lv_obj_set_style_img_opa(scr->cook1_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->cook1_img, 0, 0);
    lv_obj_add_flag(scr->cook1_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->cook1_img, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Init scr->cook1_label
    scr->cook1_label = lv_label_create(scr->rec1_cont);
    lv_label_set_text(scr->cook1_label, "Steak");
    lv_label_set_long_mode(scr->cook1_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->cook1_label, 16, 433);
    lv_obj_set_size(scr->cook1_label, 228, 69);
    lv_obj_add_flag(scr->cook1_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->cook1_label, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Set style of scr->cook1_label
    lv_obj_set_style_text_font(scr->cook1_label, fs_fzltxhjw_70, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->cook1_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->rec2_cont
    scr->rec2_cont = lv_obj_create(scr->cook_cont);
    lv_obj_set_pos(scr->rec2_cont, 455, 0);
    lv_obj_set_size(scr->rec2_cont, 455, 520);
    lv_obj_set_scrollbar_mode(scr->rec2_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(scr->rec2_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->rec2_cont, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Set style of scr->rec2_cont
    lv_obj_set_style_bg_color(scr->rec2_cont, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->rec2_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->rec2_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->rec2_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->rec2_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->rec2_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->rec2_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->rec2_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Set event handler of scr->rec2_cont
    lv_obj_add_event_cb(scr->rec2_cont, cook_screen_rec2_cont_clicked, LV_EVENT_CLICKED, NULL);

    // Init scr->cook2_img
    scr->cook2_img = lv_img_create(scr->rec2_cont);
    lv_img_set_src(scr->cook2_img, LVGL_IMAGE_PATH(pancake_q95_455x520.jpeg));
    lv_img_set_pivot(scr->cook2_img, 50, 50);
    lv_img_set_angle(scr->cook2_img, 0);
    lv_obj_set_style_img_opa(scr->cook2_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->cook2_img, 0, 0);
    lv_obj_add_flag(scr->cook2_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->cook2_img, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Init scr->cook2_label
    scr->cook2_label = lv_label_create(scr->rec2_cont);
    lv_label_set_text(scr->cook2_label, "Pancake");
    lv_label_set_long_mode(scr->cook2_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->cook2_label, 16, 433);
    lv_obj_set_size(scr->cook2_label, 310, 69);
    lv_obj_add_flag(scr->cook2_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->cook2_label, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Set style of scr->cook2_label
    lv_obj_set_style_text_font(scr->cook2_label, fs_fzltxhjw_70, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->cook2_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->rec3_cont
    scr->rec3_cont = lv_obj_create(scr->cook_cont);
    lv_obj_set_pos(scr->rec3_cont, 910, 0);
    lv_obj_set_size(scr->rec3_cont, 455, 520);
    lv_obj_set_scrollbar_mode(scr->rec3_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(scr->rec3_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->rec3_cont, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Set style of scr->rec3_cont
    lv_obj_set_style_bg_color(scr->rec3_cont, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->rec3_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->rec3_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->rec3_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->rec3_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->rec3_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->rec3_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->rec3_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Set event handler of scr->rec3_cont
    lv_obj_add_event_cb(scr->rec3_cont, cook_screen_rec3_cont_clicked, LV_EVENT_CLICKED, NULL);

    // Init scr->cook3_image
    scr->cook3_image = lv_img_create(scr->rec3_cont);
    lv_img_set_src(scr->cook3_image, LVGL_IMAGE_PATH(chocolate_q95_455x520.jpeg));
    lv_img_set_pivot(scr->cook3_image, 50, 50);
    lv_img_set_angle(scr->cook3_image, 0);
    lv_obj_set_style_img_opa(scr->cook3_image, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->cook3_image, 0, 0);
    lv_obj_add_flag(scr->cook3_image, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->cook3_image, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Init scr->cook3_label
    scr->cook3_label = lv_label_create(scr->rec3_cont);
    lv_label_set_text(scr->cook3_label, "Melting\nChocolate");
    lv_label_set_long_mode(scr->cook3_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->cook3_label, 16, 350);
    lv_obj_set_size(scr->cook3_label, 370, 156);
    lv_obj_add_flag(scr->cook3_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->cook3_label, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Set style of scr->cook3_label
    lv_obj_set_style_text_font(scr->cook3_label, fs_fzltxhjw_70, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->cook3_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->rec4_cont
    scr->rec4_cont = lv_obj_create(scr->cook_cont);
    lv_obj_set_pos(scr->rec4_cont, 1365, 0);
    lv_obj_set_size(scr->rec4_cont, 455, 520);
    lv_obj_set_scrollbar_mode(scr->rec4_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(scr->rec4_cont, LV_OBJ_FLAG_CLICKABLE);

    // Set style of scr->rec4_cont
    lv_obj_set_style_bg_color(scr->rec4_cont, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->rec4_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->rec4_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->rec4_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->rec4_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->rec4_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->rec4_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->rec4_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Set event handler of scr->rec4_cont
    lv_obj_add_event_cb(scr->rec4_cont, cook_screen_rec4_cont_clicked, LV_EVENT_CLICKED, NULL);

    // Init scr->cook4_image
    scr->cook4_image = lv_img_create(scr->rec4_cont);
    lv_img_set_src(scr->cook4_image, LVGL_IMAGE_PATH(chips_q95_455x520.jpeg));
    lv_img_set_pivot(scr->cook4_image, 50, 50);
    lv_img_set_angle(scr->cook4_image, 0);
    lv_obj_set_style_img_opa(scr->cook4_image, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->cook4_image, 0, 0);
    lv_obj_add_flag(scr->cook4_image, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->cook4_image, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Init scr->cook4_label
    scr->cook4_label = lv_label_create(scr->rec4_cont);
    lv_label_set_text(scr->cook4_label, "Chips");
    lv_label_set_long_mode(scr->cook4_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->cook4_label, 16, 433);
    lv_obj_set_size(scr->cook4_label, 228, 69);
    lv_obj_add_flag(scr->cook4_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->cook4_label, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Set style of scr->cook4_label
    lv_obj_set_style_text_font(scr->cook4_label, fs_fzltxhjw_70, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->cook4_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->rec5_cont
    scr->rec5_cont = lv_obj_create(scr->cook_cont);
    lv_obj_set_pos(scr->rec5_cont, 1820, 0);
    lv_obj_set_size(scr->rec5_cont, 455, 520);
    lv_obj_set_scrollbar_mode(scr->rec5_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(scr->rec5_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->rec5_cont, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Set style of scr->rec5_cont
    lv_obj_set_style_bg_color(scr->rec5_cont, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->rec5_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->rec5_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->rec5_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->rec5_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->rec5_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->rec5_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->rec5_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Set event handler of scr->rec5_cont
    lv_obj_add_event_cb(scr->rec5_cont, cook_screen_rec5_cont_clicked, LV_EVENT_CLICKED, NULL);

    // Init scr->rec5_image
    scr->rec5_image = lv_img_create(scr->rec5_cont);
    lv_img_set_src(scr->rec5_image, LVGL_IMAGE_PATH(omelette_q95_455x520.jpeg));
    lv_img_set_pivot(scr->rec5_image, 50, 50);
    lv_img_set_angle(scr->rec5_image, 0);
    lv_obj_set_style_img_opa(scr->rec5_image, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->rec5_image, 0, 0);
    lv_obj_add_flag(scr->rec5_image, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->rec5_image, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Init scr->cook5_label
    scr->cook5_label = lv_label_create(scr->rec5_cont);
    lv_label_set_text(scr->cook5_label, "Omelette");
    lv_label_set_long_mode(scr->cook5_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->cook5_label, 16, 433);
    lv_obj_set_size(scr->cook5_label, 327, 71);
    lv_obj_add_flag(scr->cook5_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->cook5_label, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Set style of scr->cook5_label
    lv_obj_set_style_text_font(scr->cook5_label, fs_fzltxhjw_70, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->cook5_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->rec6_cont
    scr->rec6_cont = lv_obj_create(scr->cook_cont);
    lv_obj_set_pos(scr->rec6_cont, 2265, 0);
    lv_obj_set_size(scr->rec6_cont, 455, 520);
    lv_obj_set_scrollbar_mode(scr->rec6_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(scr->rec6_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->rec6_cont, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Set style of scr->rec6_cont
    lv_obj_set_style_bg_color(scr->rec6_cont, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->rec6_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->rec6_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->rec6_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->rec6_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->rec6_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->rec6_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->rec6_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Set event handler of scr->rec6_cont
    lv_obj_add_event_cb(scr->rec6_cont, cook_screen_rec6_cont_clicked, LV_EVENT_CLICKED, NULL);

    // Init scr->cook6_image
    scr->cook6_image = lv_img_create(scr->rec6_cont);
    lv_img_set_src(scr->cook6_image, LVGL_IMAGE_PATH(popcorn_q95_455x520.jpeg));
    lv_img_set_pivot(scr->cook6_image, 50, 50);
    lv_img_set_angle(scr->cook6_image, 0);
    lv_obj_set_style_img_opa(scr->cook6_image, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->cook6_image, 0, 0);
    lv_obj_add_flag(scr->cook6_image, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->cook6_image, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Init scr->cook6_label
    scr->cook6_label = lv_label_create(scr->rec6_cont);
    lv_label_set_text(scr->cook6_label, "Popcorn");
    lv_label_set_long_mode(scr->cook6_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->cook6_label, 16, 433);
    lv_obj_set_size(scr->cook6_label, 295, 69);
    lv_obj_add_flag(scr->cook6_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scr->cook6_label, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Set style of scr->cook6_label
    lv_obj_set_style_text_font(scr->cook6_label, fs_fzltxhjw_70, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->cook6_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->cook_back
    scr->cook_back = lv_img_create(scr->obj);
    lv_img_set_src(scr->cook_back, LVGL_IMAGE_PATH(back_135x135.png));
    lv_img_set_pivot(scr->cook_back, 50, 50);
    lv_img_set_angle(scr->cook_back, 0);
    lv_obj_set_style_img_opa(scr->cook_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->cook_back, 22, 22);
    lv_obj_add_flag(scr->cook_back, LV_OBJ_FLAG_CLICKABLE);

    // Set event handler of scr->cook_back
    lv_obj_add_event_cb(scr->cook_back, cook_screen_cook_back_clicked, LV_EVENT_CLICKED, NULL);

    // Init scr->label_7
    scr->label_7 = lv_label_create(scr->obj);
    lv_label_set_text(scr->label_7, "RECIPES");
    lv_label_set_long_mode(scr->label_7, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->label_7, 168, 28);
    lv_obj_set_size(scr->label_7, 488, 117);

    // Set style of scr->label_7
    lv_obj_set_style_text_font(scr->label_7, fs_fzltxhjw_95, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->label_7, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);


}
