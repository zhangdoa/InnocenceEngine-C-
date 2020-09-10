#include "../Core/InnoMemory.h"
#include "../Core/InnoLogger.h"

template <typename T>
class TObjectPool
{
	//Single-linked-list
	struct Chunk
	{
		void* m_Target = nullptr;
		Chunk* m_Next = nullptr;
	};

public:
	TObjectPool() = delete;

	explicit TObjectPool(std::size_t poolCapability)
	{
		m_ObjectSize = sizeof(T) + sizeof(Chunk);
		m_PoolCapability = poolCapability;
		m_HeapAddress = reinterpret_cast<unsigned char*>(InnoMemory::Allocate(m_PoolCapability * m_ObjectSize));
		m_CurrentFreeChunk = reinterpret_cast<Chunk*>(m_HeapAddress);

		ConstructPool();

		InnoLogger::Log(LogLevel::Verbose, "InnoMemory: Object pool has been allocated at ", this, ".");
	}

	~TObjectPool() = default;

	T* Spawn()
	{
		if (!m_CurrentFreeChunk)
		{
			InnoLogger::Log(LogLevel::Error, "InnoMemory: Run out of object pool!");
			return nullptr;
		}

		auto l_Object = m_CurrentFreeChunk->m_Target;

		if (l_Object)
		{
			// Assign new free chunk
			auto l_Next = m_CurrentFreeChunk->m_Next;
			if (l_Next)
			{
				m_CurrentFreeChunk = l_Next;
			}
			else
			{
				m_CurrentFreeChunk = nullptr;
				InnoLogger::Log(LogLevel::Warning, "InnoMemory: Last free chuck has been allocated!");
			}

			return new(l_Object) T();
		}
		else
		{
			InnoLogger::Log(LogLevel::Error, "InnoMemory: Can't spawn object!");
			return nullptr;
		}
	}

	void Destroy(T* ptr)
	{
		//Allocate in-place a Chunk at the corresponding position
		auto l_NewFreeChunk = new(reinterpret_cast<unsigned char*>(ptr) - sizeof(Chunk)) Chunk();

		l_NewFreeChunk->m_Target = ptr;

		//Insert after the current free chunk
		if (!m_CurrentFreeChunk)
		{
			// Edge case, last Chunk
			l_NewFreeChunk->m_Next = nullptr;
			m_CurrentFreeChunk = l_NewFreeChunk;
		}
		else
		{
			l_NewFreeChunk->m_Next = m_CurrentFreeChunk->m_Next;
			m_CurrentFreeChunk->m_Next = l_NewFreeChunk;
		}
	}

private:
	void ConstructPool()
	{
		auto l_ObjectUC = m_HeapAddress;
		Chunk* l_PrevFreeChunk = nullptr;

		for (auto i = 0; i < m_PoolCapability; i++)
		{
			auto l_NewFreeChunk = new(l_ObjectUC) Chunk();

			l_NewFreeChunk->m_Target = l_ObjectUC + sizeof(Chunk);

			// Link from front to end
			if (l_PrevFreeChunk)
			{
				l_PrevFreeChunk->m_Next = l_NewFreeChunk;
			}

			l_NewFreeChunk->m_Next = nullptr;
			l_PrevFreeChunk = l_NewFreeChunk;

			l_ObjectUC += m_ObjectSize;
		}
	}

	std::size_t m_ObjectSize;
	std::size_t m_PoolCapability;
	unsigned char* m_HeapAddress;
	Chunk* m_CurrentFreeChunk;

public:
	static TObjectPool<T>* Create(uint32_t poolCapability)
	{
		auto l_TObjectPoolAddress = reinterpret_cast<TObjectPool<T>*>(InnoMemory::Allocate(sizeof(TObjectPool<T>)));
		auto l_TObjectPool = new(l_TObjectPoolAddress) TObjectPool<T>(poolCapability);
		return l_TObjectPool;
	}

	static void Clear(TObjectPool<T>* objectPool)
	{
		objectPool->ConstructPool();
	}

	static bool Destruct(TObjectPool<T>* objectPool)
	{
		Clear(objectPool);
		InnoMemory::Deallocate(objectPool);
		return true;
	}
};
