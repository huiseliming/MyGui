#pragma once
#include "Reflect/Struct.h"
#include "Reflect/Class.h"

namespace Reflect 
{

	// @test begin
	template<>
	struct TDefaultReflectInitializer<TestStruct>
	{
		void operator()(Class* uninitialized_class)
		{
			uninitialized_class->_Fields.push_back(MakeField<std::vector<std::any>>("_Vector", offsetof(TestStruct, _Vector)));
			uninitialized_class->_Fields.push_back(MakeField<int32_t>("_SInt32", offsetof(TestStruct, _SInt32)));
		}
	};

	template<>
	struct TCustomReflectModifier<TestStruct>
	{
		void operator()(Class* uninitialized_class)
		{
			uninitialized_class->_Fields.push_back(MakeField<bool>("_Boolean", offsetof(TestStruct, _Boolean)));
		}
	};

	static TReflectClassAutoInitializer<TestStruct> SReflectClassAutoInitializer;

	Class* TestStruct::StaticClass()
	{
		static Class static_class("TestStruct");
		return &static_class;
	}
	// @test end
}

