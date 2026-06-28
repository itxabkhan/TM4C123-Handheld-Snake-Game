# TM4C123GH6PM Interfacing with ST7735S TFT Display (Handheld-Snake-Game)

<img width="1634" height="640" alt="Game" src="https://github.com/user-attachments/assets/c5f6a569-a079-49d1-a115-0b27defb6270" />

## 📌 Overview
This project demonstrates interfacing a **ST7735S TFT LCD display** with the
**TM4C123GH6PM (Tiva C LaunchPad)** using GPIO-based SPI communication.

The system supports:
- TFT graphics display
- Button-based navigation
- Embedded graphics rendering

This project was developed as part of an **Embedded Systems / Open Ended Lab**.

---

## 🎯 Objectives
- Interface ST7735S TFT LCD with TM4C123
- Implement SPI communication
- Display graphics and text on TFT
- Handle user input using push buttons
- Develop low-level register-based drivers

---

## 🧰 Hardware Used
- TM4C123GH6PM (Tiva C LaunchPad)
- ST7735S TFT LCD
- Push Buttons
- 3.3V Power Supply
- Breadboard & jumper wires

---

## 🔌 Pin Configuration

### 🖥️ ST7735S TFT Display (SPI)

| TFT Signal | TM4C123 Pin |
|-----------|-------------|
| SCL (CLK) | PB4 |
| SDA (MOSI) | PB7 |
| RES | PB1 |
| DC | PB3 |
| CS | PB2 |
| BLK | 3.3V |

> Backlight is powered externally at 3.3V.

---

### 🎮 Push Buttons

| Button | TM4C Pin |
|------|---------|
| Left | PA2 |
| Right | PA3 |
| Up | PA4 |
| Down | PA5 |
| Select | PA6 |

---

## ⚙️ Software Features
- GPIO initialization using register-level programming
- SPI-based TFT command and data transfer
- Button debouncing and input detection
- Screen updates based on user input

---

## 📂 Project Structure

---

## 🧪 How It Works
1. TM4C initializes GPIO and SPI pins
2. ST7735S is reset and configured
3. Display is cleared and initialized
4. Button inputs are monitored
5. Display updates according to user actions

---

## ⚠️ Important Notes
- Ensure TFT operates at **3.3V only**
- Do NOT connect TFT to 5V
- Use common ground between MCU and TFT

---

## 📜 License
This project is open-source and intended for **educational use only**.

---

## 👤 Author
**Ali Ahsan**  
Department of Electrical Engineering  
UET Lahore

