#pragma once

/* !!! AI-Generated Code !!! */

#include "Base.h"
#include <stdarg.h>

// Базовый адрес COM1 (стандарт для PC/AT)
#define COM1_BASE 0x3F8

// Смещения регистров UART 16550
#define UART_RBR 0x00 // Receiver Buffer Register (read)
#define UART_THR 0x00 // Transmitter Holding Register (write)
#define UART_IER 0x01 // Interrupt Enable Register
#define UART_FCR 0x02 // FIFO Control Register
#define UART_LCR 0x03 // Line Control Register
#define UART_MCR 0x04 // Modem Control Register
#define UART_LSR 0x05 // Line Status Register
#define UART_DLL 0x00 // Divisor Latch Low (when LCR[7] == 1)
#define UART_DLM 0x01 // Divisor Latch High (when LCR[7] == 1)

// Простая задержка для стабилизации (зависит от частоты CPU)
static inline void delay(u32 count) {
    while (count--) {
        __asm__ volatile ("nop");
    }
}

// Отправка одного байта
static inline void uart_send_byte(u8 byte) {
    // Ждем, пока передатчик освободится (Bit 5 of LSR = THRE)
    while (1) {
        u8 status;
        __asm__ volatile (
            "movl %1, %%edx\n"   // EDX = адрес порта LSR (используем 32-битную запись для безопасности в x64)
            "inb %%dx, %%al\n"   // Читаем статус (CPU использует DX[15:0])
            "movb %%al, %0\n"    // Сохраняем в status
            : "=r"(status)      // выходной операнд
            : "i"(COM1_BASE + UART_LSR) // входной операнд (адрес)
            : "ax", "dx"        // используемые регистры
        );
        
        // Проверяем бит 5 (0x20). Если установлен, буфер пуст и готов к приему данных.
        if (status & 0x20) {
            break;
        }
    }

    // Отправляем байт в THR
    __asm__ volatile (
        "movl %1, %%edx\n"   // EDX = адрес порта THR
        "movb %0, %%al\n"    // AL = данные для отправки
        "outb %%al, %%dx\n"  // Пишем в порт
        :
        : "r"(byte), "i"(COM1_BASE + UART_THR)
        : "ax", "dx"
    );
}

// Инициализация COM1 на 9600 бод, 8N1
static inline void uart_init() {
    // 1. Отключаем прерывания (IER = 0)
    __asm__ volatile (
        "xor %%al, %%al\n"
        "movl %0, %%edx\n"
        "outb %%al, %%dx\n"
        :
        : "i"(COM1_BASE + UART_IER)
        : "ax", "dx"
    );

    // 2. Включаем доступ к делителю скорости (LCR |= 0x80)
    __asm__ volatile (
        "movb $0x80, %%al\n"
        "movl %0, %%edx\n"
        "outb %%al, %%dx\n"
        :
        : "i"(COM1_BASE + UART_LCR)
        : "ax", "dx"
    );

    // 3. Устанавливаем делитель для 9600 бод
    // 1843200 / (16 * 9600) = 12 (0x0C)
    
    // DLL = 0x0C (младший байт 12)
    __asm__ volatile (
        "movb $0x0C, %%al\n"
        "movl %0, %%edx\n"
        "outb %%al, %%dx\n"
        :
        : "i"(COM1_BASE + UART_DLL)
        : "ax", "dx"
    );

    // DLM = 0x00 (старший байт 0)
    __asm__ volatile (
        "xor %%al, %%al\n"
        "movl %0, %%edx\n"
        "outb %%al, %%dx\n"
        :
        : "i"(COM1_BASE + UART_DLM)
        : "ax", "dx"
    );

    // 4. Настраиваем формат: 8 бит, 1 стоп-бит, без четности (LCR = 0x03)
    __asm__ volatile (
        "movb $0x03, %%al\n"
        "movl %0, %%edx\n"
        "outb %%al, %%dx\n"
        :
        : "i"(COM1_BASE + UART_LCR)
        : "ax", "dx"
    );
}

// Отправка строки (с обработкой \n -> \r\n)
static void print_string(const char *str) {
    while (*str) {
        if (*str == '\n') {
            uart_send_byte('\r');
            uart_send_byte('\n');
        } else {
            uart_send_byte((u8)*str);
        }
        str++;
    }
}

// Реверс строки (нужен для преобразования чисел)
static void reverse_string(char *str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char tmp = str[start];
        str[start] = str[end];
        str[end] = tmp;
        start++;
        end--;
    }
}

// Преобразование u64 в строку (dec или hex)
static void print_u64_value(u64 value, int base) {
    char buffer[64]; // Увеличили буфер для безопасности
    u64 i = 0;

    if (value == 0) {
        buffer[i++] = '0';
    } else {
        while (value != 0 && i < 64) { // Защита от переполнения буфера
            u64 rem = value % base;
            buffer[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
            value /= base;
        }
    }

    buffer[i] = '\0';
    reverse_string(buffer, i);
    print_string(buffer);
}



static inline void print_str(const char *str) {
    if (!str) return;
    while (*str) {
        if (*str == '\n') {
            uart_send_byte('\r');
            uart_send_byte('\n');
        } else {
            uart_send_byte((u8)*str);
        }
        str++;
    }
}

// Безопасная печать 16-ричного числа (u64)
static inline void print_hex(u64 val) {
    print_u64_value(val, 16);
}

// Безопасная печать десятичного числа (u64)
static inline void print_dec(u64 val) {
    char buffer[20];
    int i = 0;
    
    if (val == 0) {
        uart_send_byte('0');
        return;
    }
    
    while (val > 0) {
        buffer[i++] = '0' + (val % 10);
        val /= 10;
    }
    
    // Выводим в обратном порядке
    while (i > 0) {
        uart_send_byte((u8)buffer[--i]);
    }
}

// --- Основной printf ---

static inline void kprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 's': {
                    // Получаем указатель на строку
                    const char *str = va_arg(args, const char *);
                    // Передаем в безопасную функцию печати
                    print_string(str);
                    break;
                }
                case 'm': {
                    const Memory mem = va_arg(args, Memory);
                    if( !MemoryValid(mem) ) {
                        print_string("[MemoryInvalid]");
                    }else{
                        print_string("[Memory: 0x");
                        print_hex((u64)mem.Base);
                        print_string(" - 0x");
                        print_hex((u64)mem.End);

                        u64 size = MemorySize(mem);
                        u64 gigs = size / (1 << 30);
                        u64 migs = (size % (1 << 30)) / (1 << 20);
                        u64 kibs = (size % (1 << 20)) / (1 << 10);
                        u64 byts = (size % (1 << 10));
                        
                        print_string(" (");
                        if( gigs > 0 ) {
                            print_dec(gigs);
                            print_string("G ");
                        }
                        if( migs > 0 ) {
                            print_dec(migs);
                            print_string("M ");
                        }

                        if( kibs > 0 ) {
                            print_dec(kibs);
                            print_string("K ");
                        }

                        if( byts > 0 || (kibs == 0 && migs == 0 && gigs == 0) ) {
                            print_dec(byts);
                        }

                        print_string(" bytes)]");
                    }
                    break;
                    
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    uart_send_byte((u8)c);
                    break;
                }
                case 'd':
                case 'p':
                case 'i': {
                    u64 val = va_arg(args, u64);
                    if( val == (u64)NULL )
                        print_string("NULL");
                    else
                        print_u64_value((u64)val, 10);
                    break;
                }
                case 'u': {
                    u64 val = va_arg(args, u64);
                    print_u64_value((u64)val, 10);
                    break;
                }
                case 'x': {
                    u64 val = va_arg(args, u64);
                    print_u64_value((u64)val, 16);
                    break;
                }
                case 'U': { // Кастомный спецификатор для u64 (unsigned decimal)
                    u64 val = va_arg(args, u64);
                    print_u64_value(val, 10);
                    break;
                }
                case 'X': { // Кастомный спецификатор для u64 (hex)
                    u64 val = va_arg(args, u64);
                    print_u64_value(val, 16);
                    break;
                }
                case 'b': {
                    u64 val = va_arg(args, u64);
                    print_u64_value(val, 2);
                    break;
                }
                case '%': {
                    uart_send_byte('%');
                    break;
                }
                default: {
                    uart_send_byte('?');
                    uart_send_byte((u8)*format); // Показать неизвестный спецификатор
                    break;
                }
            }
        } else {
            if (*format == '\n') {
                uart_send_byte('\r');
                uart_send_byte('\n');
            } else {
                uart_send_byte((u8)*format);
            }
        }
        format++;
    }

    va_end(args);
}