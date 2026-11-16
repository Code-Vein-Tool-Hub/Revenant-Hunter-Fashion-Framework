#pragma once

// Signature scan in specified memory region
extern void* sigScan(const char* signature, const char* mask, size_t sigSize, void* memory, size_t memorySize);

// Signature scan in current process
extern void* sigScan(const char* signature, const char* mask, void* hint = nullptr);

// Resolves a RIP-relative operand inside a signature match: match + instrEnd + int32 at (match + dispOffset).
// Returns nullptr if match is null or the target falls outside the game module.
extern void* resolveRipRelative(void* match, size_t dispOffset, size_t instrEnd);