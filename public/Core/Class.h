#pragma once
#include "Struct.h"
#include "Function.h"

namespace Core
{

	class MYGUI_API CLASS() Class : public Struct
	{
		GENERATED_OBJECT_BODY()
	public:
		Class(const std::string& name = "")
			: Struct(name)
		{}
		const std::vector<Class*>& GetParentClasses() { return _ParentClasses; }
		const Class* GetFirstParentClass() { return _ParentClasses.empty() ? nullptr : _ParentClasses[0]; }

		const std::vector<std::unique_ptr<Function>>& GetFunctions() { return _Functions; }

	protected:
		std::vector<Class*> _ParentClasses;
		std::vector<std::unique_ptr<Function>> _Functions;

	private:
		template<typename T> friend struct TCustomTypeModifier;
		template<typename T> friend struct TDefaultTypeInitializer;
	};

	template<typename CppType>
	class TClass : public Class
	{
	public:
		TClass(const std::string& name = "")
			: Class(name)
		{
			_MemorySize = sizeof(CppType);
			_New = []() -> void* { return new CppType(); };
			_Delete = [](void* A) { delete static_cast<CppType*>(A); };
			_Constructor = [](void* A) { new (A) CppType(); };
			_Destructor = [](void* A) { ((const CppType*)(A))->~CppType(); };
			if constexpr (std::is_copy_assignable_v<CppType>)
				_CopyAssign = [](void* A, void* B) { *static_cast<CppType*>(A) = *static_cast<CppType*>(B); };
			if constexpr (std::is_move_assignable_v<CppType>)
				_MoveAssign = [](void* A, void* B) { *static_cast<CppType*>(A) = std::move(*static_cast<CppType*>(B)); };
		}
	};

	// @test begin
	enum ETestEnum;
	class TestStructBase
	{
	public:
		static Class* StaticClass();
	};

	template<typename T>
	T* Cast(Object* object)
	{
		Class* target_class = T::StaticClass();
		Class* object_class = object->GetClass();
		Class* check_parent_class = target_class;
		while (check_parent_class)
		{
			if (check_parent_class == object_class)
			{
				return static_cast<T*>(object);
			}
			check_parent_class = const_cast<Class*>(check_parent_class->GetFirstParentClass());
		}
		return nullptr;
	}

	class CLASS() TestStructBaseB
	{
	public:
		static Class* StaticClass();
	};
	 
	class CLASS(dsdsdsds, sdsdsds, eee) TestStruct : public TestStructBase, TestStructBaseB
	{
	private:
		template<typename T> friend struct TCustomTypeModifier;
		template<typename T> friend struct TDefaultTypeInitializer;
	public:
		static Class* StaticClass();
	public:
		FIELD(abc = "123")
		std::vector<std::any> _Vector; 
		FIELD(abc = a123)
		int32_t _SInt32;
		FIELD(abc = 123.0)
		bool _Boolean;
		FIELD(abc = 123.0)
		bool* _BooleanPtr;
		FIELD(abc = 123.0)
		ETestEnum* _EnumPtr;
	public:
		
		FUNCTION()
		int& TestAdd(int*& a/* dont support int* reference wrapper, so need to register int* type */, int& b) {
			static int c = *a + b;
			return c; 
		}
	};

	MYGUI_API 
	template<> 
	struct TCustomTypeModifier<TestStruct>
	{
		void operator()(Type* uninitialized_type);
	};
	// @test end
}














