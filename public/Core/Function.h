#pragma once
#include "Struct.h"


namespace Core
{

	typedef void (*VMCall)(void* object, void* VM_call_object);

	class Function : public Struct
	{
	public:
		Function(const std::string& name = "")
			: Struct(name)
		{}

		void* GetNativeCallFuncPtr() { return _NativeCall; }
		VMCall GetVMCallFuncPtr() { return _VMCall; }

	private:
		void* _NativeCall{ nullptr };
		VMCall _VMCall{ nullptr };

	private:
		template<typename T> friend struct TCustomTypeModifier;
		template<typename T> friend struct TDefaultTypeInitializer;
	};
}












