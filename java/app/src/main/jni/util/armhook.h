#pragma once

void UnFuck(uintptr_t ptr, size_t dwSize = 100);
void FuckCode(uintptr_t ptr, size_t dwSize = 100);
void NOP(uintptr_t addr, unsigned int count);
void WriteMemory(uintptr_t dest, uintptr_t src, size_t size);
void ReadMemory(uintptr_t dest, uintptr_t src, size_t size);

void SetUpHook(uintptr_t addr, uintptr_t func, uintptr_t *orig);
void InstallMethodHook(uintptr_t addr, uintptr_t func);
void CodeInject(uintptr_t addr, uintptr_t func, int);
uintptr_t zalupa(uintptr_t func, uintptr_t addr);
void JMPCode(uintptr_t func, uintptr_t addr);
void ChangeFOV(float newFOV);
void ChangeFloatValue(uintptr_t address, float newValue);
void WriteMemory12(uintptr_t dest, const unsigned char* src, size_t size);
void installHook(uintptr_t addr, uintptr_t func, uintptr_t *orig);