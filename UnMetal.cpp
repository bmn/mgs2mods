#include "UnMetal.framework.h" 

namespace UnMetal {
     void RunUnFocused() {
         int failures = 0;

         // NOP the instruction to set pause screen active = 1 on unfocus
         failures += !PatchMemory("33 F6 56 FF 15 ? ? ? ? 89 35 ? ? ? ? E9", 9, "\x90\x90\x90\x90\x90\x90");

         // Add 0x80 (background) to the DSound device mode
         failures += !PatchMemory("C7 44 24 28 E0 01 00 00 C7", 5, "\x81");
         failures += !PatchMemory("C7 44 24 28 E0 01 00 00 89", 5, "\x81");
         failures += !PatchMemory("C7 82 BC 00 00 00 E2 01 00 00 8B", 7, "\x81");
         failures += !PatchMemory("C7 80 BC 00 00 00 E2 01 00 00 8B", 7, "\x81");

         if (failures) {
             char message[100];
             snprintf(message, 100, "%d patches failed :(\n\nYou may want to remove this ASI from scripts.", failures);
             MessageBoxA(NULL, message, "UnMetal UnFocused", MB_ICONWARNING);
         }
     }
 }