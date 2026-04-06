#include "button-detect.h"

char msg_buf[BUF_SIZE];
static ssize_t button_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    ssize_t ret;
    // pr_info("wait here for event\n");
    wait_event_interruptible(wq, data_ready);
    // pr_info("interrrupt awake\n");
    spin_lock(&lock_button);
    data_ready = 0;
    spin_unlock(&lock_button);
    // pr_info("cp to user\n");
    if (copy_to_user(buf, msg_buf, strlen(msg_buf) + 1))
        return -EFAULT;

    ret = strlen(msg_buf) + 1;
    return ret;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = button_read,
};

static irqreturn_t button_isr(int irq, void *dev_id)
{
    struct sButton_detect_data *p_data = dev_id;
    // static int button1 = 0, button2 = 0;
    spin_lock(&lock_button);
    if(irq == p_data->irq_user1){
        // pr_info("User1 Pressed!! change Blue LED status from %d to %d\n",p_data->s_LED_blue.state,!p_data->s_LED_blue.state);
        snprintf(msg_buf, BUF_SIZE, "ifconfig br-lan 88.0.100.150 netmask 255.255.255.0\r\n");
        p_data->s_LED_blue.state = !p_data->s_LED_blue.state;
        gpiod_set_value(p_data->p_led_blue_gpiod, p_data->s_LED_blue.state);
    }else{
        // pr_info("User2 Pressed!! change Green LED status from %d to %d\n",p_data->s_LED_green.state,!p_data->s_LED_green.state);
        snprintf(msg_buf, BUF_SIZE, "iperf3 -s > ethspeed.log\r\n", !p_data->s_LED_green.state);
        p_data->s_LED_green.state = !p_data->s_LED_green.state;
        gpiod_set_value(p_data->p_led_green_gpiod, p_data->s_LED_green.state);
    }
    data_ready = 1;
    spin_unlock(&lock_button);
    // pr_info("wake up interrupt\n");
    wake_up_interruptible(&wq);
    // pr_info("interrupt handled successful\n");
    return IRQ_HANDLED;
}

static int button_detect_probe(struct platform_device *pdev)
{
    struct sButton_detect_data *p_data;
    int ret;
    dev_info(&pdev->dev, "Button driver init (DT + IRQ)\n");

    p_data = devm_kzalloc(&pdev->dev, sizeof(*p_data), GFP_KERNEL);
    if (!p_data){
        return -ENOMEM;
    }

    p_data->p_button_user1_gpiod = devm_gpiod_get(&pdev->dev, "button_user1", GPIOD_IN);
    if (IS_ERR(p_data->p_button_user1_gpiod)) {
        dev_err(&pdev->dev, "Failed to get button_user1 GPIO\n");
        return PTR_ERR(p_data->p_button_user1_gpiod);
    }

    p_data->p_button_user2_gpiod = devm_gpiod_get(&pdev->dev, "button_user2", GPIOD_IN);
    if (IS_ERR(p_data->p_button_user2_gpiod)) {
        dev_err(&pdev->dev, "Failed to get button_user2 GPIO\n");
        return PTR_ERR(p_data->p_button_user2_gpiod);
    }

    p_data->p_led_blue_gpiod = devm_gpiod_get(&pdev->dev, "led_blue", GPIOD_OUT_LOW);
    if (IS_ERR(p_data->p_led_blue_gpiod)) {
        dev_err(&pdev->dev, "Failed to get LED_blue GPIO\n");
        return PTR_ERR(p_data->p_led_blue_gpiod);
    }

    p_data->p_led_green_gpiod = devm_gpiod_get(&pdev->dev, "led_green", GPIOD_OUT_LOW);
    if (IS_ERR(p_data->p_led_green_gpiod)) {
        dev_err(&pdev->dev, "Failed to get LED_green GPIO\n");
        return PTR_ERR(p_data->p_led_green_gpiod);
    }
    p_data->s_LED_blue.state = 0;
    p_data->s_LED_green.state = 0;
    of_property_read_string(pdev->dev.of_node, "label", &p_data->p_label);

    p_data->irq_user1 = platform_get_irq(pdev, 0);
    p_data->irq_user2 = platform_get_irq(pdev, 1);


    spin_lock_init(&lock_button);
    alloc_chrdev_region(&dev_num, 0, 1, "button_detect");
    cdev_init(&button_cdev, &fops);
    cdev_add(&button_cdev, dev_num, 1);
    button_class = class_create("button_class");
    if (IS_ERR(button_class)) {
        pr_err("Failed to create class\n");
        return PTR_ERR(button_class);
    }
    device_create(button_class, NULL, dev_num, NULL, "button_detect");

    ret = devm_request_threaded_irq(&pdev->dev, p_data->irq_user1,
                                    NULL, button_isr,
                                    IRQF_TRIGGER_RISING | IRQF_ONESHOT ,
                                    dev_name(&pdev->dev), p_data);
    if (ret) {
        dev_err(&pdev->dev, "Failed to request IRQ\n");
        return ret;
    }

    ret = devm_request_threaded_irq(&pdev->dev, p_data->irq_user2,
                                NULL, button_isr,
                                IRQF_TRIGGER_RISING | IRQF_ONESHOT,
                                dev_name(&pdev->dev), p_data);
    if (ret) {
        dev_err(&pdev->dev, "Failed to request IRQ for button 2\n");
        return ret;
    }

    gpiod_set_value(p_data->p_led_blue_gpiod, p_data->s_LED_blue.state);
    gpiod_set_value(p_data->p_led_green_gpiod, p_data->s_LED_green.state);
    platform_set_drvdata(pdev, p_data);
    return 0;
}

static int button_detect_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Button driver removed\n");
    class_destroy(button_class);
    device_destroy(button_class, dev_num);
    cdev_del(&button_cdev);
    unregister_chrdev_region(dev_num, 1);
    return 0;
}

static const struct of_device_id button_detect_of_match[] = {
    { .compatible = "Quanta,button-detect" },
    {},
};
MODULE_DEVICE_TABLE(of, button_detect_of_match);

static struct platform_driver button_detect_driver = {
    .probe = button_detect_probe,
    .remove = button_detect_remove,
    .driver = {
        .name = "button-detect",
        .of_match_table = button_detect_of_match,
    },
};

module_platform_driver(button_detect_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Button detect driver using IRQ from DT");