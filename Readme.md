# RTOS Trace Viewer / Analyzer Tool

Lightweight RTOS (FreeRTOS + Bare Metal Designs with Timer Scheduler) Analyzer.

### Track/View

- Task switched IN / OUT
- Task created / deleted
- ISR entered / exited
- Queue transmit / receive
- Semaphore take / give
- System Eevent triggered

### Method

Primary (mostly used) tracing method is RTOS trace hook injection by providing a custom library.

- [Tracealyzer (Percepio)](https://percepio.com/tracealyzer/)
- [TI RTOS Analyzer](https://www.ti.com/tool/TI-RTOS-MCU)
- [QTrace FreeRTOS Viewer](https://www.pdqlogic.com/rtos-viewer/)