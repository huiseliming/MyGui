#pragma once
#include "Struct.h"
#include "Function.h"

namespace Core
{

	class MYGUI_API CLASS() Class : public Struct
	{
		GENERATED_OBJECT_BODY()
	public:
		Class(const std::string & name = "")
			: Struct(name)
		{}
		const std::vector<Class*>& GetParentClasses() { return _ParentClasses; }
		const Class* GetFirstParentClass() { return _ParentClasses.empty() ? nullptr : _ParentClasses[0]; }

		const std::vector<Class*>& GetChildrenClasses() { return _ChildrenClasses; }

		const std::vector<std::unique_ptr<Function>>& GetFunctions() { return _Functions; }

	protected:
		std::vector<Class*> _ParentClasses;
		std::vector<Class*> _ChildrenClasses;
		std::vector<std::unique_ptr<Function>> _Functions;

	private:
		void AddParentClass(Class* in_class)
		{
			if (_ParentClasses.empty() && in_class)
			{
				in_class->_ChildrenClasses.push_back(this);
			}
			_ParentClasses.push_back(in_class);
		}

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
			IMPL_DEFAULT_TYPE_BASE_FUNCTION(CppType)
			IMPL_DEFAULT_TYPE_ANY_ACCESSOR_FUNCTION(CppType)
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














