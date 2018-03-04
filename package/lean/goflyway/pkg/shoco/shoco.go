package shoco

/*
 * this is a go implementation of shoco alogrithm
 * shoco is a C library to compress and decompress short strings.
 * shoco is free software, distributed under the MIT license
 * original project address: https://github.com/Ed-von-Schleck/shoco
 */

import (
	"encoding/binary"
	"fmt"
)

const SIZE_MAX = -1

var _ = fmt.Println

func decode_header(val byte) int {
	i := -1
	v := uint8(val)

	for int8(v) < 0 {
		v <<= 1
		i++
	}

	return i
}

func check_indices(indices []int16, pack_n int) bool {
	for i := 0; i < int(packs[pack_n].bytes_unpacked); i++ {
		if indices[i] > packs[pack_n].masks[i] {
			return false
		}
	}

	return true
}

func find_best_encoding(indices []int16, n_consecutive int) int {
	for p := PACK_COUNT - 1; p >= 0; p-- {
		if uint32(n_consecutive) >= packs[p].bytes_unpacked && check_indices(indices, p) {
			return p
		}
	}

	return -1
}

func shoco_compress(in []byte, out []byte) int {
	in = append(in, 0) // append NULL like C

	indices := make([]int16, MAX_SUCCESSOR_N+1)
	last_chr_index := int32(0)
	current_index := int32(0)
	current_out_index := 0
	successor_index := int32(0)
	n_consecutive := 0
	pack_n := 0

	last_resort := func() (bool, int) {
		if (in[0] & 0x80) != 0 {
			// non-ascii case
			if current_out_index+2 > len(out) {
				return true, len(out) // + 1
			}
			// put in a sentinel byte
			// *o++ = 0x00;
			out[current_out_index] = 0x00
			current_out_index++
		} else {
			// an ascii byte
			if current_out_index+1 > len(out) {
				return true, len(out) // + 1
			}
		}
		// *o++ = *in++;
		out[current_out_index] = in[0]
		current_out_index++
		in = in[1:]

		return false, 0
	}

	for len(in) > 0 {
		indices[0] = int16(chr_ids_by_chr[in[0]])
		last_chr_index = int32(indices[0])
		if last_chr_index < 0 {
			if r, v := last_resort(); r {
				return v
			} else {
				continue
			}
		}

		rest := len(in)
		for n_consecutive = 1; n_consecutive <= MAX_SUCCESSOR_N; n_consecutive++ {
			if n_consecutive == rest {
				break
			}

			if current_index = int32(chr_ids_by_chr[in[n_consecutive]]); current_index < 0 {
				break
			}

			successor_index = int32(successor_ids_by_chr_id_and_chr_id[last_chr_index][current_index])
			if successor_index < 0 {
				break
			}

			indices[n_consecutive] = int16(successor_index)
			last_chr_index = current_index
		}

		if n_consecutive < 2 {
			if r, v := last_resort(); r {
				return v
			} else {
				continue
			}
		}

		pack_n = find_best_encoding(indices, n_consecutive)
		if pack_n >= 0 {
			if current_out_index+int(packs[pack_n].bytes_packed) > len(out) {
				return len(out) //+ 1
			}

			word := packs[pack_n].word
			for i := uint32(0); i < packs[pack_n].bytes_unpacked; i++ {
				word |= uint32(indices[i]) << packs[pack_n].offsets[i]
			}

			tmp := make([]byte, 4)
			binary.BigEndian.PutUint32(tmp, word)

			for i := uint32(0); i < packs[pack_n].bytes_packed; i++ {
				out[i+uint32(current_out_index)] = tmp[i]
			}

			current_out_index += int(packs[pack_n].bytes_packed)
			in = in[packs[pack_n].bytes_unpacked:]
		} else {
			if r, v := last_resort(); r {
				return v
			}
		}
	}

	return current_out_index - 1
}

func shoco_decompress(in []byte, out []byte) int {
	mark := 0
	current_out_index := 0
	offset := uint32(0)
	mask := uint32(0)
	var last_chr byte

	for len(in) > 0 {
		mark = decode_header(in[0])
		if mark >= len(packs) {
			return 0
		}

		if mark < 0 {
			if current_out_index > len(out) {
				return len(out)
			}

			if in[0] == 0x00 {
				in = in[1:]
				if len(in) == 0 {
					return SIZE_MAX
				}
			}

			out[current_out_index] = in[0]
			current_out_index++
			in = in[1:]
		} else {
			if current_out_index+int(packs[mark].bytes_unpacked) > len(out) {
				return len(out)
			} else if int(packs[mark].bytes_packed) > len(in) {
				return SIZE_MAX
			}

			tmp := make([]byte, 4)
			for i := uint32(0); i < packs[mark].bytes_packed; i++ {
				tmp[i] = in[i]
			}

			word := binary.BigEndian.Uint32(tmp)

			offset = packs[mark].offsets[0]
			mask = uint32(packs[mark].masks[0])

			out[current_out_index] = chrs_by_chr_id[(word>>offset)&mask]
			last_chr = out[current_out_index]

			for i := uint32(1); i < packs[mark].bytes_unpacked; i++ {
				offset = packs[mark].offsets[i]
				mask = uint32(packs[mark].masks[i])

				chridx := uint8(last_chr) - uint8(MIN_CHR)
				idx := (word >> offset) & mask

				if last_chr >= MAX_CHR || last_chr < MIN_CHR || idx >= uint32(len(chrs_by_chr_and_successor_id[chridx])) {
					return 0
				}

				last_chr = byte(chrs_by_chr_and_successor_id[chridx][idx])
				out[current_out_index+int(i)] = last_chr
			}

			current_out_index += int(packs[mark].bytes_unpacked)
			in = in[packs[mark].bytes_packed:]
		}
	}

	return current_out_index
}

func Compress(text string) []byte {
	buf := make([]byte, len(text)*2)
	x := shoco_compress([]byte(text), buf)
	return buf[:x]
}

func Decompress(buf []byte) string {
	out := make([]byte, len(buf)*2)
	x := shoco_decompress(buf, out)

	if x > 0 {
		return string(out[:x])
	} else {
		return ""
	}
}
