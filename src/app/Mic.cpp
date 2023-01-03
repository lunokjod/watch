#include <Arduino.h>
#include "lunokiot_config.hpp"
#include <LilyGoWatch.h>
#include <driver/i2s.h>
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "Mic.hpp"

// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2s.html#pdm-mode-tx

extern TTGOClass *ttgo;
#define BUFFER_SIZE (2*1024)

// https://www.theunterminatedstring.com/probing-pdm/

// TWATCH 2020 V3 PDM microphone pin
#define MIC_DATA            2
#define MIC_CLOCK           0

uint8_t buffer[BUFFER_SIZE] = {0};

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


void MicApplication::_RecordThread(void *args) {
    lAppLog("Mic thread start\n");
    MicApplication * self = (MicApplication *)args;
    self->recThread=true;
    self->recThreadDead = false;
    i2s_config_t i2s_config;
    i2s_pin_config_t i2s_cfg;
    // https://www.adobe.com/ae_en/creativecloud/video/discover/audio-sampling.html
    i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate =  44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        //.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB),
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

    while ( self->recThread ) {
        delay(5);

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

            float smoothValue = val_avg;
            if ( smoothValue > 1000 ) { smoothValue /= 10.0; }
            else if ( smoothValue > 800 ) { smoothValue /= 8.0; }
            else if ( smoothValue > 200 ) { smoothValue /= 4.0; }
            else if ( smoothValue > 100 ) { smoothValue /= 2.0; }
            else if ( smoothValue < 10 ) { smoothValue *=4.0; }
            else if ( smoothValue < 30 ) { smoothValue *=2.0; }
            //if ( 0 != smoothValue) {
                Serial.printf("Value: %f smooth: %d\n",val_avg,int(smoothValue));
                self->audioWaveGraph->PushValue(int(smoothValue));
            //}

            val_avg = 0;
            val_max = 0;

            val_avg_1 = 0;
            val_max_1 = 0;

            all_val_avg = 0;
            all_val_zero1 = 0;
            all_val_zero2 = 0;
            all_val_zero3 = 0;
        }

    }
    i2s_driver_uninstall(I2S_NUM_0);
    lAppLog("Mic thread stop\n");
    self->recThreadDead = true;
    vTaskDelete(NULL);
}
MicApplication::MicApplication() {
    // Init here any you need
    lAppLog("Hello from MicApplication!\n");
    audioWaveGraph = new GraphWidget(60,160,0,200,TFT_GREEN,TFT_BLACK,TFT_RED);
    xTaskCreatePinnedToCore(MicApplication::_RecordThread, "", LUNOKIOT_TASK_STACK_SIZE, this, uxTaskPriorityGet(NULL), NULL,0);

    // compose the background
    canvas->fillSprite(ThCol(background)); // use theme colors
    audioWaveGraph->DrawTo(canvas,40,40);
    btnBack->DrawTo(canvas); // draw backbutton from TemplateApplication
}

MicApplication::~MicApplication() {
    // please remove/delete/free all to avoid leaks!
    lAppLog("Mic: trying to stop thread\n");
    this->recThread = false;
    while (false == this->recThreadDead) {
        delay(1000);
        lAppLog("Mic: Waiting thread return...\n");
    }
    delete audioWaveGraph;
    lAppLog("Mic: says goodbye!!!\n");
}

bool MicApplication::Tick() {
    bool interacted = TemplateApplication::Tick(); // calls to TemplateApplication::Tick()
    if ( interacted ) { return true; } // back button pressed
    
    // put your interacts here:
    //mywidget->Interact(touched,touchX,touchY);

    if ( millis() > graphRedraw ) {
        audioWaveGraph->DirectDraw(40,40);
        graphRedraw=millis()+(1000/12);
    }
    return false;
}
