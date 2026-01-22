
/**
 * @file gfifo.h
 * @brief Generic typed ring FIFO implementation.
 *
 * @details
 * Provides a macro to declare a strongly-typed FIFO and its full API:
 *   - init/reset
 *   - push/pop/drop
 *   - peek/peek_at
 *   - bulk push/pop with wrap-around handling
 *
 * Designed for SPSC usage with power-of-two capacity.
 *
 * @author
 *   Disen-Shaw <DisenShaw@gmail.com>
 * @date
 *   2026-01-19
 * @license MIT
 */

#ifndef __GFIFO_H__
#define __GFIFO_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief  Declare a generic ring FIFO type and its associated operations.
 *
 * This macro generates a strongly‑typed FIFO structure and a full set of
 * inline operations, including push/pop, peek, drop, and bulk array
 * read/write. The FIFO uses a power‑of‑two sized ring buffer and relies on
 * natural unsigned overflow of monotonic indices (i/o) for distance
 * calculation.
 *
 * @param name  Suffix used to form the FIFO type name.
 * @param type  Element type stored in the FIFO.
 *
 * The generated type is:
 *     gfifo_<name>_t
 *
 * All operations are O(1) and safe for single‑producer / single‑consumer
 * (SPSC) usage. Multi‑threaded usage requires external synchronization.
 */
#define DECLARE_GFIFO_TYPE(name, type)                                         \
  typedef struct {                                                             \
    type *buf;                                                                 \
    uint32_t cap;                                                              \
    uint32_t msk;                                                              \
    volatile uint32_t i;                                                       \
    volatile uint32_t o;                                                       \
  } gfifo_##name##_t;                                                          \
                                                                               \
  /**                                                                          \
   * @brief Initialize FIFO with user_provided buffer.                         \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   *                                                                           \
   * @return true  Initialization succeeded.                                   \
   * @return false Invalid size or NULL buffer.                                \
   */                                                                          \
  static inline bool gfifo_##name##_init(gfifo_##name##_t *f, type *buf,       \
                                         uint32_t size) {                      \
    if (size == 0 || (size & (size - 1)) != 0 || buf == NULL) {                \
      return false;                                                            \
    }                                                                          \
    f->i = f->o = 0;                                                           \
    f->buf = buf;                                                              \
    f->cap = size;                                                             \
    f->msk = size - 1;                                                         \
    return true;                                                               \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief  Reset FIFO indices, clearing all stored elements.                 \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   */                                                                          \
  static inline void gfifo_##name##_reset(gfifo_##name##_t *f) {               \
    f->i = f->o = 0;                                                           \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief  Check whether FIFO is empty.                                      \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   * @return true  FIFO contains no elements.                                  \
   * @return false FIFO has at least one element.                              \
   */                                                                          \
  static inline bool gfifo_##name##_is_empty(const gfifo_##name##_t *f) {      \
    return f->i == f->o;                                                       \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief  Check whether FIFO is full.                                       \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   * @return true  FIFO is full.                                               \
   * @return false FIFO has free space.                                        \
   */                                                                          \
  static inline bool gfifo_##name##_is_full(const gfifo_##name##_t *f) {       \
    return (f->i - f->o) == f->cap;                                            \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Get number of stored elements.                                     \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   * @return Number of elements currently in FIFO.                             \
   */                                                                          \
  static inline uint32_t gfifo_##name##_count(const gfifo_##name##_t *f) {     \
    return (f->i - f->o);                                                      \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Push a single element into FIFO.                                   \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   * @param e Pointer to element to push.                                      \
   *                                                                           \
   * @return true  Element pushed.                                             \
   * @return false FIFO is full.                                               \
   */                                                                          \
  static inline bool gfifo_##name##_push(gfifo_##name##_t *f, const type *e) { \
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
   * @brief Pop a single element from FIFO.                                    \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   * @param e Pointer to output element.                                       \
   *                                                                           \
   * @return true  Element popped.                                             \
   * @return false FIFO is empty.                                              \
   */                                                                          \
  static inline bool gfifo_##name##_pop(gfifo_##name##_t *f, type *e) {        \
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
   * @brief Drop one element from FIFO without returning it.                   \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   *                                                                           \
   * @return true  Element dropped.                                            \
   * @return false FIFO is empty.                                              \
   */                                                                          \
  static inline bool gfifo_##name##_drop(gfifo_##name##_t *f) {                \
    uint32_t cnt = f->i - f->o;                                                \
    if (cnt > 0) {                                                             \
      f->o++;                                                                  \
      return true;                                                             \
    }                                                                          \
    return false;                                                              \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Drop Number of element from FIFO without returning it.             \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   * @param cnt Number of element to drop                                      \
   *                                                                           \
   * @return true  Element dropped.                                            \
   * @return false FIFO is empty.                                              \
   */                                                                          \
  static inline bool gfifo_##name##_drop_multi(gfifo_##name##_t *f,            \
                                               uint32_t cnt) {                 \
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
    *(volatile uint32_t *)&f->o = out + len;                                   \
    return true;                                                               \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Read the first element without removing it.                        \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   * @param e Pointer to output element.                                       \
   *                                                                           \
   * @return true  Element available.                                          \
   * @return false FIFO is empty.                                              \
   */                                                                          \
  static inline bool gfifo_##name##_peek(const gfifo_##name##_t *f, type *e) { \
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
   * @brief Read element at offset without removing it.                        \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   * @param e Pointer to output element.                                       \
   * @param ofst Offset from head (0 = first element).                         \
   *                                                                           \
   * @return true  Element available.                                          \
   * @return false Offset out of range.                                        \
   */                                                                          \
  static inline bool gfifo_##name##_peek_at(const gfifo_##name##_t *f,         \
                                            type *e, uint32_t ofst) {          \
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
   * @brief Push multiple elements into FIFO.                                  \
   *                                                                           \
   * Handles wrap_around automatically and uses memcpy() for efficient bulk    \
   * transfer.                                                                 \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   * @param arr Source array.                                                  \
   * @param len Number of elements to push.                                    \
   *                                                                           \
   * @return true  All elements pushed.                                        \
   * @return false Not enough free space.                                      \
   */                                                                          \
  static inline bool gfifo_##name##_push_array(                                \
      gfifo_##name##_t *f, const type *arr, uint32_t len) {                    \
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
   * @brief Pop multiple elements from FIFO.                                   \
   *                                                                           \
   * Handles wrap_around automatically and uses memcpy() for efficient bulk    \
   * transfer.                                                                 \
   *                                                                           \
   * @param f FIFO instance.                                                   \
   * @param arr Destination array.                                             \
   * @param len Number of elements to pop.                                     \
   *                                                                           \
   * @return true  All elements popped.                                        \
   * @return false FIFO does not contain enough elements.                      \
   */                                                                          \
  static inline bool gfifo_##name##_pop_array(gfifo_##name##_t *f, type *arr,  \
                                              uint32_t len) {                  \
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

#endif //! __GFIFO_H__
