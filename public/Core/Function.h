#pragma once
#include "Struct.h"


namespace Core
{

	typedef void (*VMCall)(void* in_object, void* in_script_struct);

	class Function : public Struct
	{
	public:
		Function(const std::string& name = "")
			: Struct(name)
		{}

		virtual void* GetNativeCallFuncPtr() { return nullptr; }
		template<typename T>
		T* GetNativeCallFuncPtrAs() { return static_cast<T*>(GetNativeCallFuncPtr()); }

		VMCall GetVMCallFuncPtr() { return _VMCall; }

	private:
		VMCall _VMCall{ nullptr };

	private:
		template<typename T> friend struct TCustomTypeModifier;
		template<typename T> friend struct TDefaultTypeInitializer;
	};

	template<typename CppType, typename FunctionType>
	class TFunction : public Function
	{
	public:
		TFunction(const std::string& name = "")
			: Function(name)
		{
			_MemorySize = sizeof(CppType);
			_New = []() -> void* { return new CppType(); };
			_Delete = [](void* A) { delete static_cast<CppType*>(A); };
			_Constructor = [](void* A) { new (A) CppType(); };
			_Destructor = [](void* A) { ((const CppType*)(A))->~CppType(); };
			_CopyAssign = [](void* A, void* B) { *static_cast<CppType*>(A) = *static_cast<CppType*>(B); };
			_MoveAssign = [](void* A, void* B) { *static_cast<CppType*>(A) = std::move(*static_cast<CppType*>(B)); };
		}
		virtual void* GetNativeCallFuncPtr() { return &_NativeCall; }
		FunctionType _NativeCall;
	};

}












