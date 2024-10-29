#include "kernel/src/melib.h"
#include <pspsdk.h>
#include <psppower.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <malloc.h>
#include <cstring>

PSP_MODULE_INFO("mcs", 0, 1, 1);
PSP_HEAP_SIZE_KB(-1024);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU | PSP_THREAD_ATTR_USER);

int* mem = nullptr;
bool stop = false;

int meLoop() {
  mem[0]++;
  mem[1] = 777;
  return !stop;
}

int main(int argc, char **argv) {
  scePowerSetClockFrequency(333, 333, 166);

  // To avoid cache line conflicts, a simple approach is to disable caching for shared memory using 0x40000000
  void* const _mem = memalign(16, sizeof(int) * 4);
  mem = (int* const) (0x40000000 | (u32)_mem);
  memset((void*)mem, 0, sizeof(int) * 4);
  ((u32*)_mem)[2] = 888;
  
  if (pspSdkLoadStartModule("ms0:/PSP/GAME/me/ms_klib.prx", PSP_MEMORY_PARTITION_KERNEL) < 0){
    sceKernelExitGame();
    return 0;
  }
  
  MeCom meCom = {
    nullptr,
    meLoop,
  };
  me_init(&meCom);
  sceKernelDelayThread(100000);

  pspDebugScreenInit();
  SceCtrlData ctl;
  do {
    /*
    // not necessary in our case
    sceKernelDcacheWritebackInvalidateAll();
    asm("sync");
    */
    sceCtrlPeekBufferPositive(&ctl, 1);
    pspDebugScreenSetXY(0, 1);
    pspDebugScreenPrintf("Counters %i; %i; %i; %i", mem[0], mem[1], mem[2], mem[3]++);
    sceDisplayWaitVblankStart();
  } while(!(ctl.Buttons & PSP_CTRL_HOME));
  
  stop = true;
  sceKernelDcacheWritebackInvalidateAll();
  asm("sync");
  
  pspDebugScreenClear();
  pspDebugScreenSetXY(0, 1);
  pspDebugScreenPrintf("Exiting...");
  sceKernelDelayThread(1000000);
  free(_mem);
  sceKernelExitGame();
  return 0;
}
