// main.c

#include <stdio.h>
#include <string.h>                     // >>> ADDED
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wasm_export.h"
#include "bh_platform.h"
#include "guest.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#include "esp_wifi.h"                   // >>> ADDED
#include "esp_event.h"                  // >>> ADDED
#include "nvs_flash.h"                  // >>> ADDED
#include "mqtt_client.h"                // >>> ADDED

// --- CONFIG WIFI ---
#define WIFI_SSID     "Preguica"        // >>> MODIFIQUE
#define WIFI_PASS     "$GuiGiPe14"       // >>> MODIFIQUE

// --- CONFIG MQTT ---
#define MQTT_BROKER_URI "mqtt://192.168.0.122"  // >>> COLOQUE IP DA RASPBERRY

static esp_mqtt_client_handle_t mqtt_client = NULL; // >>> ADDED

// --- MAPEAMENTO DE PINOS ---
#define MOISTURE_ADC_CHAN ADC_CHANNEL_4
#define RELAY_GPIO        23
#define LED_RED           18
#define LED_GREEN         19
#define LED_BLUE          21

static adc_oneshot_unit_handle_t adc1_handle = NULL;
#define LOG_TAG "WAMR_IRRIGADOR"



// ============================================================
// WIFI INIT  >>> ADDED BLOCO COMPLETO
// ============================================================

static void wifi_init(void)
{
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        }
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();
}



// ============================================================
// MQTT INIT  >>> ADDED BLOCO COMPLETO
// ============================================================

static void mqtt_event_handler(void *handler_args,
                                esp_event_base_t base,
                                int32_t event_id,
                                void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {

        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("MQTT", "Conectado ao broker!");
            esp_mqtt_client_publish(mqtt_client,
                                    "teste",
                                    "ESP32 online",
                                    0, 1, 0);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE("MQTT", "Erro na conexao MQTT");
            break;

        default:
            break;
    }
}


static void mqtt_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(mqtt_client,
                                   ESP_EVENT_ANY_ID,
                                   mqtt_event_handler,
                                   NULL);

    esp_mqtt_client_start(mqtt_client);
}



// ============================================================
// NATIVE FUNCTIONS
// ============================================================

static int32_t host_read_sensor(wasm_exec_env_t exec_env, int32_t sensor_type) {
    int sum = 0;
    int raw = 0;

    for (int i = 0; i < 16; i++) {
        adc_oneshot_read(adc1_handle, MOISTURE_ADC_CHAN, &raw);
        sum += raw;
        vTaskDelay(pdMS_TO_TICKS(10));   // >>> MODIFIED (remove sys_delay_ms)
    }

    return sum / 16;
}

static void host_set_output(wasm_exec_env_t exec_env, int32_t pin, int32_t state) {
    gpio_set_level((gpio_num_t)pin, state);
}

static void host_delay_ms(wasm_exec_env_t exec_env, int32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void host_log_int(wasm_exec_env_t exec_env, int32_t value) {
    ESP_LOGI("WASM_VAL", "Sensor = %d", value);
}

static void host_print(wasm_exec_env_t exec_env, uint32_t msg_offset) {

    wasm_module_inst_t module_inst =
        wasm_runtime_get_module_inst(exec_env);

    if (!wasm_runtime_validate_app_str_addr(module_inst, msg_offset)) {
        ESP_LOGE("WASM_LOG", "String invalida recebida do WASM");
        return;
    }

    const char *native_msg =
        wasm_runtime_addr_app_to_native(module_inst, msg_offset);

    ESP_LOGI("WASM_LOG", "%s", native_msg);
}



// ============================================================
// MQTT HOST FUNCTION  >>> ADDED
// ============================================================

static int32_t host_mqtt_publish(wasm_exec_env_t exec_env,
                                 uint32_t topic_offset,
                                 uint32_t msg_offset)
{
    if (!mqtt_client) return -1;

    wasm_module_inst_t module_inst =
        wasm_runtime_get_module_inst(exec_env);

    if (!wasm_runtime_validate_app_str_addr(module_inst, topic_offset) ||
        !wasm_runtime_validate_app_str_addr(module_inst, msg_offset))
        return -1;

    const char *topic =
        wasm_runtime_addr_app_to_native(module_inst, topic_offset);

    const char *msg =
        wasm_runtime_addr_app_to_native(module_inst, msg_offset);

    esp_mqtt_client_publish(mqtt_client, topic, msg, 0, 1, 0);

    return 0;
}



// ============================================================
// TABELA DE SIMBOLOS  >>> MODIFIED
// ============================================================

static NativeSymbol extended_native_symbols[] = {
    { "read_sensor", host_read_sensor, "(i)i", NULL },
    { "set_output",  host_set_output,  "(ii)", NULL },
    { "delay_ms",    host_delay_ms,    "(i)",  NULL },
    { "wasm_log",    host_print,       "(i)",  NULL },
    { "log_int",     host_log_int,     "(i)",  NULL },

    { "mqtt_publish", host_mqtt_publish, "(ii)i", NULL }, // >>> ADDED
};



// ============================================================
// HARDWARE INIT (igual)
// ============================================================

void init_hardware() {

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL<<RELAY_GPIO) | (1ULL<<LED_RED) |
                        (1ULL<<LED_GREEN) | (1ULL<<LED_BLUE),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    adc_oneshot_unit_init_cfg_t init_config = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12
    };
    adc_oneshot_config_channel(adc1_handle, MOISTURE_ADC_CHAN, &config);
}



// ============================================================
// IWASM MAIN (igual)
// ============================================================

void *iwasm_main(void *arg) {

    uint8_t *wasm_file_buf = os_malloc(guest_wasm_len);
    memcpy(wasm_file_buf, guest_wasm, guest_wasm_len);

    RuntimeInitArgs init_args;
    memset(&init_args, 0, sizeof(init_args));

    init_args.mem_alloc_type = Alloc_With_Allocator;
    init_args.mem_alloc_option.allocator.malloc_func = (void *)os_malloc;
    init_args.mem_alloc_option.allocator.realloc_func = (void *)os_realloc;
    init_args.mem_alloc_option.allocator.free_func = (void *)os_free;

    wasm_runtime_full_init(&init_args);

    wasm_runtime_register_natives("env",
                                  extended_native_symbols,
                                  sizeof(extended_native_symbols)/sizeof(NativeSymbol));

    char error_buf[128];

    wasm_module_t module =
        wasm_runtime_load(wasm_file_buf, guest_wasm_len,
                          error_buf, sizeof(error_buf));

    wasm_module_inst_t inst =
        wasm_runtime_instantiate(module, 8*1024, 8*1024,
                                 error_buf, sizeof(error_buf));

    wasm_application_execute_main(inst, 0, NULL);

    return NULL;
}



// ============================================================
// APP MAIN  >>> MODIFIED
// ============================================================

void app_main(void)
{
    init_hardware();

    wifi_init();        // >>> ADDED
    vTaskDelay(pdMS_TO_TICKS(3000));  // >>> ADDED (espera wifi)

    mqtt_start();       // >>> ADDED
    vTaskDelay(pdMS_TO_TICKS(2000));  // >>> ADDED (espera mqtt)

    pthread_t t;
    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setstacksize(&tattr, 8192); // >>> aumentei stack

    pthread_create(&t, &tattr, iwasm_main, NULL);
}

