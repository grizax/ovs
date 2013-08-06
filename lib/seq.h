/*
 * Copyright (c) 2013 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SEQ_H
#define SEQ_H 1

/* Thread-safe, pollable sequence number.
 *
 *
 * Background
 * ==========
 *
 * It is sometimes desirable to take an action whenever an object changes.
 * Suppose we associate a sequence number with an object and increment the
 * sequence number whenver we change the object.  An observer can then record
 * the sequence number it sees.  Later on, if the current sequence number
 * differs from the one it saw last, then the observer knows to examine the
 * object for changes.
 *
 * Code that wants to run when a sequence number changes is challenging to
 * implement in a multithreaded environment.  A naive implementation, that
 * simply checks whether the sequence number changed and, if so, calls
 * poll_immediate_wake(), will fail when another thread increments the sequence
 * number after the check (including during poll_block()).
 *
 * struct seq is a solution.  It implements a sequence number along with enough
 * internal infrastructure so that a thread waiting on a particular value will
 * wake up if the sequence number changes, or even if the "struct seq" is
 * destroyed.
 *
 *
 * Usage
 * =====
 *
 * The object that includes a sequence number should use seq_create() and
 * seq_destroy() at creation and destruction, and seq_change() whenever the
 * object's observable state changes.
 *
 * An observer may seq_read() to read the current sequence number and
 * seq_wait() to cause poll_block() to wake up when the sequence number changes
 * from a specified value.
 *
 * To avoid races, observers should use seq_read() to check for changes,
 * process any changes, and then use seq_wait() to wait for a change from the
 * previously read value.  That is, a correct usage looks something like this:
 *
 *    new_seq = seq_read(seq);
 *    if (new_seq != last_seq) {
 *        ...process changes...
 *        last_seq = new_seq;
 *    }
 *    seq_wait(seq, new_seq);
 *    poll_block();
 *
 *
 * Thread-safety
 * =============
 *
 * Fully thread safe.
 */

#include <stdint.h>

/* For implementation of an object with a sequence number attached. */
struct seq *seq_create(void);
void seq_destroy(struct seq *);
void seq_change(struct seq *);

/* For observers. */
uint64_t seq_read(const struct seq *);
void seq_wait(const struct seq *, uint64_t value);

/* For poll_block() internal use. */
void seq_woke(void);

#endif /* seq.h */
