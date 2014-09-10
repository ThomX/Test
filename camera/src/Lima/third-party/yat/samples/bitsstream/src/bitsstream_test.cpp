/*!
 * \file     
 * \brief    A yat::BitsStream example.
 * \author   N. Leclercq - Synchrotron SOLEIL
 */


#include <iostream>
#include <limits>

#include <yat/bitsstream/BitsRecord.h>
#include <yat/memory/DataBuffer.h>

// ============================================================================
// BITS RECORD: TestRecord
// ============================================================================
YAT_BEGIN_BR(TestRecord)
  //- 1 bit members
  MEMBER(bool_0, 1, bool)
  MEMBER(bool_1, 1, bool)
  MEMBER(bool_2, 1, bool)
  MEMBER(bool_3, 1, bool)
  MEMBER(bool_4, 1, bool)
  MEMBER(bool_5, 1, bool)
  MEMBER(bool_6, 1, bool)
  MEMBER(bool_7, 1, bool)
  //- 1 byte members
  MEMBER(neg_signed_char, sizeof(char) * 8, char)
  MEMBER(mid_signed_char, sizeof(char) * 8, char)
  MEMBER(pos_signed_char, sizeof(char) * 8, char)
  MEMBER(min_unsigned_char, sizeof(unsigned char) * 8, unsigned char)
  MEMBER(mid_unsigned_char, sizeof(unsigned char) * 8, unsigned char)
  MEMBER(max_unsigned_char, sizeof(unsigned char) * 8, unsigned char)
  //- 2 bytes members
  MEMBER(neg_signed_short, sizeof(short) * 8, short)
  MEMBER(mid_signed_short, sizeof(short) * 8, short)
  MEMBER(pos_signed_short, sizeof(short) * 8, short)
  MEMBER(min_unsigned_short, sizeof(unsigned short) * 8, unsigned short)
  MEMBER(mid_unsigned_short, sizeof(unsigned short) * 8, unsigned short)
  MEMBER(max_unsigned_short, sizeof(unsigned short) * 8, unsigned short)
  //- 4 bytes members
  MEMBER(neg_signed_int, sizeof(int) * 8, int)
  MEMBER(mid_signed_int, sizeof(int) * 8, int)
  MEMBER(pos_signed_int, sizeof(int) * 8, int)
  MEMBER(min_unsigned_int, sizeof(unsigned int) * 8, unsigned int)
  MEMBER(mid_unsigned_int, sizeof(unsigned int) * 8, unsigned int)
  MEMBER(max_unsigned_int, sizeof(unsigned int) * 8, unsigned int)
  //- 4 or 8 bytes members (sizeof(long) is 4 on Windows-64 and 8 on Linux/MacOSX-64)
  MEMBER(neg_signed_long, sizeof(long) * 8, long)
  MEMBER(mid_signed_long, sizeof(long) * 8, long)
  MEMBER(pos_signed_long, sizeof(long) * 8, long)
  MEMBER(min_unsigned_long, sizeof(unsigned long) * 8, unsigned long)
  MEMBER(mid_unsigned_long, sizeof(unsigned long) * 8, unsigned long)
  MEMBER(max_unsigned_long, sizeof(unsigned long) * 8, unsigned long)
#if defined(YAT_64BITS) && defined(WIN64)
  //- 8 bytes members
  MEMBER(neg_signed_int64, sizeof(__int64) * 8, __int64)
  MEMBER(mid_signed_int64, sizeof(__int64) * 8, __int64)
  MEMBER(pos_signed_int64, sizeof(__int64) * 8, __int64)
  MEMBER(min_unsigned_int64, sizeof(unsigned __int64) * 8, unsigned __int64)
  MEMBER(mid_unsigned_int64, sizeof(unsigned __int64) * 8, unsigned __int64)
  MEMBER(max_unsigned_int64, sizeof(unsigned __int64) * 8, unsigned __int64)
#endif
YAT_END_BR(TestRecord)
//-----------------------------------------------------------------------------
YAT_BEGIN_BR_EXTRACTOR(TestRecord)
  EXTRACT_MEMBER(bool_0)
  EXTRACT_MEMBER(bool_1)
  EXTRACT_MEMBER(bool_2)
  EXTRACT_MEMBER(bool_3)
  EXTRACT_MEMBER(bool_4)
  EXTRACT_MEMBER(bool_5)
  EXTRACT_MEMBER(bool_6)
  EXTRACT_MEMBER(bool_7)
  EXTRACT_MEMBER(neg_signed_char)
  EXTRACT_MEMBER(mid_signed_char)
  EXTRACT_MEMBER(pos_signed_char)
  EXTRACT_MEMBER(min_unsigned_char)
  EXTRACT_MEMBER(mid_unsigned_char)
  EXTRACT_MEMBER(max_unsigned_char)
  EXTRACT_MEMBER(neg_signed_short)
  EXTRACT_MEMBER(mid_signed_short)
  EXTRACT_MEMBER(pos_signed_short)
  EXTRACT_MEMBER(min_unsigned_short)
  EXTRACT_MEMBER(mid_unsigned_short)
  EXTRACT_MEMBER(max_unsigned_short)
  EXTRACT_MEMBER(neg_signed_int)
  EXTRACT_MEMBER(mid_signed_int)
  EXTRACT_MEMBER(pos_signed_int)
  EXTRACT_MEMBER(min_unsigned_int)
  EXTRACT_MEMBER(mid_unsigned_int)
  EXTRACT_MEMBER(max_unsigned_int)
  EXTRACT_MEMBER(neg_signed_long)
  EXTRACT_MEMBER(mid_signed_long)
  EXTRACT_MEMBER(pos_signed_long)
  EXTRACT_MEMBER(min_unsigned_long)
  EXTRACT_MEMBER(mid_unsigned_long)
  EXTRACT_MEMBER(max_unsigned_long)
#if defined(YAT_64BITS) && defined(WIN64)
  EXTRACT_MEMBER(neg_signed_int64)
  EXTRACT_MEMBER(mid_signed_int64)
  EXTRACT_MEMBER(pos_signed_int64)
  EXTRACT_MEMBER(min_unsigned_int64)
  EXTRACT_MEMBER(mid_unsigned_int64)
  EXTRACT_MEMBER(max_unsigned_int64)
#endif
YAT_END_BR_EXTRACTOR(TestRecord)
//-----------------------------------------------------------------------------
YAT_BEGIN_BR_DUMP(TestRecord)
  DUMP_MEMBER(bool_0)
  DUMP_MEMBER(bool_1)
  DUMP_MEMBER(bool_2)
  DUMP_MEMBER(bool_3)
  DUMP_MEMBER(bool_4)
  DUMP_MEMBER(bool_5)
  DUMP_MEMBER(bool_6)
  DUMP_MEMBER(bool_7)
  DUMP_MEMBER(neg_signed_char)
  DUMP_MEMBER(mid_signed_char)
  DUMP_MEMBER(pos_signed_char)
  DUMP_MEMBER(min_unsigned_char)
  DUMP_MEMBER(mid_unsigned_char)
  DUMP_MEMBER(max_unsigned_char)
  DUMP_MEMBER(neg_signed_short)
  DUMP_MEMBER(mid_signed_short)
  DUMP_MEMBER(pos_signed_short)
  DUMP_MEMBER(min_unsigned_short)
  DUMP_MEMBER(mid_unsigned_short)
  DUMP_MEMBER(max_unsigned_short)
  DUMP_MEMBER(neg_signed_int)
  DUMP_MEMBER(mid_signed_int)
  DUMP_MEMBER(pos_signed_int)
  DUMP_MEMBER(min_unsigned_int)
  DUMP_MEMBER(mid_unsigned_int)
  DUMP_MEMBER(max_unsigned_int)
  DUMP_MEMBER(neg_signed_long)
  DUMP_MEMBER(mid_signed_long)
  DUMP_MEMBER(pos_signed_long)
  DUMP_MEMBER(min_unsigned_long)
  DUMP_MEMBER(mid_unsigned_long)
  DUMP_MEMBER(max_unsigned_long)
#if defined(YAT_64BITS) && defined(WIN64)
  DUMP_MEMBER(neg_signed_int64)
  DUMP_MEMBER(mid_signed_int64)
  DUMP_MEMBER(pos_signed_int64)
  DUMP_MEMBER(min_unsigned_int64)
  DUMP_MEMBER(mid_unsigned_int64)
  DUMP_MEMBER(max_unsigned_int64)
#endif
YAT_END_BR_DUMP(TestRecord)
// ============================================================================
// BITS RECORD: 32 BITS STATUS WORD
// ============================================================================
YAT_BEGIN_BR(StatusWord32To32Bool)
  MEMBER(bit_00, 1, bool);
  MEMBER(bit_01, 1, bool);
  MEMBER(bit_02, 1, bool);
  MEMBER(bit_03, 1, bool);
  MEMBER(bit_04, 1, bool);
  MEMBER(bit_05, 1, bool); 
  MEMBER(bit_06, 1, bool);
  MEMBER(bit_07, 1, bool);
  MEMBER(bit_08, 1, bool); 
  MEMBER(bit_09, 1, bool); 
  MEMBER(bit_10, 1, bool); 
  MEMBER(bit_11, 1, bool); 
  MEMBER(bit_12, 1, bool);
  MEMBER(bit_13, 1, bool);
  MEMBER(bit_14, 1, bool);
  MEMBER(bit_15, 1, bool);
  MEMBER(bit_16, 1, bool);
  MEMBER(bit_17, 1, bool);
  MEMBER(bit_18, 1, bool);
  MEMBER(bit_19, 1, bool);
  MEMBER(bit_20, 1, bool);
  MEMBER(bit_21, 1, bool);  
  MEMBER(bit_22, 1, bool);
  MEMBER(bit_23, 1, bool);
  MEMBER(bit_24, 1, bool);
  MEMBER(bit_25, 1, bool);
  MEMBER(bit_26, 1, bool);  
  MEMBER(bit_27, 1, bool);
  MEMBER(bit_28, 1, bool);
  MEMBER(bit_29, 1, bool); 
  MEMBER(bit_30, 1, bool); 
  MEMBER(bit_31, 1, bool); 
YAT_END_BR(StatusWord32To32Bool)
//-----------------------------------------------------------------------------
YAT_BEGIN_BR_EXTRACTOR(StatusWord32To32Bool)
  EXTRACT_MEMBER(bit_00);
  EXTRACT_MEMBER(bit_01);
  EXTRACT_MEMBER(bit_02);
  EXTRACT_MEMBER(bit_03);
  EXTRACT_MEMBER(bit_04);
  EXTRACT_MEMBER(bit_05);  
  EXTRACT_MEMBER(bit_06);
  EXTRACT_MEMBER(bit_07);
  EXTRACT_MEMBER(bit_08); 
  EXTRACT_MEMBER(bit_09); 
  EXTRACT_MEMBER(bit_10); 
  EXTRACT_MEMBER(bit_11); 
  EXTRACT_MEMBER(bit_12);
  EXTRACT_MEMBER(bit_13);
  EXTRACT_MEMBER(bit_14);
  EXTRACT_MEMBER(bit_15);
  EXTRACT_MEMBER(bit_16);
  EXTRACT_MEMBER(bit_17);
  EXTRACT_MEMBER(bit_18);
  EXTRACT_MEMBER(bit_19);
  EXTRACT_MEMBER(bit_20);
  EXTRACT_MEMBER(bit_21);
  EXTRACT_MEMBER(bit_22);
  EXTRACT_MEMBER(bit_23);
  EXTRACT_MEMBER(bit_24);
  EXTRACT_MEMBER(bit_25);
  EXTRACT_MEMBER(bit_26);  
  EXTRACT_MEMBER(bit_27);
  EXTRACT_MEMBER(bit_28);
  EXTRACT_MEMBER(bit_29); 
  EXTRACT_MEMBER(bit_30); 
  EXTRACT_MEMBER(bit_31); 
YAT_END_BR_EXTRACTOR(StatusWord32To32Bool)
//-----------------------------------------------------------------------------
YAT_BEGIN_BR_DUMP(StatusWord32To32Bool)
  DUMP_MEMBER(bit_00);
  DUMP_MEMBER(bit_01);
  DUMP_MEMBER(bit_02);
  DUMP_MEMBER(bit_03);
  DUMP_MEMBER(bit_04);
  DUMP_MEMBER(bit_05); 
  DUMP_MEMBER(bit_06);
  DUMP_MEMBER(bit_07);
  DUMP_MEMBER(bit_08); 
  DUMP_MEMBER(bit_09); 
  DUMP_MEMBER(bit_10); 
  DUMP_MEMBER(bit_11); 
  DUMP_MEMBER(bit_13);
  DUMP_MEMBER(bit_14);
  DUMP_MEMBER(bit_15);
  DUMP_MEMBER(bit_16);
  DUMP_MEMBER(bit_17);
  DUMP_MEMBER(bit_18);
  DUMP_MEMBER(bit_19);
  DUMP_MEMBER(bit_20);
  DUMP_MEMBER(bit_21);
  DUMP_MEMBER(bit_22);
  DUMP_MEMBER(bit_23);
  DUMP_MEMBER(bit_24);
  DUMP_MEMBER(bit_25);
  DUMP_MEMBER(bit_26);  
  DUMP_MEMBER(bit_27);
  DUMP_MEMBER(bit_28);
  DUMP_MEMBER(bit_29); 
  DUMP_MEMBER(bit_30); 
  DUMP_MEMBER(bit_31); 
YAT_END_BR_DUMP(StatusWord32To32Bool)
// ============================================================================
// BITS RECORD: 32 BITS STATUS WORD
// ============================================================================
YAT_BEGIN_BR(StatusWord32To04Bytes)
  MEMBER(byte_00, 8, unsigned char);
  MEMBER(byte_01, 8, unsigned char);
  MEMBER(byte_02, 8, unsigned char);
  MEMBER(byte_03, 8, unsigned char);
YAT_END_BR(StatusWord32To04Bytes)
//-----------------------------------------------------------------------------
YAT_BEGIN_BR_EXTRACTOR(StatusWord32To04Bytes)
  EXTRACT_MEMBER(byte_00);
  EXTRACT_MEMBER(byte_01);
  EXTRACT_MEMBER(byte_02);
  EXTRACT_MEMBER(byte_03);
YAT_END_BR_EXTRACTOR(StatusWord32To04Bytes)
//-----------------------------------------------------------------------------
YAT_BEGIN_BR_DUMP(StatusWord32To04Bytes)
  DUMP_MEMBER(byte_00);
  DUMP_MEMBER(byte_01);
  DUMP_MEMBER(byte_02);
  DUMP_MEMBER(byte_03);
YAT_END_BR_DUMP(StatusWord32To04Bytes)
// ============================================================================
// BITS RECORD: 16 BITS STATUS WORD
// ============================================================================
YAT_BEGIN_BR(StatusWord16To16Bool)
  MEMBER(bit_00, 1, bool);
  MEMBER(bit_01, 1, bool);
  MEMBER(bit_02, 1, bool);
  MEMBER(bit_03, 1, bool);
  MEMBER(bit_04, 1, bool);
  MEMBER(bit_05, 1, bool); 
  MEMBER(bit_06, 1, bool);
  MEMBER(bit_07, 1, bool);
  MEMBER(bit_08, 1, bool); 
  MEMBER(bit_09, 1, bool); 
  MEMBER(bit_10, 1, bool); 
  MEMBER(bit_11, 1, bool); 
  MEMBER(bit_12, 1, bool);
  MEMBER(bit_13, 1, bool);
  MEMBER(bit_14, 1, bool);
  MEMBER(bit_15, 1, bool);
YAT_END_BR(StatusWord16To16Bool)
//-----------------------------------------------------------------------------
YAT_BEGIN_BR_EXTRACTOR(StatusWord16To16Bool)
  EXTRACT_MEMBER(bit_00);
  EXTRACT_MEMBER(bit_01);
  EXTRACT_MEMBER(bit_02);
  EXTRACT_MEMBER(bit_03);
  EXTRACT_MEMBER(bit_04);
  EXTRACT_MEMBER(bit_05);  
  EXTRACT_MEMBER(bit_06);
  EXTRACT_MEMBER(bit_07);
  EXTRACT_MEMBER(bit_08); 
  EXTRACT_MEMBER(bit_09); 
  EXTRACT_MEMBER(bit_10); 
  EXTRACT_MEMBER(bit_11); 
  EXTRACT_MEMBER(bit_12);
  EXTRACT_MEMBER(bit_13);
  EXTRACT_MEMBER(bit_14);
  EXTRACT_MEMBER(bit_15);
YAT_END_BR_EXTRACTOR(StatusWord16To16Bool)
//-----------------------------------------------------------------------------
YAT_BEGIN_BR_DUMP(StatusWord16To16Bool)
  DUMP_MEMBER(bit_00);
  DUMP_MEMBER(bit_01);
  DUMP_MEMBER(bit_02);
  DUMP_MEMBER(bit_03);
  DUMP_MEMBER(bit_04);
  DUMP_MEMBER(bit_05); 
  DUMP_MEMBER(bit_06);
  DUMP_MEMBER(bit_07);
  DUMP_MEMBER(bit_08); 
  DUMP_MEMBER(bit_09); 
  DUMP_MEMBER(bit_10); 
  DUMP_MEMBER(bit_11); 
  DUMP_MEMBER(bit_12);
  DUMP_MEMBER(bit_13);
  DUMP_MEMBER(bit_14);
  DUMP_MEMBER(bit_15);
YAT_END_BR_DUMP(StatusWord16To16Bool)
// ============================================================================
// BITS RECORD: 16 BITS STATUS WORD
// ============================================================================
YAT_BEGIN_BR(StatusWord16To02Bytes)
  MEMBER(m1, 8, unsigned char);
  MEMBER(m2, 8, unsigned char);
YAT_END_BR(StatusWord16To02Bytes)
//-----------------------------------------------------------------------------
YAT_BEGIN_BR_EXTRACTOR(StatusWord16To02Bytes)
  EXTRACT_MEMBER(m1);
  EXTRACT_MEMBER(m2);
YAT_END_BR_EXTRACTOR(StatusWord16To02Bytes)
//-----------------------------------------------------------------------------
YAT_BEGIN_BR_DUMP(StatusWord16To02Bytes)
  DUMP_MEMBER(m1);
  DUMP_MEMBER(m2);
YAT_END_BR_DUMP(StatusWord16To02Bytes)
// ============================================================================
// BITS RECORD: 16 BITS STATUS WORD
// ============================================================================
YAT_BEGIN_BR(StatusWord32ToNbits)
  MEMBER(m1, 1, bool);
  MEMBER(m2, 2, unsigned short);
  MEMBER(m3, 3, unsigned short);
  MEMBER(m4, 4, unsigned short);
  MEMBER(m5, 5, unsigned short);
  MEMBER(m6, 6, unsigned short); 
  MEMBER(m7, 7, unsigned short);
  IGNORE_MEMBER(none, 1, none);
YAT_END_BR(StatusWord32ToNbits)
//-----------------------------------------------------------------------------
YAT_BEGIN_BR_EXTRACTOR(StatusWord32ToNbits)
  EXTRACT_MEMBER(m1);
  EXTRACT_MEMBER(m2);
  EXTRACT_MEMBER(m3);
  EXTRACT_MEMBER(m4);
  EXTRACT_MEMBER(m5);
  EXTRACT_MEMBER(m6);  
  EXTRACT_MEMBER(m7);
  SKIP_BITS(1);
YAT_END_BR_EXTRACTOR(StatusWord32ToNbits)
//-----------------------------------------------------------------------------
YAT_BEGIN_BR_DUMP(StatusWord32ToNbits)
  DUMP_MEMBER(m1);
  DUMP_MEMBER(m2);
  DUMP_MEMBER(m3);
  DUMP_MEMBER(m4);
  DUMP_MEMBER(m5);
  DUMP_MEMBER(m6); 
  DUMP_MEMBER(m7);
  DUMP_SKIP_BITS(1);
YAT_END_BR_DUMP(StatusWord32ToNbits)

//-----------------------------------------------------------------------------
// forward declaration
//-----------------------------------------------------------------------------
void fill_buffers (yat::Buffer<unsigned char>& leb, yat::Buffer<unsigned char>& beb);

//-----------------------------------------------------------------------------
// MAIN
//-----------------------------------------------------------------------------
int main (int, char**)
{
  std::cout << "------------------------------------" << std::endl; 
  std::cout << "size of bool...." << sizeof(bool)    * 8 << " bits" << std::endl;
  std::cout << "size of char...." << sizeof(char)    * 8 << " bits" << std::endl;
  std::cout << "size of short..." << sizeof(short)   * 8 << " bits" << std::endl;
  std::cout << "size of int....." << sizeof(int)     * 8 << " bits" << std::endl;
  std::cout << "size of long...." << sizeof(long)    * 8 << " bits" << std::endl;
  std::cout << "------------------------------------" << std::endl;

  try
  {
    //--------------------------------------------------------------------
    // HARDWARE DATA READING SIMULATION
    //--------------------------------------------------------------------
    //- big endian buffer
    yat::Buffer<unsigned char> big_endian_buffer;
    //- little endian buffer
    yat::Buffer<unsigned char> little_endian_buffer;
    //- each buffer will be filled according to its byte ordering
    //- this will allow to test the yat::BitsStream byte reordering
    fill_buffers(little_endian_buffer, big_endian_buffer);
    //--------------------------------------------------------------------
    // LITTLE ENDIAN TEST
    //--------------------------------------------------------------------
    //- instanciate the little endian BitsStream from data buffer
    yat::BitsStream little_endian_bs(little_endian_buffer.base(), 
                                     little_endian_buffer.size(), 
                                     yat::Endianness::BO_LITTLE_ENDIAN);
    //- push BitsStream content into a TestRecord then dump the result 
    TestRecord little_endian_tr;
    little_endian_bs >> little_endian_tr;
    std::cout << little_endian_tr << std::endl;
    //--------------------------------------------------------------------
    // BIG ENDIAN TEST
    //--------------------------------------------------------------------
    //- instanciate the big endian BitsStream from data buffer
    yat::BitsStream big_endian_bs(big_endian_buffer.base(),
                                  big_endian_buffer.size(),
                                  yat::Endianness::BO_BIG_ENDIAN);
    //- push BitsStream content into a TestRecord then dump the result 
    TestRecord big_endian_tr;
    big_endian_bs >> big_endian_tr;
    std::cout << big_endian_tr << std::endl;
    //--------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------
    unsigned long ui32 = 0xAA00FF55;
    yat::BitsStream ui32_bs(reinterpret_cast<unsigned char *>(&ui32),
                            sizeof(unsigned long));
    StatusWord32To32Bool ui32_tr;
    ui32_bs >> ui32_tr;
    std::cout << ui32_tr << std::endl;
    //--------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------
    yat::BitsStream ui32_bs2(reinterpret_cast<unsigned char *>(&ui32),
                             sizeof(unsigned long));
    StatusWord32To04Bytes ui32_tr2;
    ui32_bs2 >> ui32_tr2;
    std::cout << ui32_tr2 << std::endl;
    //--------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------
    unsigned short ui16 = 0xAA55;
    yat::BitsStream ui16_bs(reinterpret_cast<unsigned char *>(&ui16),
                            sizeof(unsigned short));
    StatusWord16To16Bool ui16_tr;
    ui16_bs >> ui16_tr;
    std::cout << ui16_tr << std::endl;
    //--------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------
    yat::BitsStream ui16_bs2(reinterpret_cast<unsigned char *>(&ui16),
                             sizeof(unsigned short));
    StatusWord16To02Bytes ui16_tr2;
    ui16_bs2 >> ui16_tr2;
    std::cout << ui16_tr2 << std::endl;
    //--------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------
    yat::uint32 ui32_3 = 0xFFFFFFFF;
    yat::BitsStream ui32_bs3(reinterpret_cast<unsigned char *>(&ui32_3),
                             sizeof(yat::uint32));
    StatusWord32ToNbits ui32_tr3;
    ui32_bs3 >> ui32_tr3;
    std::cout << ui32_tr3 << std::endl;
  }
  catch (...)
  {
    std::cout << "Unknown exception caught" << std::endl;
  }

	return 0;  
}

//-----------------------------------------------------------------------------
// dump_val
//-----------------------------------------------------------------------------
template <typename _T> void dump_val (const char * _txt, const _T& _v)
{
  std::bitset<8 * sizeof(_T)> _v_bs(static_cast<yat::BitsStorage>(_v));
  std::cout << _txt 
            << _v
            << " ["
            << std::hex
            << _v
            << std::dec
            << "] ["
            << _v_bs.to_string()
            << "]"
            << std::endl;
  std::cout << "------------------------------------" << std::endl;
}

//-----------------------------------------------------------------------------
// fill_buffers
//-----------------------------------------------------------------------------
void fill_buffers (yat::Buffer<unsigned char>& leb, yat::Buffer<unsigned char>& beb)
{
  const size_t num_val_per_type = 3;
 
  const unsigned char bools(0xAA);

  char chars[num_val_per_type] = 
  { 
    std::numeric_limits<char>::min(), 
    std::numeric_limits<char>::max() / 2, 
    std::numeric_limits<char>::max() 
  }; 

  unsigned char uchars[num_val_per_type] = 
  { 
    std::numeric_limits<unsigned char>::min(),
    std::numeric_limits<unsigned char>::max() / 2,
    std::numeric_limits<unsigned char>::max()
  };

  short shorts[num_val_per_type] = 
  { 
    std::numeric_limits<short>::min(), 
    std::numeric_limits<short>::max() / 2,
    std::numeric_limits<short>::max()
  }; 

  unsigned short ushorts[num_val_per_type] = 
  { 
    std::numeric_limits<unsigned short>::min(),
    std::numeric_limits<unsigned short>::max() / 2,
    std::numeric_limits<unsigned short>::max()
  }; 

  int ints[num_val_per_type] = 
  { 
    std::numeric_limits<int>::min(), 
    std::numeric_limits<int>::max() / 2,
    std::numeric_limits<int>::max()
  }; 
  
  unsigned int uints[num_val_per_type] = 
  { 
    std::numeric_limits<unsigned int>::min(),
    std::numeric_limits<unsigned int>::max() / 2,
    std::numeric_limits<unsigned int>::max()
  }; 
  
  long longs[num_val_per_type] = 
  { 
    std::numeric_limits<long>::min(), 
    std::numeric_limits<long>::max() / 2,
    std::numeric_limits<long>::max()
  }; 

  unsigned long ulongs[num_val_per_type] = 
  { 
    std::numeric_limits<unsigned long>::min(), 
    std::numeric_limits<unsigned long>::max() / 2,
    std::numeric_limits<unsigned long>::max()
  }; 

#if defined(YAT_64BITS) && defined(WIN64)
  __int64 int64s[num_val_per_type] = 
  { 
    std::numeric_limits<__int64>::min(), 
    std::numeric_limits<__int64>::max() / 2,
    std::numeric_limits<__int64>::max()
  }; 

  for (size_t i =0; i < num_val_per_type; i++)
    std::cout << int64s[i] << std::endl;

  unsigned __int64 uint64s[num_val_per_type] = 
  { 
    std::numeric_limits<unsigned __int64>::min(), 
    std::numeric_limits<unsigned __int64>::max() / 2,
    std::numeric_limits<unsigned __int64>::max()
  }; 

  for (size_t i =0; i < num_val_per_type; i++)
    std::cout << uint64s[i] << std::endl;
#endif

 size_t buffer_size = sizeof(bools)
                    + sizeof(chars)
                    + sizeof(uchars)
                    + sizeof(shorts)
                    + sizeof(ushorts)
                    + sizeof(ints)
                    + sizeof(uints)
                    + sizeof(longs)
                    + sizeof(ulongs);

#if defined(YAT_64BITS) && defined(WIN64)
  buffer_size += sizeof(int64s) + sizeof(uint64s);
#endif

  leb.capacity(buffer_size);
  leb.force_length(buffer_size);
  
  beb.capacity(buffer_size);
  beb.force_length(buffer_size);

  size_t offset = 0;

  ::memcpy(leb.base() + offset, &bools, sizeof(bools));
  
  dump_val("leb[bools]= ", static_cast<int>(bools));

  ::memcpy(beb.base() + offset, &bools, sizeof(bools));
  
  dump_val("beb[bools]= ", static_cast<int>(bools));

  offset += sizeof(bools);

  ::memcpy(leb.base() + offset, chars, sizeof(chars));
  
  char * cp = reinterpret_cast<char*>(leb.base() + offset);
  
  dump_val("leb[char:0]= ", static_cast<char>(*(cp + 0)));
  dump_val("leb[char:1]= ", static_cast<char>(*(cp + 1)));
  dump_val("leb[char:2]= ", static_cast<char>(*(cp + 2)));

  ::memcpy(beb.base() + offset, chars, sizeof(chars));
  
  cp = reinterpret_cast<char*>(beb.base() + offset);
  
  dump_val("beb[char:0]= ", static_cast<char>(*(cp + 0)));
  dump_val("beb[char:1]= ", static_cast<char>(*(cp + 1)));
  dump_val("beb[char:2]= ", static_cast<char>(*(cp + 2)));

  offset += sizeof(chars);

  ::memcpy(leb.base() + offset, uchars, sizeof(uchars));
  
  unsigned char * ucp = reinterpret_cast<unsigned char*>(leb.base() + offset);
  
  dump_val("leb[uchar:0]= ", static_cast<unsigned char>(*(ucp + 0)));
  dump_val("leb[uchar:1]= ", static_cast<unsigned char>(*(ucp + 1)));
  dump_val("leb[uchar:2]= ", static_cast<unsigned char>(*(ucp + 2)));

  ::memcpy(beb.base() + offset, uchars, sizeof(uchars));
  
  ucp = reinterpret_cast<unsigned char*>(beb.base() + offset);
  
  dump_val("beb[char:0]= ", static_cast<unsigned char>(*(ucp + 0)));
  dump_val("beb[char:1]= ", static_cast<unsigned char>(*(ucp + 1)));
  dump_val("beb[char:2]= ", static_cast<unsigned char>(*(ucp + 2)));

  offset += sizeof(uchars);

  ::memcpy(leb.base() + offset, shorts, sizeof(shorts));
  
  short * sp = reinterpret_cast<short*>(leb.base() + offset);
  
  dump_val("leb[shorts:0]= ", *(sp + 0));
  dump_val("leb[shorts:1]= ", *(sp + 1));
  dump_val("leb[shorts:2]= ", *(sp + 2));

  ::memcpy(beb.base() + offset, shorts, sizeof(shorts));
  
  sp = reinterpret_cast<short*>(beb.base() + offset);
  
  yat::Endianness::swap_2_array(reinterpret_cast<char*>(sp), 
                                reinterpret_cast<char*>(sp), 
                                num_val_per_type);
  
  dump_val("beb[shorts:0]= ", *(sp + 0));
  dump_val("beb[shorts:1]= ", *(sp + 1));
  dump_val("beb[shorts:1]= ", *(sp + 2));

  offset += sizeof(shorts);

  ::memcpy(leb.base() + offset, ushorts, sizeof(ushorts));
  
  unsigned short * usp = reinterpret_cast<unsigned short*>(leb.base() + offset);
  
  dump_val("leb[unsigned shorts:0]= ", *(usp + 0));
  dump_val("leb[unsigned shorts:1]= ", *(usp + 1));
  dump_val("leb[unsigned shorts:2]= ", *(usp + 2));

  ::memcpy(beb.base() + offset, ushorts, sizeof(ushorts));
  
  usp = reinterpret_cast<unsigned short*>(beb.base() + offset);
  
  yat::Endianness::swap_2_array(reinterpret_cast<char*>(usp), 
                                reinterpret_cast<char*>(usp), 
                                num_val_per_type);
  
  dump_val("beb[unsigned shorts:0]= ", *(usp + 0));
  dump_val("beb[unsigned shorts:1]= ", *(usp + 1));
  dump_val("beb[unsigned shorts:2]= ", *(usp + 2));

  offset += sizeof(ushorts);

  ::memcpy(leb.base() + offset, ints, sizeof(ints));
  
  int * ip = reinterpret_cast<int*>(leb.base() + offset);
  
  dump_val("leb[int:0]= ", *(ip + 0));
  dump_val("leb[int:1]= ", *(ip + 1));
  dump_val("leb[int:2]= ", *(ip + 2));
  
  ::memcpy(beb.base() + offset, ints, sizeof(ints));
  
  ip = reinterpret_cast<int*>(beb.base() + offset);
  
  if (sizeof(int) == 4)
    yat::Endianness::swap_4_array(reinterpret_cast<char*>(ip), 
                                  reinterpret_cast<char*>(ip), 
                                  num_val_per_type);
  else
    yat::Endianness::swap_2_array(reinterpret_cast<char*>(ip), 
                                  reinterpret_cast<char*>(ip), 
                                  num_val_per_type);
  
  dump_val("beb[int:0]= ", *(ip + 0));
  dump_val("beb[int:1]= ", *(ip + 1));
  dump_val("beb[int:2]= ", *(ip + 2));
  
  offset += sizeof(ints);
  
  ::memcpy(leb.base() + offset, uints, sizeof(uints));
  
  unsigned int * uip = reinterpret_cast<unsigned int*>(leb.base() + offset);
  
  dump_val("leb[unsigned int:0]= ", *(uip + 0));
  dump_val("leb[unsigned int:1]= ", *(uip + 1));
  dump_val("leb[unsigned int:2]= ", *(uip + 2));
  
  ::memcpy(beb.base() + offset, uints, sizeof(uints));
  
  uip = reinterpret_cast<unsigned int*>(beb.base() + offset);
  
  if (sizeof(unsigned int) == 4)
    yat::Endianness::swap_4_array(reinterpret_cast<char*>(uip), 
                                  reinterpret_cast<char*>(uip), 
                                  num_val_per_type);
  else
    yat::Endianness::swap_2_array(reinterpret_cast<char*>(uip), 
                                  reinterpret_cast<char*>(uip), 
                                  num_val_per_type);
  
  dump_val("beb[unsigned int:0]= ", *(uip + 0));
  dump_val("beb[unsigned int:1]= ", *(uip + 1));
  dump_val("beb[unsigned int:2]= ", *(uip + 2));
  
  offset += sizeof(uints);
  
  ::memcpy(leb.base() + offset, longs, sizeof(longs));
  
  long * lp = reinterpret_cast<long*>(leb.base() + offset);
  
  dump_val("leb[long:0]= ", *(lp + 0));
  dump_val("leb[long:1]= ", *(lp + 1));
  dump_val("leb[long:2]= ", *(lp + 2));

  ::memcpy(beb.base() + offset, longs, sizeof(longs));
  
  lp = reinterpret_cast<long*>(beb.base() + offset);
  
  if (sizeof(long) == 8)
    yat::Endianness::swap_8_array(reinterpret_cast<char*>(lp), 
                                  reinterpret_cast<char*>(lp), 
                                  num_val_per_type);
  else
    yat::Endianness::swap_4_array(reinterpret_cast<char*>(lp), 
                                  reinterpret_cast<char*>(lp), 
                                  num_val_per_type);
  
  dump_val("beb[long:0]= ", *(lp + 0));
  dump_val("beb[long:1]= ", *(lp + 1));
  dump_val("beb[long:2]= ", *(lp + 2));
  
  offset += num_val_per_type * sizeof(long);

  ::memcpy(leb.base() + offset, ulongs, sizeof(ulongs));
  
  unsigned long * ulp = reinterpret_cast<unsigned long*>(leb.base() + offset);
  
  dump_val("leb[unsigned long:0]= ", *(ulp + 0));
  dump_val("leb[unsigned long:1]= ", *(ulp + 1));
  dump_val("leb[unsigned long:2]= ", *(ulp + 2));

  ::memcpy(beb.base() + offset, ulongs, sizeof(ulongs));
  
  ulp = reinterpret_cast<unsigned long*>(beb.base() + offset);
  
  if (sizeof(unsigned long) == 8)
    yat::Endianness::swap_8_array(reinterpret_cast<char*>(ulp), 
                                  reinterpret_cast<char*>(ulp), 
                                  num_val_per_type);
  else
    yat::Endianness::swap_4_array(reinterpret_cast<char*>(ulp), 
                                  reinterpret_cast<char*>(ulp), 
                                  num_val_per_type);
  
  dump_val("beb[unsigned long:0]= ", *(ulp + 0));
  dump_val("beb[unsigned long:1]= ", *(ulp + 1));
  dump_val("beb[unsigned long:2]= ", *(ulp + 2));

#if defined(YAT_64BITS) && defined(WIN64)
  offset += num_val_per_type * sizeof(unsigned long);

  ::memcpy(leb.base() + offset, int64s, sizeof(int64s));
  
  __int64 * i64p = reinterpret_cast<__int64*>(leb.base() + offset);

  dump_val("leb[__int64:0]= ", *(i64p + 0));
  dump_val("leb[__int64:1]= ", *(i64p + 1));
  dump_val("leb[__int64:2]= ", *(i64p + 2));

  ::memcpy(beb.base() + offset, int64s, sizeof(int64s));

  i64p = reinterpret_cast<__int64*>(beb.base() + offset);

  yat::Endianness::swap_8_array(reinterpret_cast<char*>(i64p), 
                                reinterpret_cast<char*>(i64p), 
                                num_val_per_type);

  dump_val("beb[__int64:0]= ", *(i64p + 0));
  dump_val("beb[__int64:1]= ", *(i64p + 1));
  dump_val("beb[__int64:2]= ", *(i64p + 2));

  offset += num_val_per_type * sizeof(__int64);

  ::memcpy(leb.base() + offset, uint64s, sizeof(uint64s));
  
  unsigned __int64 * ui64p = reinterpret_cast<unsigned __int64*>(leb.base() + offset);

  dump_val("leb[unsigned __int64:0]= ", *(ui64p + 0));
  dump_val("leb[unsigned __int64:1]= ", *(ui64p + 1));
  dump_val("leb[unsigned __int64:2]= ", *(ui64p + 2));

  ::memcpy(beb.base() + offset, uint64s, sizeof(uint64s));
  
  ui64p = reinterpret_cast<unsigned __int64*>(beb.base() + offset);
  
  yat::Endianness::swap_8_array(reinterpret_cast<char*>(ui64p), 
                                reinterpret_cast<char*>(ui64p), 
                                num_val_per_type);

  dump_val("beb[unsigned __int64:0]= ", *(ui64p + 0));
  dump_val("beb[unsigned __int64:1]= ", *(ui64p + 1));
  dump_val("beb[unsigned __int64:2]= ", *(ui64p + 2));
#endif
}
