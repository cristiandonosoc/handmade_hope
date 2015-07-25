#ifndef _GAME_MEMORY_CPP

internal void
InitializeMemoryManager(memory_manager* memoryManager, size_t size, uint8* base)
{
  memoryManager->size = size;
  memoryManager->base = base;
  memoryManager->used = 0;
}

/**
 * Pushes certain size into the memory manager stack.
 * Returns the current free address and then advances the pointer,
 * thus working as a stack push
 */
void*
_PushSize(memory_manager* memoryManager, size_t size)
{
  ASSERT((memoryManager->used + size) <= memoryManager->size);

  void* result = memoryManager->base + memoryManager->used;
  memoryManager->used += size;

  return result;
}
/**
 * Macros define to use the underlying _PushSize function. As the macros receive the type
 * and make the case to the pointer type, they work as a template
 */

#define PushStruct(memoryManager, type) (type*)_PushSize(memoryManager, sizeof(type))
#define PushArray(memoryManager, count, type) (type*)_PushSize(memoryManager, (count) * sizeof(type))

#define _GAME_MEMORY_CPP
#endif
