
# gfifo -- A Lightweight Generic FIFO Library for C

## Intro

A lightweight, zero‑dependency, embeddable, and reusable FIFO (ring buffer) library for C.
Designed for embedded systems, device drivers, protocol stacks, and high‑performance data pipelines.

This library provides:

+ A type‑safe FIFO template
+ O(1) enqueue/dequeue operations
+ Configurable memory model
+ Optional zero dynamic allocation
+ Clean, readable, maintainable API

> This document generated via gpt :)

## Features

+ **Lightweight implementation**: small, self‑contained codebase
+ **Minimal dependencies**: only requires basic C standard library (`string.h`)
- **No OS dependencies**: works in bare‑metal and RTOS environments
- **Generic design**: can store arbitrary element types
- **Embedded‑friendly**: suitable for STM32 / GD32 / ESP32 / Linux / other MCUs
- **Predictable performance**: constant‑time operations for enqueue/dequeue
- **Configurable memory model**: static buffers, user‑provided storage
- **Optional extensions**: can be wrapped for thread safety if needed

## Usage

### Include Header File

This library has only one header file, you can add the following files to your project:

```c
#include "gfifo.h"
```

The library assumes:

+ A C99(or later version) compiler.
+ Availability of basic C standard library headers (e.g. string.h)

### Quick Start

Create a FIFO (example type `uint8_t`)

```c
#include <stdint.h>
#include "gfifo.h"

// size of buffer must be power of 2!
#define FIFO_BUF_SIZE (256)
// declare fifo structure and its APIs.
DECLARE_GFIFO_TYPE(byte, uint8_t);

static uint8_t fifo_buf[FIFO_BUF_SIZE];
static gfifo_byte_t fifo_test;

int main(void)
{
	if(gfifo_byte_init(&fifo_test, fifo_buf, FIFO_BUF_SIZE))
	{
		printf("fifo create successfully");
	}
	else
	{
		printf("fifo create failed");
	}
}
```

More information you can see the comment in the `gfifo.h`.
