#include <Arduino.h>
#include "lunokiot_config.hpp"
#include <LilyGoWatch.h>
#include <driver/i2s.h>
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "Mic.hpp"


#define BUFFER_SIZE (2*1024)

// https://www.theunterminatedstring.com/probing-pdm/

// TWATCH 2020 V3 PDM microphone pin
#define MIC_DATA            2
#define MIC_CLOCK           0

uint8_t buffer[BUFFER_SIZE] = {0};

const int           define_max = 200;
const int           define_avg = 15;
const int           define_zero = 3000;
float               val_avg = 0;
float               val_avg_1 = 0;
float               all_val_avg = 0;

uint8_t             val1, val2;
int16_t             val16 = 0;
int16_t             val_max = 0;
int16_t             val_max_1 = 0;
int32_t             all_val_zero1 = 0;
int32_t             all_val_zero2 = 0;
int32_t             all_val_zero3 = 0;
uint32_t            j = 0;


MicApplication::MicApplication() {
    // Init here any you need
    lAppLog("Hello from MicApplication!\n");
    audioWaveGraph = new GraphWidget(80,160,0,200,TFT_GREEN,TFT_BLACK,TFT_RED);

    i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate =  44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        //.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S|I2S_COMM_FORMAT_STAND_MSB),
        //.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
        //.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
        .dma_buf_len = 128,
    };

    //i2s_pin_config_t i2s_cfg;
    i2s_cfg.bck_io_num   = I2S_PIN_NO_CHANGE;
    i2s_cfg.ws_io_num    = MIC_CLOCK;
    i2s_cfg.data_out_num = I2S_PIN_NO_CHANGE;
    i2s_cfg.data_in_num  = MIC_DATA;

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &i2s_cfg);
    i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);

    Tick(); // OR call this if no splash 
}

MicApplication::~MicApplication() {
    // please remove/delete/free all to avoid leaks!
    delete audioWaveGraph;
    i2s_driver_uninstall(I2S_NUM_0);
}

bool MicApplication::Tick() {
    bool interacted = TemplateApplication::Tick(); // calls to TemplateApplication::Tick()
    if ( interacted ) { return true; } // back button pressed
    
    // put your interacts here:
    //mywidget->Interact(touched,touchX,touchY);


    if ( millis() > nextRefresh ) {
        canvas->fillSprite(ThCol(background)); // use theme colors
        // draw your interface widgets here!!!
        audioWaveGraph->DrawTo(canvas,10,20);

        TemplateApplication::Tick(); // redraw back button
        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }

    size_t read_len = 0;
    j = j + 1;
    i2s_read(I2S_NUM_0, (char *) buffer, BUFFER_SIZE, &read_len, portMAX_DELAY);
    for (int i = 0; i < BUFFER_SIZE / 2 ; i++) {
        val1 = buffer[i * 2];
        val2 = buffer[i * 2 + 1] ;
        val16 = val1 + val2 *  256;
        if (val16 > 0) {
            val_avg = val_avg + val16;
            val_max = max( val_max, val16);
        }
        if (val16 < 0) {
            val_avg_1 = val_avg_1 + val16;
            val_max_1 = min( val_max_1, val16);
        }
        all_val_avg = all_val_avg + val16;
        if (abs(val16) >= 20)
            all_val_zero1 = all_val_zero1 + 1;
        if (abs(val16) >= 15)
            all_val_zero2 = all_val_zero2 + 1;
        if (abs(val16) > 5)
            all_val_zero3 = all_val_zero3 + 1;
    }

    if (j % 2 == 0 && j > 0) {
        val_avg = val_avg / BUFFER_SIZE ;
        val_avg_1 = val_avg_1 / BUFFER_SIZE;
        all_val_avg = all_val_avg / BUFFER_SIZE ;

        audioWaveGraph->PushValue(val_avg);
        lAppLog("Value: %f %d\n",val_avg,int(val_avg));

        val_avg = 0;
        val_max = 0;

        val_avg_1 = 0;
        val_max_1 = 0;

        all_val_avg = 0;
        all_val_zero1 = 0;
        all_val_zero2 = 0;
        all_val_zero3 = 0;
    }
    return false;
}
