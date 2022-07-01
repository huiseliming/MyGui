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

	private:
		template<typename T> friend struct TCustomClassModifier;
		template<typename T> friend struct TDefaultClassInitializer;
	};

	// @test begin
	enum ETestEnum;

	class CLASS(dsdsdsds, sdsdsds, eee) TestStruct
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
















