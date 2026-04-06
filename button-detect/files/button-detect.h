#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/of.h>


#define BUF_SIZE 128

static spinlock_t lock_button;
static dev_t dev_num;
static struct cdev button_cdev;
static DECLARE_WAIT_QUEUE_HEAD(wq);
static volatile bool data_ready = 0; 
static struct class *button_class;

typedef struct sLED{
    int state;
}LED_state;

typedef struct uart_setting{
    struct tty_struct *p_tty;
    int baud_rate;
    char sz_uart_msg[64];
}sUart;

struct sButton_detect_data {
    struct gpio_desc *p_button_user1_gpiod;
    struct gpio_desc *p_button_user2_gpiod;
    struct gpio_desc *p_led_blue_gpiod;
    struct gpio_desc *p_led_green_gpiod;
    int irq_user1;
    int irq_user2;
    const char *p_label;
    struct work_struct s_worker;
    LED_state s_LED_blue;
    LED_state s_LED_green;
    // sUart *p_uart;
};

