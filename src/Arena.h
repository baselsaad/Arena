#pragma once
#include <memory>
#include <functional>

#define ARENA_KB 1024
#define ARENA_MB 1024 * 1024

#if defined(_MSC_VER) 
#define BreakPoint() __debugbreak()
#else 
#include <cassert>
#define BreakPoint() assert(false)
#endif

namespace MemoryManagement {

	//@ToDo: using function pointer will be faster & optimize space 
	using DtorCallback = std::function<void()>;

	struct Node
	{
		DtorCallback DestructurFunc;
		Node* Parent;
	};

	class Arena
	{
	public:
		Arena(std::size_t sizeInBytes)
			: m_TotalSize(sizeInBytes)
		{
			m_Buffer = new uint8_t[sizeInBytes];
			//std::memset(m_Buffer, 0, sizeInBytes);
		}

		~Arena()
		{
			InvokeDestructors();
			delete[] m_Buffer;

			m_Buffer = nullptr;
			m_LinkedDestructors = nullptr;
		}

		Arena(const Arena&) = delete;
		Arena(Arena&&) = delete;
		Arena& operator=(const Arena&) = delete;
		Arena& operator=(const Arena&&) = delete;

	public:
		template<typename T, typename... Args>
		T* AllocateObj(Args&&... args)
		{
			// @ToDO: reset Offset on fail 
			T* allocatedBuffer = AllocateRaw<T>(1);
			if (!allocatedBuffer)
			{
				BreakPoint();
				return nullptr;
			}

			T* obj = new(allocatedBuffer) T(std::forward<Args>(args)...);

			if constexpr (std::is_trivially_destructible_v<T>)
				return obj;
			
			if (!ChainDestructor(obj))
			{
				// no place for detor => clean 
				obj->~T();
				BreakPoint();
				return nullptr;
			}

			return obj;
		}

		template<typename T>
		T* AllocateRaw(std::size_t count)
		{
			std::size_t sizeInByte = count * sizeof(T);
			std::size_t space = m_TotalSize - m_Offset;

			void* currentPtr = m_Buffer + m_Offset;
			void* alignedPtr = currentPtr;
			if (std::align(alignof(T), sizeInByte, alignedPtr, space) == nullptr)
			{
				BreakPoint();
				return nullptr; // we could return new* and maintain it in a list, but will keep it simple 
			}

			m_Offset = (static_cast<uint8_t*>(alignedPtr) - m_Buffer) + sizeInByte;
			return static_cast<T*>(alignedPtr);
		}

		void Reset()
		{
			InvokeDestructors();
			m_Offset = 0;
		}


	private:
		template<typename T>
		bool ChainDestructor(T* obj)
		{
			Node* buffer = AllocateRaw<Node>(1);
			if (buffer == nullptr)
				return false;

			Node* node = new(buffer) Node();
			node->DestructurFunc = [obj]()
			{
				obj->~T();
			};
			node->Parent = m_LinkedDestructors;
			m_LinkedDestructors = node;

			return true;
		}

		void InvokeDestructors()
		{
			while (m_LinkedDestructors)
			{
				m_LinkedDestructors->DestructurFunc();
				m_LinkedDestructors = m_LinkedDestructors->Parent;
			}
		}

	private:
		Node* m_LinkedDestructors = nullptr;
		uint8_t* m_Buffer = nullptr;
		std::size_t m_Offset = 0;
		std::size_t m_TotalSize;
	};
}