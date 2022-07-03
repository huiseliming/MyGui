#pragma once
#include "Type.h"

namespace Core
{

	struct EnumValue
	{
		uint64_t _Value{ 0 };
		std::string _CppName;
		std::string _DisplayName;
	};

	MYGUI_API extern EnumValue EmptyEnumValue;

	class MYGUI_API CLASS() Enum : public Type
	{
		GENERATED_OBJECT_BODY()
	public:
		Enum(const std::string& name = "")
			: Type(name)
		{}

		const EnumValue& GetEnumValue(uint64_t Value) 
		{
			auto enum_value_iterator = _EnumValueMap.find(Value);
			if (enum_value_iterator != _EnumValueMap.end())
			{
				return enum_value_iterator->second;
			}
			return EmptyEnumValue;
		}

		const std::map<uint64_t, EnumValue>& GetEnumValueMap() { return _EnumValueMap; }

	protected:
		std::map<uint64_t, EnumValue> _EnumValueMap;

	private:
		template<typename T> friend struct TCustomTypeModifier;
		template<typename T> friend struct TDefaultTypeInitializer;
	};

	template<typename CppType>
	class TEnum : public Enum
	{
	public:
		TEnum(const std::string& name = "")
			: Enum(name)
		{
			_MemorySize = sizeof(CppType);
			_New = []() -> void* { return new CppType(); };
			_Delete = [](void* A) { delete static_cast<CppType*>(A); };
			_Constructor = [](void* A) { new (A) CppType(); };
			_Destructor = [](void* A) { ((const CppType*)(A))->~CppType(); };
			_CopyAssign = [](void* A, void* B) { *static_cast<CppType*>(A) = *static_cast<CppType*>(B); };
			_MoveAssign = [](void* A, void* B) { *static_cast<CppType*>(A) = std::move(*static_cast<CppType*>(B)); };
		}
	};

	// @test begin
	enum ENUM() ETestEnum
	{
		TE_0 METADATA(DisplayName = test0),
		TE_1 METADATA(DisplayName = "test1"),
		TE_2 METADATA(DisplayName = "test2"),
		TE_3 METADATA(DisplayName = 30),
		TE_4 METADATA(DisplayName = 4.0),
		TE_5 METADATA(DisplayName = TestFive),
		TE_6 METADATA(DisplayName = "测试6"),
	};

	MYGUI_API template<> Enum* GetStaticEnum<ETestEnum>();
	
	MYGUI_API
	template<>
	struct TCustomTypeModifier<ETestEnum> 
	{
		void operator()(Type* initialized_type);
	};
	// @test end
};

















