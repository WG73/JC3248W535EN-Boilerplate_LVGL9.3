#include <Arduino.h>
#include <lvgl.h>
#include "pincfg.h"
#include "dispcfg.h"
#include "AXS15231B_touch.h"
#include <Arduino_GFX_Library.h>

Arduino_DataBus *bus = new Arduino_ESP32QSPI(TFT_CS, TFT_SCK, TFT_SDA0, TFT_SDA1, TFT_SDA2, TFT_SDA3);
Arduino_GFX *g = new Arduino_AXS15231B(bus, GFX_NOT_DEFINED, 0, false, TFT_res_W, TFT_res_H);
Arduino_Canvas *gfx = new Arduino_Canvas(TFT_res_W, TFT_res_H, g, 0, 0, TFT_rot);
AXS15231B_Touch touch(Touch_SCL, Touch_SDA, Touch_INT, Touch_ADDR, TFT_rot);


// LVGL calls it when a rendered image needs to copied to the display
void my_flush_cb(lv_display_t  *disp, const lv_area_t *area, uint8_t *color_p) {
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)color_p, w, h);
    // Call it to tell LVGL everthing is ready
    lv_disp_flush_ready(disp);

}

// Read the touchpad
void my_touchpad_read_cb(lv_indev_t  *indev, lv_indev_data_t *data) {
    static uint16_t last_x = 0, last_y = 0;
    if (touch.touched()) {
		uint16_t x, y;
        touch.readData(&x, &y);
        last_x = x;
        last_y = y;
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->point.x = last_x;
        data->point.y = last_y;
        data->state = LV_INDEV_STATE_RELEASED;
		data->continue_reading = false;
    }
}


inline void init_lvgl(){
	Serial.println("Arduino_GFX LVGL ");
    String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch() + " example";
    Serial.println(LVGL_Arduino);


    // Display setup
    if(!gfx->begin(40000000UL)) {
        Serial.println("Failed to initialize display!");
        return;
    }
    gfx->fillScreen(BLACK);

    // Switch backlight on
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

/**/
    // Touch setup
    if(!touch.begin()) {
        Serial.println("Failed to initialize touch module!");
        return;
    }
    touch.enOffsetCorrection(true);
    touch.setOffsets(Touch_X_min, Touch_X_max, TFT_res_W-1, Touch_Y_min, Touch_Y_max, TFT_res_H-1);

    // Init LVGL
    lv_init();


	// Initialize the display buffer
	lv_display_t * display1 = lv_display_create(TFT_res_H, TFT_res_W);
	#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))
	static uint8_t buf1[TFT_res_H * TFT_res_W / 10 * BYTES_PER_PIXEL];
	lv_display_set_buffers(display1, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_flush_cb(display1, my_flush_cb);

/**/
    // Initialize the input device driver
	/* Create and set up at least one display before you register any input devices. */
	lv_indev_t * indev = lv_indev_create();        /* Create input device connected to Default Display. */
	lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);   /* Touch pad is a pointer-like device. */
	lv_indev_set_read_cb(indev, my_touchpad_read_cb);    /* Set driver function. */

};

// Event callback function must be defined at file scope, not inside setup()
static void btn_event_cb(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	auto * btn = (lv_obj_t *) lv_event_get_target(e);
	auto * label2 = (lv_obj_t *)lv_obj_get_user_data(btn);
	if(code == LV_EVENT_RELEASED) {
		lv_label_set_text(label2, "Dont do this again!11!!");
	}else if(code == LV_EVENT_PRESSED) {
		lv_label_set_text(label2, "BOOM PENG CRUSH!!!!");
	}
    
}

void setup() {
	Serial.begin(115200);
	while(!Serial);
	init_lvgl();

	lv_obj_set_style_bg_color(lv_scr_act(), lv_palette_main(LV_PALETTE_GREEN),0);
	lv_obj_t * btn_cont = lv_btn_create(lv_scr_act());
	lv_obj_set_size(btn_cont, 240, 60);
	lv_obj_align(btn_cont, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_bg_color(btn_cont, lv_palette_main(LV_PALETTE_RED), 0);
	lv_obj_t * label = lv_label_create(btn_cont);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
	lv_label_set_text(label, "Big RED Button");
	lv_obj_center(label);	
	
    lv_obj_t * label2 = lv_label_create(lv_scr_act());
    lv_label_set_text(label2, "Dont push the button!");
	lv_obj_align(label2, LV_ALIGN_TOP_MID, 0, 40);	
	lv_obj_set_style_text_font(label2, &lv_font_montserrat_30, 0);
	lv_obj_set_style_text_color(label2, lv_palette_main(LV_PALETTE_NONE), 0);	

    // Set label2 as user data for the button
    lv_obj_set_user_data(btn_cont, label2);

    // Register the event callback function
    lv_obj_add_event_cb(btn_cont, btn_event_cb, LV_EVENT_PRESSED , 0);
	lv_obj_add_event_cb(btn_cont, btn_event_cb, LV_EVENT_RELEASED , 0);

}



void loop() {	
	lv_task_handler();
	lv_tick_inc(5);
	gfx->flush();
	delay(5);
}


