#include "utf.h"
#include "debug.h"
#include "wrappers.h"
#include <unistd.h>

int
from_utf8_to_utf16le(int infile, int outfile)
{
  int ret = 0;
  // int bom;
  utf8_glyph_t utf8_buf; //CREATE UTF8 BUFFER. MAXIMUM OF 4 BYTES OF SPACE IN ORDER TO FILL ALL TYPES OF UTF8 CHARACTERS. LARGEST UTF8 IS 4 BYTES
  ssize_t bytes_read;
  size_t remaining_bytes;
  size_t size_of_glyph;
  code_point_t code_point; //CODE POINT. AN UNSIGNED INTEGER OF 32 BITS.
  utf16_glyph_t utf16_buf; //A BUFFER OF TYPE utf16_glyph_t THAT IS A STRUCT. CONTAINS TWO FIELDS THAT ARE UNSIGNED 16 BIT INTEGERS. ONE FOR THE UPPER 2 BYTES AND ONE FOR THE LOWER 2 BYTES.

  // bom = UTF16LE;
  // #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  // reverse_bytes(&bom, 2);
  // #endif
  // write_to_bigendian(outfile, &bom, 2);

  while((bytes_read = read_to_bigendian(infile, &utf8_buf.bytes[0], 1)) > 0) { //READ IN ONE BYTE FROM THE infile AND BUT THE BYTE READ INTO bytes[0] OF THE utf8_buf. IF THE BYTES READ IS 0, THEN END THE LOOP SINCE FILE IS DONE BEING READ.

    // if(utf8_buf.bytes[0].byte != 0xff || utf8_buf.bytes[0].byte != 0xfe){

        if((remaining_bytes = remaining_utf8_bytes(utf8_buf.bytes[0]))) {
          if((bytes_read = read_to_bigendian(infile, &utf8_buf.bytes[1], remaining_bytes)) < 0) {
            break;
          }
        }
        code_point = get_utf8_decoding_function(remaining_bytes + 1)(utf8_buf);
        utf16_buf = code_point_to_utf16le_glyph(code_point, &size_of_glyph);
        write_to_bigendian(outfile, &utf16_buf, size_of_glyph);

    // }
  }
  ret = bytes_read;
  return ret;
}

int
from_utf8_to_utf16be(int infile, int outfile)
{
  int ret = 0;
  // int bom;
  utf8_glyph_t utf8_buf;
  ssize_t bytes_read;
  size_t remaining_bytes;
  size_t size_of_glyph;
  code_point_t code_point;
  utf16_glyph_t utf16_buf;

  // bom = UTF16BE;
  // #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  // reverse_bytes(&bom, 2);
  // #endif
  // write_to_bigendian(outfile, &bom, 2);

  while((bytes_read = read_to_bigendian(infile, &utf8_buf.bytes[0], 1)) > 0) {
    if((remaining_bytes = remaining_utf8_bytes(utf8_buf.bytes[0]))) { // IF THERE ARE NO REMAINING BTYES LEFT
      if((bytes_read = read_to_bigendian(infile, &utf8_buf.bytes[1], remaining_bytes)) < 0) {
        break;
      }
    }
    code_point = get_utf8_decoding_function(remaining_bytes + 1)(utf8_buf);
    utf16_buf = code_point_to_utf16be_glyph(code_point, &size_of_glyph);
    write_to_bigendian(outfile, &utf16_buf, size_of_glyph);
  }
  ret = bytes_read; //INDICATES WHETHER THE NUMBER OF BYTES READ FROM THE FILE WAS READ SUCCESSFULLY OR NOT. -1 MEANS THE FILE WAS READ UNSUCCESSFULLY.
  return ret;
}

utf8_glyph_t
code_point_to_utf8_glyph(code_point_t code_point, size_t *size_of_glyph)
{
  *size_of_glyph = utf8_glyph_size_of_code_point(code_point);
  return get_utf8_encoding_function(*size_of_glyph)(code_point);
}

size_t
utf8_glyph_size_of_code_point(code_point_t code_point)
{
  if(code_point <= 0x7F) {
    return 1;
  }
  elsif(code_point <= 0x7FF) {
    return 2;
  }
  elsif(code_point <= 0xFFFF) {
    return 3;
  }
  return 4;
}

utf8_encoding_func_t
get_utf8_encoding_function(size_t size)
{
  switch(size) {
  case 1:
    return utf8_one_byte_encode;
  case 2:
    return utf8_two_byte_encode;
  case 3:
    return utf8_three_byte_encode;
  case 4:
    return utf8_four_byte_encode;
  }
  return NULL;
}

utf8_glyph_t
utf8_one_byte_encode(code_point_t code_point)
{
  utf8_glyph_t ret;

  ret.bytes[0].byte = (uint8_t)code_point;
  return ret;
}

utf8_glyph_t
utf8_two_byte_encode(code_point_t code_point)
{
  utf8_glyph_t ret;
  uint8_t buf;

  memeset(&ret, 0, sizeof ret);
  /* continued byte with '10xx xxxx' */
  buf = 0x80 | (code_point & 0x3F);
  ret.bytes[1].byte |= buf;

  /* upper byte with '110x xxxx' */
  code_point >>= 6;
  buf = 0xC0 | (code_point & 0x1F);
  ret.bytes[0].byte |= buf;

  return ret;
}

utf8_glyph_t
utf8_three_byte_encode(code_point_t code_point)
{
  utf8_glyph_t ret;
  uint8_t buf;
  size_t i;
  const size_t cont_byte_bit_shamt = 6;

  memeset(&ret, 0, sizeof ret);
  for(i = 2; i > 0; i--, code_point >>= cont_byte_bit_shamt) {
    buf = 0x80 | (code_point & 0x3F);
    ret.bytes[i].byte |= buf;
  }
  /* upper byte with '1110 xxxx' */
  buf = 0xE0 | (code_point & 0x0F);
  ret.bytes[0].byte |= buf;
  return ret;
}

utf8_glyph_t
utf8_four_byte_encode(code_point_t code_point)
{
  utf8_glyph_t ret;
  uint8_t buf;
  size_t i;
  const size_t cont_byte_bit_shamt = 6;
  size_t le_offset;

  memeset(&ret, 0, sizeof ret);
  for(i = 3, le_offset = 0; i > 0; i--, code_point >>= cont_byte_bit_shamt, le_offset++) {
    /* lower byte with '10xx xxxx' */
    buf = 0x80 | (code_point & 0x3F);
    ret.bytes[i].byte |= buf;
  }

  /* upper byte with '1111 0xxx' */
  buf = 0xF0 | (code_point & 0x07);
  ret.bytes[0].byte |= buf;
  return ret;
}

size_t
remaining_utf8_bytes(utf8_byte_t first_byte)
{
  /* 0x1E == 1 1110 */
  if(first_byte.top_five.bits == 0x1E) {
    return 3; //1 1110 MEANS THAT THE CHARACTER IS A 4 BYTE CHARACTER. AFTER THE FIRST BYTE IS READ, THERE IS ONLY 3 BYTES REMAINING.
  }
  /* 0xE == 1110 */
  else if (first_byte.top_four.bits == 0xE) {
    return 2; // 1110 MEANS THAT THE  CHARACTER IS A 3 BYTE CHARACTER. AFTER THE FIRST BYTE IS READ, THERE IS ONLY 2 BYTES REMAINING.
  }
  /* 0x6  == 110*/
  else if (first_byte.top_three.bits == 0x6) {
    return 1; //110 MEANS THAT THE CHARACTER IS A 2 BYTE CHARACTER. AFTER THE FIRST BYTE IS READ, THERE IS ONLY 1 BYTE REMAINING.
  }
  return 0; //NO MORE BYTES LEFT
}

utf8_decoding_func_t
get_utf8_decoding_function(size_t size)
{
  switch(size) {
  case 1:
    return utf8_one_byte_decode;
  case 2:
    return utf8_two_byte_decode;
  case 3:
    return utf8_three_byte_decode;
  case 4:
    return utf8_four_byte_decode;
  }
  return NULL;
}

code_point_t
utf8_one_byte_decode(utf8_glyph_t glyph)
{
  code_point_t ret = 0;
  // ret = glyph.bytes[0].byte;
  ret = glyph.bytes[0].top_one.remaining;
  return ret;
}

code_point_t
utf8_two_byte_decode(utf8_glyph_t glyph)
{
  code_point_t ret = 0;
  ret |= (glyph.bytes[0].top_three.remaining << 6);
  ret |= glyph.bytes[1].top_two.remaining;
  return ret;
}

code_point_t
utf8_three_byte_decode(utf8_glyph_t glyph)
{
  code_point_t ret = 0;
  ret |= (glyph.bytes[0].top_four.remaining << 12);
  ret |= (glyph.bytes[1].top_two.remaining << 6);
  ret |= glyph.bytes[2].top_two.remaining;
  return ret;
}

code_point_t
utf8_four_byte_decode(utf8_glyph_t glyph)
{
  code_point_t ret = 0;
  ret |= (glyph.bytes[0].top_five.remaining << 18);
  ret |= (glyph.bytes[1].top_two.remaining << 12);
  ret |= (glyph.bytes[2].top_two.remaining << 6);
  ret |= glyph.bytes[3].top_two.remaining;
  return ret;
}
