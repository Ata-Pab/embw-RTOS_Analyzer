# RTOS Trace Viewer / Analyzer Tool

Lightweight RTOS (FreeRTOS + Bare Metal Designs with Timer Scheduler) Analyzer.

### Environment/Platform/Framework
- Windows 10/11
- Qt Creator 19.0.0
- Based on Qt 6.10.2 (MSVC 2022, x86_64)
- QT Widget Application

### Track/View

- Task switched IN / OUT
- Task created / deleted
- ISR entered / exited
- Queue transmit / receive
- Semaphore take / give
- System Event triggered

### Method

Primary (mostly used) tracing method is RTOS trace hook injection by providing a custom library.

- [Tracealyzer (Percepio)](https://percepio.com/tracealyzer/)
- [TI RTOS Analyzer](https://www.ti.com/tool/TI-RTOS-MCU)
- [QTrace FreeRTOS Viewer](https://www.pdqlogic.com/rtos-viewer/)
