
/**
 * @file sfifo.h
 * @brief Static, fixed‑capacity lock‑free FIFO implementation.
 *
 * This module provides a compile‑time sized circular FIFO buffer with
 * zero dynamic allocation and no locking requirements. The FIFO is
 * implemented using a power‑of‑two sized ring buffer, enabling fast
 * index wrapping via bit masking.
 *
 * Features:
 *   - Fixed capacity defined at compile time
 *   - Lock‑free single‑producer / single‑consumer operation
 *   - Constant‑time push/pop/peek operations
 *   - Efficient bulk push/pop for contiguous or wrapped regions
 *   - Header‑only, fully inlined implementation
 *
 * Usage:
 *   DECLARE_SFIFO_TYPE(byte, uint8_t, 1024);
 *   sfifo_byte_1024_t fifo;
 *   sfifo_byte_1024_init(&fifo);
 *
 * Constraints:
 *   - FIFO size must be a power of two
 *   - Not safe for multi‑writer or multi‑reader without external sync
 *
 * This module is suitable for embedded systems, protocol stacks,
 * DMA pipelines, ISR-to-task communication, and any scenario requiring
 * deterministic, allocation‑free buffering.
 */

#ifndef __SFIFO_H__
#define __SFIFO_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Declare a static FIFO type with fixed capacity.
 *
 * This macro generates:
 *   - A struct type: sfifo_<name>_<size>_t
 *   - Inline functions for FIFO operations
 *
 * Requirements:
 *   - size must be a power of 2
 *
 * @param name  Logical name of the FIFO type
 * @param type  Element type stored in the FIFO
 * @param size  FIFO capacity (must be power of 2)
 */
#define DECLARE_SFIFO_TYPE(name, type, size)                                   \
  _Static_assert(((size) & ((size) - 1)) == 0,                                 \
                 "sfifo size must be power of 2");                             \
  typedef struct {                                                             \
    type buf[size];                                                            \
    uint32_t cap;                                                              \
    uint32_t msk;                                                              \
    volatile uint32_t i;                                                       \
    volatile uint32_t o;                                                       \
  } sfifo_##name##_##size##_t;                                                 \
                                                                               \
  /**                                                                          \
   * @brief Initialize FIFO state.                                             \
   *                                                                           \
   * Sets read/write indices to zero and configures capacity and mask.         \
   * Must be called before any push/pop operations.                            \
   *                                                                           \
   * @param f Pointer to FIFO instance                                         \
   * @return true if initialized, false if f is NULL                           \
   */                                                                          \
  static inline bool sfifo_##name##_##size##_init(                             \
      sfifo_##name##_##size##_t *f) {                                          \
    if (f == NULL) {                                                           \
      return false;                                                            \
    }                                                                          \
    f->i = f->o = 0;                                                           \
    f->cap = size;                                                             \
    f->msk = size - 1;                                                         \
    return true;                                                               \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Reset FIFO to empty state.                                         \
   *                                                                           \
   * Clears read/write indices without modifying buffer contents.              \
   * Safe to call at any time.                                                 \
   */                                                                          \
  static inline void sfifo_##name##_##size##_reset(                            \
      sfifo_##name##_##size##_t *f) {                                          \
    f->i = f->o = 0;                                                           \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Check whether FIFO contains no elements.                           \
   *                                                                           \
   * @return true if empty, false otherwise                                    \
   */                                                                          \
  static inline bool sfifo_##name##_##size##_is_empty(                         \
      const sfifo_##name##_##size##_t *f) {                                    \
    return f->i == f->o;                                                       \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Check whether FIFO is full.                                        \
   *                                                                           \
   * Full means (i - o) == capacity.                                           \
   *                                                                           \
   * @return true if full, false otherwise                                     \
   */                                                                          \
  static inline bool sfifo_##name##_##size##_is_full(                          \
      const sfifo_##name##_##size##_t *f) {                                    \
    return (f->i - f->o) == f->cap;                                            \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Get number of elements currently stored in FIFO.                   \
   *                                                                           \
   * @return Element count                                                     \
   */                                                                          \
  static inline uint32_t sfifo_##name##_##size##_count(                        \
      const sfifo_##name##_##size##_t *f) {                                    \
    return (f->i - f->o);                                                      \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Push one element into FIFO.                                        \
   *                                                                           \
   * @param f FIFO instance                                                    \
   * @param e Pointer to element to push                                       \
   * @return true if pushed, false if FIFO is full                             \
   */                                                                          \
  static inline bool sfifo_##name##_##size##_push(                             \
      sfifo_##name##_##size##_t *f, const type *e) {                           \
    uint32_t cnt = f->i - f->o;                                                \
    if (cnt < f->cap) {                                                        \
      uint32_t idx = f->i;                                                     \
      f->buf[idx & f->msk] = *e;                                               \
      f->i++;                                                                  \
      return true;                                                             \
    }                                                                          \
    return false;                                                              \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Pop one element from FIFO.                                         \
   *                                                                           \
   * @param f FIFO instance                                                    \
   * @param e Output pointer for popped element                                \
   * @return true if popped, false if FIFO is empty                            \
   */                                                                          \
  static inline bool sfifo_##name##_##size##_pop(sfifo_##name##_##size##_t *f, \
                                                 type *e) {                    \
    uint32_t cnt = f->i - f->o;                                                \
    if (cnt > 0) {                                                             \
      uint32_t idx = f->o;                                                     \
      *e = f->buf[idx & f->msk];                                               \
      f->o++;                                                                  \
      return true;                                                             \
    }                                                                          \
    return false;                                                              \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Drop (discard) one element from FIFO.                              \
   *                                                                           \
   * @return true if dropped, false if FIFO is empty                           \
   */                                                                          \
  static inline bool sfifo_##name##_##size##_drop(                             \
      sfifo_##name##_##size##_t *f) {                                          \
    uint32_t cnt = f->i - f->o;                                                \
    if (cnt > 0) {                                                             \
      f->o++;                                                                  \
      return true;                                                             \
    }                                                                          \
    return false;                                                              \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Peek the first element without removing it.                        \
   *                                                                           \
   * @param f FIFO instance                                                    \
   * @param e Output pointer for peeked element                                \
   * @return true if element available, false otherwise                        \
   */                                                                          \
  static inline bool sfifo_##name##_##size##_peek(                             \
      const sfifo_##name##_##size##_t *f, type *e) {                           \
    uint32_t cnt = f->i - f->o;                                                \
    uint32_t idx;                                                              \
    if (cnt > 0) {                                                             \
      idx = f->o;                                                              \
      *e = f->buf[idx & f->msk];                                               \
      return true;                                                             \
    }                                                                          \
    return false;                                                              \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Peek element at a given offset without removing it.                \
   *                                                                           \
   * @param f FIFO instance                                                    \
   * @param e Output pointer                                                   \
   * @param ofst Offset from head (0 = first element)                          \
   * @return true if valid offset, false otherwise                             \
   */                                                                          \
  static inline bool sfifo_##name##_##size##_peek_at(                          \
      const sfifo_##name##_##size##_t *f, type *e, uint32_t ofst) {            \
    uint32_t cnt = f->i - f->o;                                                \
    uint32_t idx;                                                              \
    if (ofst < cnt) {                                                          \
      idx = f->o;                                                              \
      *e = f->buf[(idx + ofst) & f->msk];                                      \
      return true;                                                             \
    }                                                                          \
    return false;                                                              \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Push an array of elements into FIFO.                               \
   *                                                                           \
   * Handles wrap_around automatically and performs at most two memcpy()       \
   * operations depending on buffer alignment.                                 \
   *                                                                           \
   * @param f FIFO instance                                                    \
   * @param arr Input array                                                    \
   * @param len Number of elements to push                                     \
   * @return true if all elements pushed, false if insufficient space          \
   */                                                                          \
  static inline bool sfifo_##name##_##size##_push_array(                       \
      sfifo_##name##_##size##_t *f, const type *arr, uint32_t len) {           \
    uint32_t in = *(volatile uint32_t *)&f->i;                                 \
    uint32_t out = *(volatile uint32_t *)&f->o;                                \
    uint32_t cap = f->cap;                                                     \
    uint32_t msk = f->msk;                                                     \
    uint32_t spc = cap - (in - out);                                           \
    if (len == 0) {                                                            \
      return true;                                                             \
    }                                                                          \
    if (len > spc) {                                                           \
      return false;                                                            \
    }                                                                          \
    uint32_t ofst = in & msk;                                                  \
    uint32_t l2e = cap - ofst;                                                 \
    uint32_t l1 = (len < l2e) ? len : l2e;                                     \
    memcpy(&f->buf[ofst], arr, l1 * sizeof(type));                             \
    if (len > l1) {                                                            \
      memcpy(f->buf, &arr[l1], (len - l1) * sizeof(type));                     \
    }                                                                          \
    *(volatile uint32_t *)&f->i = in + len;                                    \
    return true;                                                               \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Pop an array of elements from FIFO.                                \
   *                                                                           \
   * Handles wrap_around automatically and performs at most two memcpy()       \
   * operations depending on buffer alignment.                                 \
   *                                                                           \
   * @param f FIFO instance                                                    \
   * @param arr Output array                                                   \
   * @param len Number of elements to pop                                      \
   * @return true if all elements popped, false if insufficient data           \
   */                                                                          \
  static inline bool sfifo_##name##_##size##_pop_array(                        \
      sfifo_##name##_##size##_t *f, type *arr, uint32_t len) {                 \
    uint32_t in = *(volatile uint32_t *)&f->i;                                 \
    uint32_t out = *(volatile uint32_t *)&f->o;                                \
    uint32_t cap = f->cap;                                                     \
    uint32_t msk = f->msk;                                                     \
    uint32_t cnt = in - out;                                                   \
    if (len == 0) {                                                            \
      return true;                                                             \
    }                                                                          \
    if (cnt < len) {                                                           \
      return false;                                                            \
    }                                                                          \
    uint32_t ofst = out & msk;                                                 \
    uint32_t l2e = cap - ofst;                                                 \
    uint32_t l1 = (len < l2e) ? len : l2e;                                     \
    memcpy(arr, &f->buf[ofst], l1 * sizeof(type));                             \
    if (len > l1) {                                                            \
      memcpy(&arr[l1], f->buf, (len - l1) * sizeof(type));                     \
    }                                                                          \
    *(volatile uint32_t *)&f->o = out + len;                                   \
    return true;                                                               \
  }

#endif //! __SFIFO_H__
