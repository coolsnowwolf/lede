#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/interrupt.h>
#include <linux/ktime.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nuno Goncalves");
MODULE_DESCRIPTION("GL-MiFi power monitoring MCU interface");
MODULE_VERSION("0.1");

static int gpio_tx = 19;
static int gpio_rx = 8;
static int baudrate = 1200;
static int query_interval_sec = 4;

static struct hrtimer timer_tx;
static struct hrtimer timer_rx;
static ktime_t period;
static int rx_bit_index = -1;

static unsigned read_buf_ready = 0;
static unsigned read_buf_size = 0;
static char read_buf[2][64] = {{0},{0}};

static int proc_show(struct seq_file *m, void *v)
{
  seq_printf(m, "%s\n", read_buf[read_buf_ready]);
  return 0;
}

static int proc_open(struct inode *inode, struct  file *file)
{
  return single_open(file, proc_show, NULL);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops hello_proc_ops = {
  .proc_open = proc_open,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = single_release,
};
#else
static const struct file_operations hello_proc_ops = {
  .owner = THIS_MODULE,
  .open = proc_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};
#endif

static irq_handler_t handle_rx_start(unsigned int irq, void* device, struct pt_regs* registers)
{
  if (rx_bit_index == -1)
  {
    hrtimer_start(&timer_rx, ktime_set(0, period / 2), HRTIMER_MODE_REL);
  }
  return (irq_handler_t) IRQ_HANDLED;
}

static enum hrtimer_restart handle_tx(struct hrtimer* timer)
{
  ktime_t current_time = ktime_get();
  const unsigned char character = 'g';
  static int bit_index = -1;

  // Start bit.
  if (bit_index == -1)
  {
      gpio_set_value(gpio_tx, 0);
      bit_index++;
  }

  // Data bits.
  else if (0 <= bit_index && bit_index < 8)
  {
    gpio_set_value(gpio_tx, 1 & (character >> bit_index));
    bit_index++;
  }

  // Stop bit.
  else if (bit_index == 8)
  {
    gpio_set_value(gpio_tx, 1);
    bit_index = -1;
  }

  hrtimer_forward(&timer_tx, current_time, bit_index == 8
    ? ktime_set(query_interval_sec, 0) //wait for next query cycle
    : period); //wait for next bit period

  return HRTIMER_RESTART;
}

void receive_character(unsigned char character)
{
  if(character == '{')
    read_buf_size = 0;

  if(read_buf_size < (sizeof(read_buf[0])-1) || character == '}')
  {
    read_buf[!read_buf_ready][read_buf_size++] = character;
    if(character == '}')
    {
      read_buf[!read_buf_ready][read_buf_size] = '\0';
      read_buf_ready = !read_buf_ready;
      read_buf_size = 0;
    }
  }
}

static enum hrtimer_restart handle_rx(struct hrtimer* timer)
{
  ktime_t current_time = ktime_get();
  static unsigned int character = 0;
  int bit_value = gpio_get_value(gpio_rx);
  enum hrtimer_restart result = HRTIMER_NORESTART;
  bool must_restart_timer = false;

  // Start bit.
  if (rx_bit_index == -1)
  {
    rx_bit_index++;
    character = 0;
    must_restart_timer = true;
  }

  // Data bits.
  else if (0 <= rx_bit_index && rx_bit_index < 8)
  {
    if (bit_value == 0)
    {
      character &= 0xfeff;
    }
    else
    {
      character |= 0x0100;
    }

    rx_bit_index++;
    character >>= 1;
    must_restart_timer = true;
  }

  // Stop bit.
  else if (rx_bit_index == 8)
  {
    receive_character(character);
    rx_bit_index = -1;
  }

  // Restarts the RX timer.
  if (must_restart_timer)
  {
    hrtimer_forward(&timer_rx, current_time, period);
    result = HRTIMER_RESTART;
  }

  return result;
}

static int __init init(void)
{
  bool success = true;

  proc_create("gl_mifi_mcu", 0, NULL, &hello_proc_ops);

  success &= gpio_request(gpio_tx, "soft_uart_tx") == 0;
  success &= gpio_direction_output(gpio_tx, 1) == 0;
  success &= gpio_request(gpio_rx, "soft_uart_rx") == 0;
  success &= gpio_direction_input(gpio_rx) == 0;
  success &= gpio_set_debounce(gpio_rx, 1000/baudrate/2);

  success &= request_irq(
    gpio_to_irq(gpio_rx),
    (irq_handler_t) handle_rx_start,
    IRQF_TRIGGER_FALLING,
    "gl_mifi_mcu_irq_handler",
    NULL) == 0;

  hrtimer_init(&timer_tx, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
  timer_tx.function = &handle_tx;
  hrtimer_init(&timer_rx, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
  timer_rx.function = &handle_rx;
  period = ktime_set(0, 1000000000/baudrate);
  hrtimer_start(&timer_tx, period, HRTIMER_MODE_REL);

  return success;
}

static void __exit exit(void)
{
  disable_irq(gpio_to_irq(gpio_rx));
  hrtimer_cancel(&timer_tx);
  hrtimer_cancel(&timer_rx);
  free_irq(gpio_to_irq(gpio_rx), NULL);
  gpio_set_value(gpio_tx, 0);
  gpio_free(gpio_tx);
  gpio_free(gpio_rx);
  remove_proc_entry("gl_mifi_mcu", NULL);
}

module_init(init);
module_exit(exit);

