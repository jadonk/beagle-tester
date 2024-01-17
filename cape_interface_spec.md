# Cape Interface Spec

## I2C

| Links           | DT symbol    | SCL   | SDA    |
| --------------- | ------------ | ----- | ------ |
| /dev/bone/i2c/0 | bone_i2c_0   | N/A   | N/A    |
| /dev/bone/i2c/1 | bone_i2c_1   | P9.17 | P9.18  |
| /dev/bone/i2c/2 | bone_i2c_2   | P9.19 | P9.20  |
| /dev/bone/i2c/3 | bone_i2c_3   | P9.24 | P9.26  |
| /dev/bone/i2c/4 | bone_i2c_4   | P9.21 | P9.22  |

## SPI

| Bone bus           | DT symbol  | COPI  | CIPO  | CLK   | CS    |
| ------------------ | ---------- | ----- | ----- | ----- | ----- |
| /dev/bone/spi/0.0  | bone_spi_0 | P9.18 | P9.21 | P9.22 | P9.17 |
| /dev/bone/spi/0.1  |            | P9.18 | P9.21 | P9.22 | P9.23 |
| /dev/bone/spi/1.0  | bone_spi_1 | P9.30 | P9.29 | P9.31 | P9.28 |
| /dev/bone/spi/1.1  |            | P9.30 | P9.29 | P9.31 | P9.42 |
| /dev/bone/spi/1.2  |            | P9.30 | P9.29 | P9.31 | P8.10 |

## UART

| Bone bus          | DT symbol    | TX     | RX     |
| ----------------- | ------------ | ------ | ------ |
| /dev/bone/uart/0  | bone_uart_0  | N/A    | N/A    |
| /dev/bone/uart/1  | bone_uart_1  | P9.24  | P9.26  |
| /dev/bone/uart/2  | bone_uart_2  | P9.21  | P9.22  |
| /dev/bone/uart/3  | bone_uart_3  | P9.42  | *n/a*  |
| /dev/bone/uart/4  | bone_uart_4  | P9.13  | P9.11  |
| /dev/bone/uart/5  | bone_uart_5  | P8.37  | P8.38  |
| /dev/bone/uart/6  | bone_uart_6  | P8.29  | P8.28  |
| /dev/bone/uart/7  | bone_uart_7  | P8.34  | P8.22  |

## CAN

| Bone bus         | TX     | RX     |
| ---------------- | ------ | ------ |
| /dev/bone/can/0  | P9.20  | P9.19  |
| /dev/bone/can/1  | P9.26  | P9.24  |
| /dev/bone/can/2  | P8.08  | P8.07  |
| /dev/bone/can/3  | P8.10  | P8.09  |
| /dev/bone/can/4  | P8.05  | P8.06  |

## ADC

| Index  | Header pin  |
| ------ | ----------- |
| 0      | P9.39       |
| 1      | P9.40       |
| 2      | P9.37       |
| 3      | P9.38       |
| 4      | P9.33       |
| 5      | P9.36       |
| 6      | P9.35       |

## PWM

| SYSFS link       | DT symbol  | A      | B      |
| ---------------- | ---------- | ------ | ------ |
| /dev/bone/pwm/0  | bone_pwm_0 | P9.22  | P9.21  |
| /dev/bone/pwm/1  | bone_pwm_1 | P9.14  | P9.16  |
| /dev/bone/pwm/2  | bone_pwm_2 | P8.19  | P8.13  |

## Counter

| SYSFS link           | DT symbol      | A     | B     |
| -------------------- | -------------- | ----- | ----- |
| /dev/bone/counter/0  | bone_counter_0 | P9.42 | P9.27 |
| /dev/bone/counter/1  | bone_counter_1 | P8.35 | P8.33 |
| /dev/bone/counter/2  | bone_counter_2 | P8.12 | P8.11 |
| /dev/bone/counter/3  | bone_counter_3 | P8.41 | P8.42 |
| /dev/bone/counter/4  | bone_counter_4 | P8.16 | P8.15 |

## LCD

### 0

| Header pin  | RGB565  |
| ----------- | ------- |
| P8.27       | VSYNC   |
| P8.28       | PCLK    |
| P8.29       | HSYNC   |
| P8.30       | DE      |
| P8.32       | R4      |
| P8.31       | R3      |
| P8.33       | R2      |
| P8.35       | R1      |
| P8.34       | R0      |
| P8.36       | G5      |
| P8.38       | G4      |
| P8.37       | G3      |
| P8.40       | G2      |
| P8.39       | G1      |
| P8.42       | G0      |
| P8.41       | B4      |
| P8.44       | B3      |
| P8.43       | B2      |
| P8.46       | B1      |
| P8.45       | B0      |

## I2S

### 0

| I2S     | Header pin  |
| ------- | ----------- |
| HCLK    | P9.25       |
| SCLK    | P9.31       |
| SCLK_IN | P9.12       |
| WS      | P9.29       |
| WS_IN   | P9.27       |
| SDO0    | P9.28       |
| SDI0    | P9.30       |
| SDO1    | P8.31       |
| SDI1    | P8.33       |

## SDIO

### 0

| Header pin  | Description  |
| ----------- | ------------ |
| P8.20       | CMD          |
| P8.21       | CLK          |
| P8.25       | DAT0         |
| P8.24       | DAT1         |
| P8.5        | DAT2         |
| P8.6        | DAT3         |
| P8.23       | DAT4         |
| P8.22       | DAT5         |
| P8.3        | DAT6         |
| P8.4        | DAT7         |

## mikroBUS

### By bus

| n | AN  | SPI | PWM | UART | I2C | RST   | INT   | CAN   | I2S | SDIO | LCD |
| - | --- | --- | --- | ---- | --- | ----- | ----- | ----- | --- | ---- | --- |
| 0 | TBD | TBD | TBD | TBD  | TBD | N/A   | N/A   | 0,N/A | N/A | N/A  | N/A |
| 1 | 0   | 1.0 | 1A  | 2    | 2   | P9.12 | P9.15 | 0,N/A | N/A | N/A  | N/A |
| 2 | 1   | 1.1 | 1B  | 1    | 2   | P9.23 | TBD   | 0,N/A | N/A | N/A  | N/A |
| 3 | 2   | 0.0 | 2A  | 1    | 2   | P8.14 | P8.18 | 0,N/A | N/A | N/A  | N/A |
| 4 | 3   | 1.2 | 2B  | 4    | 2   | P8.16 | P8.17 | 0,N/A | N/A | N/A  | N/A |
| 5 | TBD | TBD | TBD | TBD  | TBD | TBD   | TBD   | TBD   | 0   | N/A  | N/A |

### By pin

| n | AN    | RST   | CS    | SCK   | CIPO  | COPI  | PWM   | INT   | RX    | TX    | SCL   | SDA   |
| - | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
| 0 | N/A   | N/A   | N/A   | N/A   | N/A   | N/A   | N/A   | N/A   | N/A   | N/A   | N/A   | N/A   |
| 1 | P9.39 | P9.12 | P9.28 | P9.31 | P9.29 | P9.30 | P9.14 | P9.15 | P9.22 | P9.21 | P9.19 | P9.20 |
| 2 | P9.40 | P9.23 | P9.42 | P9.31 | P9.29 | P9.30 | P9.16 | TBD   | P9.26 | P9.24 | P9.19 | P9.20 |
| 3 | P9.37 | P8.14 | P9.17 | P9.22 | P9.21 | P9.18 | P8.19 | P8.18 | P9.26 | P9.24 | P9.19 | P9.20 |
| 4 | P9.38 | P9.23 | P9.42 | P9.31 | P9.29 | P9.30 | P8.13 | P8.17 | P9.11 | P9.13 | P9.19 | P9.20 |

### Reverse by pin

| Pin   | mikroBUS usage                                   |
| ----- | ------------------------------------------------ |
| P8.3  | TBD                                              |
| P8.4  | TBD                                              |
| P8.5  | TBD                                              |
| P8.6  | TBD                                              |
| P8.7  | TBD                                              |
| P8.8  | TBD                                              |
| P8.9  | TBD                                              |
| P8.10 | TBD                                              |
| P8.11 | TBD                                              |
| P8.12 | TBD                                              |
| P8.13 | 4                                                |
| P8.14 | 3                                                |
| P8.15 | TBD                                              |
| P8.16 | TBD                                              |
| P8.17 | 4                                                |
| P8.46 | TBD                                              |
| P9.11 | TBD                                              |
| P9.12 | TBD                                              |
| P9.42 | TBD                                              |
