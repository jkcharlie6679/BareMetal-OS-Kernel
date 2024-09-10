unsigned int little_2_big_u32(unsigned int str){
  return  ((str>>24)&0xff) | // move byte 3 to byte 0
          ((str<<8)&0xff0000) | // move byte 1 to byte 2
          ((str>>8)&0xff00) | // move byte 2 to byte 1
          ((str<<24)&0xff000000); // byte 0 to byte 3
}