# Hướng dẫn sử dụng lệnh điều khiển

## 1. Điều khiển Stepper Motor

| Chức năng                  | Lệnh                              | Mô tả                                                                 |
|---------------------------|-----------------------------------|-----------------------------------------------------------------------|
| Enable/Disable Driver     | `step on`                         | Bật driver                                                            |
|                           | `step off`                        | Tắt driver                                                            |
| Set Direction             | `step dir 0`                      | CCW
|                           | `step dir 1`                      | CW
| Set Speed                 | `step speed x`                    | Đặt tốc độ<br>*Lưu ý: 0 < x ≤ 10000*                                 |
| Set MicroStep             | `step ms x y`                     | Cài đặt MicroStep<br>**x**: 1/2 (MS1/MS2)<br>**y**: 1/0 (On/Off)     |
| Enable PWM Output         | `step start`                      | Bật xuất PWM                                                          |
| Disable PWM Output        | `step stop`                       | Tắt xuất PWM                                                          |

## 2. Điều khiển DC Motor

| Chức năng                        | Lệnh                  | Mô tả                                      |
|----------------------------------|-----------------------|--------------------------------------------|
| Set Speed (Single Channel)       | `dc ch 1 100`         | Đặt tốc độ cho kênh 1                      |
|                                  | `dc ch 2 200`         | Đặt tốc độ cho kênh 2                      |
|                                  | `dc ch 3 50`          | Đặt tốc độ cho kênh 3                      |
| Enable PWM Output                | `dc start 1`          | Bật PWM cho kênh 1                         |
|                                  | `dc start 2`          | Bật PWM cho kênh 2                         |
|                                  | `dc start 3`          | Bật PWM cho kênh 3                         |

**Ghi chú:**
- Các lệnh `dc ch` dùng để đặt tốc độ cho từng kênh riêng lẻ.
- Các lệnh `dc start` dùng để bật xuất PWM cho từng kênh.