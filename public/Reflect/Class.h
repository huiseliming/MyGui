#pragma once
#include "Struct.h"

namespace Reflect
{

	class MYGUI_API Class : public Struct
	{
	public:
		Class(const std::string& name = "")
			: Struct(name)
		{}
		const std::vector<Struct*>& GetParentClasses() { return _ParentClasses; }
		const Struct* GetFirstParentClass() { return _ParentClasses.empty() ? nullptr : _ParentClasses[0]; }

	protected:
		std::vector<Struct*> _ParentClasses;

	private:
		template<typename T> friend struct TCustomClassModifier;
		template<typename T> friend struct TDefaultClassInitializer;
	};

	// @test begin
	enum ETestEnum;
	class CLASS() TestStructBase
	{
	public:
		static Class* StaticClass();
	};


	class CLASS() TestStructBaseB
	{
	public:
		static Class* StaticClass();
	};

	 
	class CLASS(dsdsdsds, sdsdsds, eee) TestStruct : public TestStructBase, TestStructBaseB
	{
	private:
		template<typename T> friend struct TCustomClassModifier;
		template<typename T> friend struct TDefaultClassInitializer;
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
	};

	MYGUI_API 
	template<> 
	struct TCustomClassModifier<TestStruct>
	{
		void operator()(Class* uninitialized_class);
	};

	// @test end
}
















