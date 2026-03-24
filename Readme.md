# RTOS Trace Viewer / Analyzer Tool for Virtual Embedded Systems

Lightweight RTOS (FreeRTOS + Bare Metal Designs with Timer Scheduler) Analyzer. This tool is designed to visualize and analyze the behavior of RTOS-based embedded systems in a virtual environment, not on actual hardware. It captures various events such as task switches, ISR entries/exits, queue operations, and semaphore interactions, providing insights into the system's performance and behavior. Simulation the peripheral events and system events to generate trace data for analysis is developer's responsibility. The main focus of this tool is to provide an interface for developers to detect algorithm bugs, inefficiencies, identify bottlenecks and optimize performance. It is not intended for real-time debugging or hardware-level analysis, but rather for understanding the overall system behavior and performance in a simulated environment. In fact it accelerates the development process by allowing developers to run and analyze their RTOS-based applications in a virtual environment with higher speed and flexibility compared to real hardware.

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
