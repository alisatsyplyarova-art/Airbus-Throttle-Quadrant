#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../Firmware/AirbusThrottleQuadrant/AxisMath.h"
static uint16_t map(uint16_t raw, uint16_t lo, uint16_t hi) {
  uint16_t out=9999; assert(axisMathCalibratedValue(raw,lo,hi,0,1023,false,out)); return out;
}
int main() {
  assert(map(100,100,300)==0); assert(map(200,100,300)==512); assert(map(300,100,300)==1023);
  assert(map(99,100,300)==0); assert(map(95,100,300)==0);
  assert(map(301,100,300)==1023); assert(map(305,100,300)==1023);
  assert(map(4090,4090,20)==0); assert(map(0,4090,20)>0 && map(0,4090,20)<1023);
  assert(map(20,4090,20)==1023); assert(map(4089,4090,20)==0); assert(map(21,4090,20)==1023);
  uint16_t out=0;
  assert(!axisMathCalibratedValue(100,100,100,0,1023,false,out));
  assert(!axisMathCalibratedValue(4096,0,100,0,1023,false,out));
  assert(axisMathCalibratedValue(100,100,300,0,1023,true,out) && out==1023);
  assert(axisMathCalibratedValue(300,100,300,0,1023,true,out) && out==0);
  puts("AxisMath tests passed."); return 0;
}
