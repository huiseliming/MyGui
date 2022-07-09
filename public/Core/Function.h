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

		void Invoke(void* in_object, void* in_script_struct) { _VMCall(in_object, in_script_struct); }
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
			IMPL_DEFAULT_TYPE_BASE_FUNCTION(CppType)
		}
		virtual void* GetNativeCallFuncPtr() { return &_NativeCall; }
		FunctionType _NativeCall;
	};

}












