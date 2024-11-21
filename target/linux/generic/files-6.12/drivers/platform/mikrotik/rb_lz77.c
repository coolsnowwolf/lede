// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 John Thomson
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/minmax.h>

#include "rb_lz77.h"

#define MIKRO_LZ77 "[rb lz77] "

/*
 * The maximum number of bits used in a counter.
 * For the look behind window, long instruction match offsets
 * up to 6449 have been seen in provided compressed caldata blobs
 * (that would need 21 counter bits: 4 to 12 + 11 to 0).
 * conservative value here: 27 provides offset up to 0x8000 bytes
 * uses a u8 in this code
 */
#define MIKRO_LZ77_MAX_COUNT_BIT_LEN 27

enum rb_lz77_instruction {
	INSTR_ERROR = -1,
	INSTR_LITERAL_BYTE = 0,
	/* a (non aligned) byte follows this instruction,
	 * which is directly copied into output
	 */
	INSTR_PREVIOUS_OFFSET = 1,
	/* this group is a match, with a bytes length defined by
	 * following counter bits, starting at bitshift 0,
	 * less the built-in count of 1
	 * using the previous offset as source
	 */
	INSTR_LONG = 2
	/* this group has two counters,
	 * the first counter starts at bitshift 4,
	 *	 if this counter == 0, this is a non-matching group
	 *	 the second counter (bytes length) starts at bitshift 4,
	 *	 less the built-in count of 11+1.
	 *	 The final match group has this count 0,
	 *	 and following bits which pad to byte-alignment.
	 *
	 *	 if this counter > 0, this is a matching group
	 *	 this first count is the match offset (in bytes)
	 *	 the second count is the match length (in bytes),
	 *	 less the built-in count of 2
	 *	 these groups can source bytes that are part of this group
	 */
};

struct rb_lz77_instr_opcodes {
	/* group instruction */
	enum rb_lz77_instruction instruction;
	/* if >0, a match group,
	 * which starts at byte output_position - 1*offset
	 */
	size_t offset;
	/* how long the match group is,
	 * or how long the (following counter) non-match group is
	 */
	size_t length;
	/* how many bits were used for this instruction + op code(s) */
	size_t bits_used;
	/* input char */
	u8 *in;
	/* offset where this instruction started */
	size_t in_pos;
};

/**
 * rb_lz77_get_bit
 *
 * @in:			compressed data ptr
 * @in_offset_bit:	bit offset to extract
 *
 * convert the bit offset to byte offset,
 * shift to modulo of bits per bytes, so that wanted bit is lsb
 * and to extract only that bit.
 * Caller is responsible for ensuring that in_offset_bit/8
 * does not exceed input length
 */
static inline u8 rb_lz77_get_bit(const u8 *in, const size_t in_offset_bit)
{
	return ((in[in_offset_bit / BITS_PER_BYTE] >>
		 (in_offset_bit % BITS_PER_BYTE)) &
		1);
}

/**
 * rb_lz77_get_byte
 *
 * @in:			compressed data
 * @in_offset_bit:	bit offset to extract byte
 */
static inline u8 rb_lz77_get_byte(const u8 *in, const size_t in_offset_bit)
{
	u8 buf = 0;
	int i;

	/* built a reversed byte from (likely) unaligned bits */
	for (i = 0; i <= 7; ++i)
		buf += rb_lz77_get_bit(in, in_offset_bit + i) << (7 - i);
	return buf;
}

/**
 * rb_lz77_decode_count - decode bits at given offset as a count
 *
 * @in:			compressed data
 * @in_len:		length of compressed data
 * @in_offset_bit:	bit offset where count starts
 * @shift:		left shift operand value of first count bit
 * @count:		initial count
 * @bits_used:		how many bits were consumed by this count
 * @max_bits:		maximum bit count for this counter
 *
 * Returns the decoded count
 */
static int rb_lz77_decode_count(const u8 *in, const size_t in_len,
				const size_t in_offset_bit, u8 shift,
				size_t count, u8 *bits_used, const u8 max_bits)
{
	size_t pos = in_offset_bit;
	const size_t max_pos = min(pos + max_bits, in_len * BITS_PER_BYTE);
	bool up = true;

	*bits_used = 0;
	pr_debug(MIKRO_LZ77
		 "decode_count inbit: %zu, start shift:%u, initial count:%zu\n",
		 in_offset_bit, shift, count);

	while (true) {
		/* check the input offset bit does not overflow the minimum of
		 * a reasonable length for this encoded count, and
		 * the end of the input */
		if (unlikely(pos >= max_pos)) {
			pr_err(MIKRO_LZ77
			       "max bit index reached before count completed\n");
			return -EFBIG;
		}

		/* if the bit value at offset is set */
		if (rb_lz77_get_bit(in, pos))
			count += (1 << shift);

		/* shift increases until we find an unsed bit */
		else if (up)
			up = false;

		if (up)
			++shift;
		else {
			if (!shift) {
				*bits_used = pos - in_offset_bit + 1;
				return count;
			}
			--shift;
		}

		++pos;
	}

	return -EINVAL;
}

/**
 * rb_lz77_decode_instruction
 *
 * @in:			compressed data
 * @in_offset_bit:	bit offset where instruction starts
 * @bits_used:		how many bits were consumed by this count
 *
 * Returns the decoded instruction
 */
static enum rb_lz77_instruction
rb_lz77_decode_instruction(const u8 *in, size_t in_offset_bit, u8 *bits_used)
{
	if (rb_lz77_get_bit(in, in_offset_bit)) {
		*bits_used = 2;
		if (rb_lz77_get_bit(in, ++in_offset_bit))
			return INSTR_LONG;
		else
			return INSTR_PREVIOUS_OFFSET;
	} else {
		*bits_used = 1;
		return INSTR_LITERAL_BYTE;
	}
	return INSTR_ERROR;
}

/**
 * rb_lz77_decode_instruction_operators
 *
 * @in:			compressed data
 * @in_len:		length of compressed data
 * @in_offset_bit:	bit offset where instruction starts
 * @previous_offset:	last used match offset
 * @opcode:		struct to hold instruction & operators
 *
 * Returns error code
 */
static int rb_lz77_decode_instruction_operators(
	const u8 *in, const size_t in_len, const size_t in_offset_bit,
	const size_t previous_offset, struct rb_lz77_instr_opcodes *opcode)
{
	enum rb_lz77_instruction instruction;
	u8 bit_count = 0;
	u8 bits_used = 0;
	int offset = 0;
	int length = 0;

	instruction = rb_lz77_decode_instruction(in, in_offset_bit, &bit_count);

	/* skip bits used by instruction */
	bits_used += bit_count;

	switch (instruction) {
	case INSTR_LITERAL_BYTE:
		/* non-matching char */
		offset = 0;
		length = 1;
		break;

	case INSTR_PREVIOUS_OFFSET:
		/* matching group uses previous offset */
		offset = previous_offset;

		length = rb_lz77_decode_count(in, in_len,
					      in_offset_bit + bits_used, 0, 1,
					      &bit_count,
					      MIKRO_LZ77_MAX_COUNT_BIT_LEN);
		if (unlikely(length < 0))
			return length;
		/* skip bits used by count */
		bits_used += bit_count;
		break;

	case INSTR_LONG:
		offset = rb_lz77_decode_count(in, in_len,
					      in_offset_bit + bits_used, 4, 0,
					      &bit_count,
					      MIKRO_LZ77_MAX_COUNT_BIT_LEN);
		if (unlikely(offset < 0))
			return offset;

		/* skip bits used by offset count */
		bits_used += bit_count;

		if (offset == 0) {
			/* non-matching long group */
			length = rb_lz77_decode_count(
				in, in_len, in_offset_bit + bits_used, 4, 12,
				&bit_count, MIKRO_LZ77_MAX_COUNT_BIT_LEN);
			if (unlikely(length < 0))
				return length;
			/* skip bits used by length count */
			bits_used += bit_count;
		} else {
			/* matching group */
			length = rb_lz77_decode_count(
				in, in_len, in_offset_bit + bits_used, 0, 2,
				&bit_count, MIKRO_LZ77_MAX_COUNT_BIT_LEN);
			if (unlikely(length < 0))
				return length;
			/* skip bits used by length count */
			bits_used += bit_count;
		}

		break;

	case INSTR_ERROR:
		return -EINVAL;
	}

	opcode->instruction = instruction;
	opcode->offset = offset;
	opcode->length = length;
	opcode->bits_used = bits_used;
	opcode->in = (u8 *)in;
	opcode->in_pos = in_offset_bit;
	return 0;
}

/**
 * rb_lz77_decompress
 *
 * @in:			compressed data ptr
 * @in_len:		length of compressed data
 * @out:		buffer ptr to decompress into
 * @out_len:		length of decompressed buffer in input,
 *			length of decompressed data in success
 *
 * Returns 0 on success, or negative error
 */
int rb_lz77_decompress(const u8 *in, const size_t in_len, u8 *out,
		       size_t *out_len)
{
	u8 *output_ptr;
	size_t input_bit = 0;
	const u8 *output_end = out + *out_len;
	struct rb_lz77_instr_opcodes *opcode;
	size_t match_offset = 0;
	int rc = 0;
	size_t match_length, partial_count, i;

	output_ptr = out;

	if (unlikely((in_len * BITS_PER_BYTE) > SIZE_MAX)) {
		pr_err(MIKRO_LZ77 "input longer than expected\n");
		return -EFBIG;
	}

	opcode = kmalloc(sizeof(*opcode), GFP_KERNEL);
	if (!opcode)
		return -ENOMEM;

	while (true) {
		if (unlikely(output_ptr > output_end)) {
			pr_err(MIKRO_LZ77 "output overrun\n");
			rc = -EOVERFLOW;
			goto free_lz77_struct;
		}
		if (unlikely(input_bit > in_len * BITS_PER_BYTE)) {
			pr_err(MIKRO_LZ77 "input overrun\n");
			rc = -ENODATA;
			goto free_lz77_struct;
		}

		rc = rb_lz77_decode_instruction_operators(in, in_len, input_bit,
							  match_offset, opcode);
		if (unlikely(rc < 0)) {
			pr_err(MIKRO_LZ77
			       "instruction operands decode error\n");
			goto free_lz77_struct;
		}

		pr_debug(MIKRO_LZ77 "inbit:0x%zx->outbyte:0x%zx", input_bit,
			 output_ptr - out);

		input_bit += opcode->bits_used;
		switch (opcode->instruction) {
		case INSTR_LITERAL_BYTE:
			pr_debug(" short");
			fallthrough;
		case INSTR_LONG:
			if (opcode->offset == 0) {
				/* this is a non-matching group */
				pr_debug(" non-match, len: 0x%zx\n",
					 opcode->length);
				/* test end marker */
				if (opcode->length == 0xc &&
				    ((input_bit +
				      opcode->length * BITS_PER_BYTE) >
				     in_len)) {
					*out_len = output_ptr - out;
					pr_debug(
						MIKRO_LZ77
						"lz77 decompressed from %zu to %zu\n",
						in_len, *out_len);
					rc = 0;
					goto free_lz77_struct;
				}
				for (i = opcode->length; i > 0; --i) {
					*output_ptr =
						rb_lz77_get_byte(in, input_bit);
					++output_ptr;
					input_bit += BITS_PER_BYTE;
				}
				/* do no fallthrough if a non-match group */
				break;
			}
			match_offset = opcode->offset;
			fallthrough;
		case INSTR_PREVIOUS_OFFSET:
			match_length = opcode->length;
			partial_count = 0;

			pr_debug(" match, offset: 0x%zx, len: 0x%zx",
				 opcode->offset, match_length);

			if (unlikely(opcode->offset == 0)) {
				pr_err(MIKRO_LZ77
				       "match group missing opcode->offset\n");
				rc = -EBADMSG;
				goto free_lz77_struct;
			}

			/* overflow */
			if (unlikely((output_ptr + match_length) >
				     output_end)) {
				pr_err(MIKRO_LZ77
				       "match group output overflow\n");
				rc = -ENOBUFS;
				goto free_lz77_struct;
			}

			/* underflow */
			if (unlikely((output_ptr - opcode->offset) < out)) {
				pr_err(MIKRO_LZ77
				       "match group offset underflow\n");
				rc = -ESPIPE;
				goto free_lz77_struct;
			}

			/* there are cases where the match (length) includes
			 * data that is a part of the same match
			 */
			while (opcode->offset < match_length) {
				++partial_count;
				memcpy(output_ptr, output_ptr - opcode->offset,
				       opcode->offset);
				output_ptr += opcode->offset;
				match_length -= opcode->offset;
			}
			memcpy(output_ptr, output_ptr - opcode->offset,
			       match_length);
			output_ptr += match_length;
			if (partial_count)
				pr_debug(" (%zu partial memcpy)",
					 partial_count);
			pr_debug("\n");

			break;

		case INSTR_ERROR:
			rc = -EINVAL;
			goto free_lz77_struct;
		}
	}

	pr_err(MIKRO_LZ77 "decode loop broken\n");
	rc = -EINVAL;

free_lz77_struct:
	kfree(opcode);
	return rc;
}
EXPORT_SYMBOL_GPL(rb_lz77_decompress);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Mikrotik Wi-Fi caldata LZ77 decompressor");
MODULE_AUTHOR("John Thomson");
