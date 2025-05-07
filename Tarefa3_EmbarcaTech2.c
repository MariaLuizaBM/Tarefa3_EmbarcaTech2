#include "hardware/gpio.h"  // Biblioteca para GPIO
#include "hardware/i2c.h"   // Biblioteca para I2C
#include "hardware/pwm.h"   // Biblioteca para PWM
#include "lib/ssd1306.h"    // Biblioteca para o display OLED
#include "lib/font.h"       // Biblioteca para fonte do display OLED
#include "FreeRTOS.h"       // Biblioteca FreeRTOS
#include "FreeRTOSConfig.h" // Configuração do FreeRTOS
#include "task.h"           // Biblioteca para tarefas do FreeRTOS
#include <stdlib.h>         // Biblioteca para malloc/free
#include "hardware/pio.h"   // Biblioteca para PIO
#include "blinkConta.pio.h" // Biblioteca gerada para o PIO

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define led_g 11
#define led_r 13
#define botaoA 5
#define BUZZER_PIN 21
volatile bool modo_led = false;
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

// Buffer para armazenar quais LEDs estão ligados matriz 5x5
bool led_buffer[10][NUM_PIXELS] = {
    
    //número 0
    {0, 0, 0, 0, 0, 
     0, 0, 0, 0, 0,                 
     0, 0, 0, 0, 0, 
     0, 0, 0, 0, 0, 
     0, 0, 0, 0, 0},

    //número 1
    {0, 0, 1, 0, 0, 
     0, 0, 1, 0, 0,                 
     0, 0, 1, 0, 0, 
     0, 1, 1, 1, 0, 
     0, 0, 1, 0, 0},

    //número 2
    {0, 0, 1, 0, 0, 
     0, 0, 0, 0, 0, 
     0, 0, 1, 0, 0, 
     0, 0, 1, 0, 0, 
     0, 0, 1, 0, 0},

    //número 3
    {1, 0, 0, 0, 1, 
     0, 1, 0, 1, 0, 
     0, 0, 1, 0, 0, 
     0, 1, 0, 1, 0, 
     1, 0, 0, 0, 1}
};

// Função para enviar os dados para o PIO
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Função para converter RGB para GRB
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Função para exibir o número na matriz de LEDs
void exibir_numero(const bool *buffer, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = urgb_u32(r, g, b);

    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(buffer[i] ? color : 0);
    }
}

// Task para a matriz de LEDs
void vMatrizTask() {
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &blinkConta_program);
    blinkConta_program_init(pio, 0, offset, WS2812_PIN, 800000, IS_RGBW);

    while (true) {
        bool verde = gpio_get(led_g);
        bool vermelho = gpio_get(led_r);
        int numero = 0;

        if (verde && !vermelho)
            numero = 1;
        else if (verde && vermelho)
            numero = 2;
        else if (!verde && vermelho)
            numero = 3;

        switch (numero) {
            case 0:
                numero = 0;
                exibir_numero(led_buffer[numero], 0, 0, 0); // Desliga todos os LEDs
                break;
            case 1:
                numero = 1;
                exibir_numero(led_buffer[numero], 0, 5, 0); // Verde
                break;
            case 2:
                numero = 2;
                exibir_numero(led_buffer[numero], 5, 5, 0); // Amarelo
                break;
            case 3:
                numero = 3;
                exibir_numero(led_buffer[numero], 5, 0, 0); // Vermelho
                break;
            default:
                numero = 0;
                exibir_numero(led_buffer[numero], 0, 0, 0); // Desliga todos os LEDs
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

// Task para o botão A
void vBotaoTask()
{
    gpio_init(botaoA);
    gpio_set_dir(botaoA, GPIO_IN);
    gpio_pull_up(botaoA);

    bool estadoAnterior = true;

    while (true)
    {
        bool estadoAtual = gpio_get(botaoA);

        if (estadoAnterior && !estadoAtual)
        {
            // Debounce inicial
            vTaskDelay(pdMS_TO_TICKS(20));
            if (!gpio_get(botaoA))
            {
                while (!gpio_get(botaoA))
                {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }

                // Alterna a flag do modo
                modo_led = !modo_led;
            }
        }

        estadoAnterior = estadoAtual;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Task para o LED
void vBlinkLedTask()
{
    // Inicializa os LEDs
    gpio_init(led_g);
    gpio_set_dir(led_g, GPIO_OUT);
    gpio_init(led_r);
    gpio_set_dir(led_r, GPIO_OUT);

    while (true)
    {
        if (!modo_led) // Modo Normal
        {
            gpio_put(led_g, true);
            vTaskDelay(pdMS_TO_TICKS(5000));
            gpio_put(led_g, false);
            
            gpio_put(led_r, true);
            gpio_put(led_g, true);
            vTaskDelay(pdMS_TO_TICKS(3000));
            gpio_put(led_r, false);
            gpio_put(led_g, false);
            
            gpio_put(led_r, true);
            vTaskDelay(pdMS_TO_TICKS(5000));
            gpio_put(led_r, false);
        }
        else // Modo Noturno
        {
            gpio_put(led_r, true);
            gpio_put(led_g, true);
            vTaskDelay(pdMS_TO_TICKS(3000));
            gpio_put(led_r, false);
            gpio_put(led_g, false);
        }
    }
}

// Função para tocar o buzzer
void tocar_buzzer(uint slice, uint channel, uint freq_hz, uint duracao_ms) {
    uint32_t clock_hz = 125000000;
    uint32_t top = clock_hz / (freq_hz * 4); // Ajuste de clock para buzzer audível
    pwm_set_wrap(slice, top);
    pwm_set_chan_level(slice, channel, top / 2);
    pwm_set_enabled(slice, true);
    vTaskDelay(pdMS_TO_TICKS(duracao_ms));
    pwm_set_enabled(slice, false);
}

// Task para o buzzer
void vBuzzerTask() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint channel = pwm_gpio_to_channel(BUZZER_PIN);
    pwm_set_clkdiv(slice, 4.f);

    while (true) {
        bool verde = gpio_get(led_g);
        bool vermelho = gpio_get(led_r);

        if (!modo_led) {  // Modo Normal
            if (verde && !vermelho) {
                // Beep curto por segundo
                tocar_buzzer(slice, channel, 1000, 200);
                vTaskDelay(pdMS_TO_TICKS(800));
            } else if (verde && vermelho) {
                // Beep rápido intermitente
                tocar_buzzer(slice, channel, 800, 100); 
                vTaskDelay(pdMS_TO_TICKS(100));
            } else if (!verde && vermelho) {
                // Beep contínuo curto
                tocar_buzzer(slice, channel, 500, 500); 
                vTaskDelay(pdMS_TO_TICKS(1500));
            } else {
                vTaskDelay(pdMS_TO_TICKS(200)); // Nenhum LED ativo
            }
        } else {
            // Modo Noturno: beep lento a cada 2 segundos
            tocar_buzzer(slice, channel, 700, 100);
            vTaskDelay(pdMS_TO_TICKS(1900));
        }
    }
}

// Task para o display
void vDisplayTask()
{
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    char str_y[5]; // Buffer para armazenar a string
    int contador = 0;
    bool cor = true;
    while (true)
    {
        bool verde = gpio_get(led_g);
        bool vermelho = gpio_get(led_r);
        const char* mensagem;
        const char* modo;

        // Verifica o estado dos LEDs e define a mensagem
        if (verde && !vermelho)
            mensagem = "LIVRE!";
        else if (verde && vermelho)
            mensagem = "ATENCAO!";
        else if (!verde && vermelho)
            mensagem = "PARE!";
        
        if (!modo_led) {  // Modo Normal
            if (verde && !vermelho) {
                modo = "Modo Normal";
            } else if (verde && vermelho) {
                modo = "Modo Normal";
            } else if (!verde && vermelho) {
                modo = "Modo Normal";
            } else {
                vTaskDelay(pdMS_TO_TICKS(200)); // Nenhum LED ativo
            }
            } else {
                // Modo Noturno
                modo = "Modo Noturno";
            }

        // Desenha a tela
        ssd1306_fill(&ssd, false);  // Limpa display
        ssd1306_rect(&ssd, 3, 3, 122, 60, true, false);   // Moldura
        ssd1306_draw_string(&ssd, modo, 20, 10); 
        ssd1306_line(&ssd, 5, 25, 122, 25, true);
        ssd1306_draw_string(&ssd, mensagem, 35, 40);
        ssd1306_send_data(&ssd);

        vTaskDelay(pdMS_TO_TICKS(250));  
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int main()
{
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    stdio_init_all();

    xTaskCreate(vMatrizTask, "Task Matriz", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBotaoTask, "Task BotaoA", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBlinkLedTask, "Task Led", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBuzzerTask, "Task Buzzer", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplayTask, "Task Display", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    panic_unsupported();
}