#include "stm32_u8g2.h"
#include "tim.h"
#include "i2c.h"


uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
    static uint8_t buffer[128];
    static uint8_t buf_idx;
    uint8_t *data;
 
    switch (msg)
    {
    case U8X8_MSG_BYTE_INIT:
    {
        /* add your custom code to init i2c subsystem */
        MX_I2C2_Init(); //I2C��ʼ��
    }
    break;
 
    case U8X8_MSG_BYTE_START_TRANSFER:
    {
        buf_idx = 0;
    }
    break;
 
    case U8X8_MSG_BYTE_SEND:
    {
        data = (uint8_t *)arg_ptr;
 
        while (arg_int > 0)
        {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }
    }
    break;
 
    case U8X8_MSG_BYTE_END_TRANSFER:
    {
        if (HAL_I2C_Master_Transmit(&hi2c2, OLED_ADDRESS, buffer, buf_idx, 1000) != HAL_OK)
            return 0;
    }
    break;
 
    case U8X8_MSG_BYTE_SET_DC:
        break;
 
    default:
        return 0;
    }
 
    return 1;
}



uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_DELAY_100NANO: // delay arg_int * 100 nano seconds
        __NOP();
        break;
    case U8X8_MSG_DELAY_10MICRO: // delay arg_int * 10 micro seconds
        for (uint16_t n = 0; n < 320; n++)
        {
            __NOP();
        }
        break;
    case U8X8_MSG_DELAY_MILLI: // delay arg_int * 1 milli second
        HAL_Delay(1);
        break;
    case U8X8_MSG_DELAY_I2C: // arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
        Tims_delay_us(5);
        break;                    // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
    case U8X8_MSG_GPIO_I2C_CLOCK: // arg_int=0: Output low at I2C clock pin
        break;                    // arg_int=1: Input dir with pullup high for I2C clock pin
    case U8X8_MSG_GPIO_I2C_DATA:  // arg_int=0: Output low at I2C data pin
        break;                    // arg_int=1: Input dir with pullup high for I2C data pin
    case U8X8_MSG_GPIO_MENU_SELECT:
        u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_NEXT:
        u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_PREV:
        u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_HOME:
        u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
        break;
    default:
        u8x8_SetGPIOResult(u8x8, 1); // default return value
        break;
    }
    return 1;
}
void u8g2Init(u8g2_t *u8g2)
{
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay); // ��ʼ��u8g2 �ṹ��
	u8g2_InitDisplay(u8g2);                                                                       // 
	u8g2_SetPowerSave(u8g2, 0);                                                                   // 
	u8g2_ClearBuffer(u8g2);
}


void draw(u8g2_t *u8g2)
{
	u8g2_ClearBuffer(u8g2);

    u8g2_SetFontMode(u8g2, 1); /*����ģʽѡ��*/
    u8g2_SetFontDirection(u8g2, 0); /*���巽��ѡ��*/
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf); /*�ֿ�ѡ��*/
    u8g2_DrawStr(u8g2, 0, 20, "U");

    u8g2_SetFontDirection(u8g2, 1);
    u8g2_SetFont(u8g2, u8g2_font_inb30_mn);
    u8g2_DrawStr(u8g2, 21,8,"8");

    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf);
    u8g2_DrawStr(u8g2, 51,30,"g");
    u8g2_DrawStr(u8g2, 67,30,"\xb2");

    u8g2_DrawHLine(u8g2, 2, 35, 47);
    u8g2_DrawHLine(u8g2, 3, 36, 47);
    u8g2_DrawVLine(u8g2, 45, 32, 12);
    u8g2_DrawVLine(u8g2, 46, 33, 12);

    u8g2_SetFont(u8g2, u8g2_font_4x6_tr);
    u8g2_DrawStr(u8g2, 1,54,"github.com/olikraus/u8g2");

	u8g2_SendBuffer(u8g2);
	HAL_Delay(1000);
}

void draw2(u8g2_t *u8g2)
{
	u8g2_ClearBuffer(u8g2);

    u8g2_SetFontMode(u8g2, 1); /*����ģʽѡ��*/
    u8g2_SetFontDirection(u8g2, 0); /*���巽��ѡ��*/

    /* ��ʾѧ�� */
    u8g2_SetFont(u8g2, u8g2_font_ncenB12_tf); /* ��ֿ�ѡ�� */
    u8g2_DrawStr(u8g2, 0, 25, "632307030504");

    /* ��ʾ���� */
    u8g2_SetFont(u8g2, u8g2_font_ncenB14_tr);
    u8g2_DrawStr(u8g2, 0, 45, "Einstein_Niu");

    /* ��ʾװ��װ�� */
    u8g2_SetFont(u8g2, u8g2_font_6x10_tr);
    u8g2_DrawStr(u8g2, 0, 60, "Student Info");
}

//�������
void draw3(u8g2_t *u8g2)
{
	u8g2_ClearBuffer(u8g2);

    u8g2_SetFontMode(u8g2, 1);
    u8g2_SetFontDirection(u8g2, 0);

    /* 标题 */
    u8g2_SetFont(u8g2, u8g2_font_ncenB14_tr);
    u8g2_DrawStr(u8g2, 0, 20, "System Status");

    /* 状态信息 */
    u8g2_SetFont(u8g2, u8g2_font_8x13_tf);
    u8g2_DrawStr(u8g2, 0, 35, "CPU: STM32F103");
    u8g2_DrawStr(u8g2, 0, 48, "OLED: 128x64");
    u8g2_DrawStr(u8g2, 0, 61, "Status: Running");
}

void testDrawPixelToFillScreen(u8g2_t *u8g2)
{
  int t = 1000;
	u8g2_ClearBuffer(u8g2);

  for (int j = 0; j < 64; j++)
  {
    for (int i = 0; i < 128; i++)
    {
      u8g2_DrawPixel(u8g2,i, j);
    }
  }
  HAL_Delay(1000);
}

// 自动循环滑动功能实现
// 海绵宝宝页面特殊处理函数
void draw4_with_indicator(u8g2_t *u8g2)
{
    // 绘制海绵宝宝动画
    updateSpongeBobAnimation(u8g2);

    // 在海绵宝宝动画上叠加页面指示器
    drawPageIndicator(u8g2);

    // 再次发送缓冲区（因为海绵宝宝动画已经发送了一次）
    u8g2_SendBuffer(u8g2);
}

static DisplayPage pages[] = {
    {"Student Info", draw2},
    {"U8g2 Demo", draw},
    {"System Status", draw3},
    {"SpongeBob", draw4_with_indicator},  // 海绵宝宝动画页面
};

#define PAGE_COUNT (sizeof(pages) / sizeof(DisplayPage))
#define AUTO_SLIDE_INTERVAL 3000  // 3秒切换间隔

static uint8_t current_page = 0;
static uint32_t last_switch_time = 0;
static uint8_t auto_enabled = 1;  // 自动滑动开关

void initAutoSlide(void)
{
    current_page = 0;
    last_switch_time = HAL_GetTick();
    auto_enabled = 1;
}

void drawPageIndicator(u8g2_t *u8g2)
{
    char indicator[8];
    sprintf(indicator, "%d/%d", current_page + 1, PAGE_COUNT);

    u8g2_SetFont(u8g2, u8g2_font_5x7_tr);
    u8g2_DrawStr(u8g2, 128 - u8g2_GetStrWidth(u8g2, indicator) - 2, 8, indicator);

    // 绘制页面指示点
    for(uint8_t i = 0; i < PAGE_COUNT; i++)
    {
        uint8_t x = 128/2 - (PAGE_COUNT * 4) + (i * 8);
        if(i == current_page)
        {
            u8g2_DrawFilledEllipse(u8g2, x, 62, 3, 2, U8G2_DRAW_ALL);  // 当前页面实心点
        }
        else
        {
            u8g2_DrawEllipse(u8g2, x, 62, 3, 2, U8G2_DRAW_ALL);        // 其他页面空心点
        }
    }
}

void updateAutoSlide(u8g2_t *u8g2)
{
    uint32_t current_time = HAL_GetTick();

    // 检查是否需要切换页面
    if(auto_enabled && (current_time - last_switch_time >= AUTO_SLIDE_INTERVAL))
    {
        last_switch_time = current_time;
        current_page = (current_page + 1) % PAGE_COUNT;  // 循环到下一页
    }

    // 绘制当前页面
    if(pages[current_page].draw_func != NULL)
    {
        // 如果是海绵宝宝页面，特殊处理（实时动画）
        if(current_page == 3)  // SpongeBob page
        {
            pages[current_page].draw_func(u8g2);
            HAL_Delay(100);  // 稍微长一点的延迟让动画更流畅
        }
        else
        {
            pages[current_page].draw_func(u8g2);

            // 在当前页面上叠加页面指示器
            drawPageIndicator(u8g2);

            // 发送缓冲区并短暂延迟
            u8g2_SendBuffer(u8g2);
            HAL_Delay(50);  // 短暂延迟确保显示稳定
        }
    }
}

// 海绵宝宝动画实现
static uint8_t eye_state = 0;  // 眼睛状态：0=睁开，1=半闭，2=闭合
static uint32_t last_blink_time = 0;
static uint8_t blink_count = 0;
static uint8_t in_blink = 0;

#define BLINK_INTERVAL 3000    // 眨眼间隔3秒
#define BLINK_DURATION 150     // 眨眼持续时间150ms

void drawSpongeBobFace(u8g2_t *u8g2, int8_t eye_offset)
{
    // 绘制海绵宝宝的脸（黄色方形）
    u8g2_SetDrawColor(u8g2, 1);
    u8g2_DrawRBox(u8g2, 20, 10, 88, 44, 8);  // 脸部

    // 绘制脸上的毛孔（小圆点）
    for(int y = 15; y < 45; y += 6)
    {
        for(int x = 25; x < 100; x += 6)
        {
            u8g2_DrawPixel(u8g2, x, y);
            u8g2_DrawPixel(u8g2, x+2, y+2);
        }
    }

    // 绘制眼睛
    u8g2_SetDrawColor(u8g2, 0);  // 黑色眼睛

    // 左眼
    u8g2_DrawFilledEllipse(u8g2, 45, 25, 8, 8+eye_offset, U8G2_DRAW_ALL);

    // 右眼
    u8g2_DrawFilledEllipse(u8g2, 83, 25, 8, 8+eye_offset, U8G2_DRAW_ALL);

    // 眼睛高光（睁开时显示）
    if(eye_offset <= 4)
    {
        u8g2_SetDrawColor(u8g2, 1);
        u8g2_DrawDisc(u8g2, 47, 23, 2, U8G2_DRAW_ALL);
        u8g2_DrawDisc(u8g2, 85, 23, 2, U8G2_DRAW_ALL);
    }

    // 绘制嘴巴
    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawCircle(u8g2, 64, 42, 8, U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_LOWER_RIGHT);
    u8g2_DrawHLine(u8g2, 48, 42, 32);  // 嘴巴底线

    // 绘制鼻子
    u8g2_DrawPixel(u8g2, 64, 32);

    // 绘制牙齿
    u8g2_SetDrawColor(u8g2, 1);
    u8g2_DrawBox(u8g2, 58, 43, 3, 4);
    u8g2_DrawBox(u8g2, 63, 43, 3, 4);
    u8g2_DrawBox(u8g2, 67, 43, 3, 4);

    // 绘制衬衫领子
    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawTriangle(u8g2, 40, 54, 50, 54, 45, 58);
    u8g2_DrawTriangle(u8g2, 88, 54, 78, 54, 83, 58);

    u8g2_SetDrawColor(u8g2, 1);  // 恢复白色
}

void drawSpongeBob(u8g2_t *u8g2)
{
    u8g2_ClearBuffer(u8g2);

    // 绘制标题
    u8g2_SetFont(u8g2, u8g2_font_ncenB10_tr);
    u8g2_DrawStr(u8g2, 35, 8, "SpongeBob");

    // 根据眼睛状态绘制海绵宝宝
    int8_t eye_offset = 0;
    switch(eye_state)
    {
        case 1: eye_offset = 4; break;   // 半闭
        case 2: eye_offset = 7; break;   // 闭合
        default: eye_offset = 0; break;  // 睁开
    }

    drawSpongeBobFace(u8g2, eye_offset);
}

void initSpongeBobAnimation(void)
{
    eye_state = 0;
    last_blink_time = HAL_GetTick();
    blink_count = 0;
    in_blink = 0;
}

void updateSpongeBobAnimation(u8g2_t *u8g2)
{
    uint32_t current_time = HAL_GetTick();

    // 检查是否需要开始眨眼
    if(!in_blink && (current_time - last_blink_time >= BLINK_INTERVAL))
    {
        in_blink = 1;
        blink_count = 0;
        last_blink_time = current_time;
    }

    // 处理眨眼动画
    if(in_blink)
    {
        static uint32_t last_frame_time = 0;
        if(current_time - last_frame_time >= BLINK_DURATION / 3)
        {
            last_frame_time = current_time;
            blink_count++;

            switch(blink_count)
            {
                case 1: eye_state = 1; break;  // 半闭
                case 2: eye_state = 2; break;  // 完全闭合
                case 3: eye_state = 1; break;  // 半开
                case 4: eye_state = 0; in_blink = 0; last_blink_time = current_time; break;  // 完全睁开
            }
        }
    }

    // 绘制海绵宝宝
    drawSpongeBob(u8g2);

    // 发送缓冲区
    u8g2_SendBuffer(u8g2);
}

// 包装函数，用于自动滑动系统
void draw4(u8g2_t *u8g2)
{
    // 直接调用海绵宝宝动画更新函数
    updateSpongeBobAnimation(u8g2);
}