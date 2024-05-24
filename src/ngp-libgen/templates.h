#define ASM_FUNC_TEMPLATE \
".arch armv7a\n\n" \
".section .vitalink.fstubs.{libname},\"ax\",%progbits\n\n" \
".align 4\n" \
".global {funcname}\n" \
".type {funcname}, %function\n" \
"{funcname}:\n" \
".if GEN_WEAK_EXPORTS\n" \
"\t.word 0x{attr:04X}0008\n" \
".else\n" \
"\t.word 0x{attr:04X}0000\n" \
".endif //GEN_WEAK_EXPORTS\n" \
".word 0x{libnid:08X}\n" \
".word 0x{funcnid:08X}\n" \
".align 4\n\n"

#define ASM_VAR_TEMPLATE \
".arch armv7a\n\n" \
".section .vitalink.vstubs.{libname},\"\",%progbits\n\n" \
".align 4\n" \
".global {varname}\n" \
".type {varname}, %object\n" \
"{varname}:\n" \
".if GEN_WEAK_EXPORTS\n" \
"\t.word 0x{attr:04X}0008\n" \
".else\n" \
"\t.word 0x{attr:04X}0000\n" \
".endif //GEN_WEAK_EXPORTS\n" \
".word 0x{libnid:08X}\n" \
".word 0x{varnid:08X}\n" \
".align 4\n\n"

#define CMAKE_MAIN_TEMPLATE \
"cmake_minimum_required(VERSION 3.20)\n\n" \
"if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)\n" \
"  message(FATAL_ERROR \"Please define cmake toolchain!\")\n" \
"endif()\n"

#define CMAKE_PROJECT_TEMPLATE \
"project({project})\n" \
"enable_language(ASM)\n\n"

#define CMAKE_LIB_TEMPLATE \
"add_library({library}_stub STATIC ${{{library}_ASM}})\n" \
"target_compile_definitions({library}_stub PRIVATE -DGEN_WEAK_EXPORTS=0)\n" \
"add_library({library}_stub_weak STATIC ${{{library}_ASM}})\n" \
"target_compile_definitions({library}_stub_weak PRIVATE -DGEN_WEAK_EXPORTS=1)\n" \
"install(TARGETS {library}_stub {library}_stub_weak DESTINATION /opt/ngpsdk/arm-vita-eabi/lib/{firmware})\n\n"
