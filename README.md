# UART-Controlled Analog Signal Visualization (AVR XMEGA, 12-bit ADC)

This project implements **real-time analog signal acquisition and visualization** on an AVR XMEGA microcontroller.  
A **12-bit differential ADC with gain** samples analog inputs, while the **event system** triggers conversions via timer overflow.  
Captured data is streamed over **UART** for plotting or monitoring on a connected PC, with runtime input commands to switch signal sources.

## Features
- **12-bit ADC sampling** with differential inputs and gain for high-resolution measurement.
- **Event-driven acquisition** using timer overflow to eliminate CPU polling.
- **Interrupt-based UART communication** for real-time data transmission.
- **Command interface** to switch between multiple analog sources on-the-fly.

## Hardware & Tools
- **Microcontroller**: AVR XMEGA
- **Language**: C
- **Tools**: Atmel Studio / avr-gcc, ModelSim (for UART data testing), Serial Plotter
- **Interfaces**: ADC, Event System, USART

## How It Works
1. Timer overflow triggers ADC conversions via the AVR XMEGA event system.
2. ADC results are processed in an interrupt service routine (ISR).
3. Data is transmitted over UART in both decimal and hexadecimal formats.
4. Host PC can send commands (`B` or `F`) to change the measurement source.